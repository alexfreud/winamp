#include "threading/thread.h"
#ifndef _WIN32
#include <unistd.h>
#include "global.h"
#else
#include <pthread.h>
#endif

using namespace std;

// thread safe sleep function
// which is clamped to 10 ms.
void safe_sleep(int sec, int usec)
{
#ifdef _WIN32
	int ms = (usec / 1000);
	ms += (sec * 1000);
	if (ms < 10) ms = 10;
	::Sleep(ms);
#else
	struct timeval mytime;
	if (!sec && (usec < 10000)) usec = 10000;
	mytime.tv_sec = sec;
	mytime.tv_usec = usec;
	select(0, NULL, NULL, NULL, &mytime);
#endif
}

#ifndef _WIN32
#include <errno.h>
#include <vector>
#endif

using namespace AOL_namespace;

rwLock::rwLock()
{
#ifdef PTHREAD_RWLOCK_PREFER_WRITER_NP
    pthread_rwlockattr_t attr;
    ::pthread_rwlockattr_init (&attr);
    ::pthread_rwlockattr_setkind_np (&attr, PTHREAD_RWLOCK_PREFER_WRITER_NP);
    int r = ::pthread_rwlock_init (&m_lock, &attr);
    ::pthread_rwlockattr_destroy (&attr);
#else
    int r = ::pthread_rwlock_init (&m_lock, NULL);
#endif
    if (r)
        throw runtime_error("Could not create rwlock");
}

rwLock::~rwLock()
{
    ::pthread_rwlock_destroy (&m_lock);
}

void rwLock::lock()
{
    ::pthread_rwlock_wrlock (&m_lock);
}

void rwLock::rdLock()
{
    ::pthread_rwlock_rdlock (&m_lock);
}

bool rwLock::tryRdLock()
{
    int n = ::pthread_rwlock_tryrdlock (&m_lock);
    if (n == 0)
        return true;
    if (n != EBUSY)
        throw runtime_error("Error with trying rwlock, " + n);
    return false;
}

void rwLock::unlock()
{
    ::pthread_rwlock_unlock (&m_lock);
}


mutex::mutex() throw(runtime_error)
{
	if (::pthread_mutex_init(&m_mutex, NULL))
	{
		throw runtime_error("Could not create mutex");
	}
}

mutex::~mutex() throw()
{
	::pthread_mutex_destroy(&m_mutex);
#ifdef _WIN32
	m_mutex = NULL;
#endif
}

void mutex::lock() throw(runtime_error)
{
	if (::pthread_mutex_lock(&m_mutex))
	{
		throw runtime_error("Could not lock mutex");
	}
}

bool mutex::timedLock(int milliseconds) throw(runtime_error)
{
	if (milliseconds == INFINITE)
	{
		lock();
		return true;
	}

	if (milliseconds == 0)
	{
		int err = ::pthread_mutex_trylock(&m_mutex);
		if (err == EBUSY)
		{
			return false;
		}
		if (err)
		{
			throw runtime_error("Could not trylock mutex");
		}
		return true;
	}

	int tenth_second_sleep_intervals = (milliseconds / 100);
	for (int x = 0; x < tenth_second_sleep_intervals; ++x)
	{
		int err = ::pthread_mutex_trylock(&m_mutex);
		if (!err)
		{
			return true;
		}
		safe_sleep(0, 100000);
		if (err == EBUSY)
		{
			continue;
		}
		throw runtime_error("Could not trylock mutex");
	}
	return false;
}

void mutex::unlock() throw(runtime_error)
{
	if (::pthread_mutex_unlock(&m_mutex))
	{
		throw runtime_error("Could not unlock mutex");
	}
}

conditionVariable::conditionVariable()  throw(runtime_error)
{
	if (::pthread_cond_init(&m_conditionVariable,NULL))
	{
		throw runtime_error("Could not create conditionVariable");
	}
}

conditionVariable::~conditionVariable() throw()
{
	::pthread_cond_destroy(&m_conditionVariable);
}

void conditionVariable::wait(mutex &m) throw(runtime_error)
{
	if (::pthread_cond_wait(&m_conditionVariable,&m.m_mutex))
	{
		throw runtime_error("Could not wait on condition variable");
	}
}

bool conditionVariable::timedWait(AOL_namespace::mutex &m,int milliseconds) throw(runtime_error)
{
	struct timespec ts;
	ts.tv_sec = ::time(NULL) + (milliseconds / 1000);
	ts.tv_nsec = (milliseconds - ((milliseconds / 1000) * 1000)) * 1000000; 
	int err = ::pthread_cond_timedwait(&m_conditionVariable,&m.m_mutex,&ts);
	if (!err)
	{
		return true;
	}
	if (err == ETIMEDOUT)
	{
		return false;
	}
	throw runtime_error("timedWait error");
}

void conditionVariable::signal() throw(runtime_error)
{
	if (::pthread_cond_signal(&m_conditionVariable))
	{
		throw runtime_error("Could not signal condition variable");
	}
}

void conditionVariable::broadcast() throw(runtime_error)
{
	if (::pthread_cond_broadcast(&m_conditionVariable))
	{
		throw runtime_error("Could not broadcast on condition variable");
	}
}

#ifdef _WIN32
event::event(BOOL bManualReset) throw(runtime_error) : m_event(NULL)
{
	m_event = ::CreateEvent(NULL,bManualReset,FALSE,NULL);
	if (!m_event)
	{
		throw runtime_error("Could not create event object");
	}
}

event::~event() throw()
{
	forgetHandleNULL(m_event);
}

void event::wait() throw(std::runtime_error)
{
	if (::WaitForSingleObject(m_event,INFINITE) != WAIT_OBJECT_0)
	{
		throw runtime_error("event::wait() - wait error");
	}
}

void event::setEvent() throw(std::runtime_error)
{
	if (!::SetEvent(m_event))
	{
		throw runtime_error("event::setEvent() - set error");
	}
}

void event::resetEvent() throw(std::runtime_error)
{
	if (!::ResetEvent(m_event))
	{
		throw runtime_error("event::resetEvent() - reset error");
	}
}
#else
event::event(bool manualReset) throw(runtime_error)
	: m_manualReset(manualReset), m_signaled(false)
{
}

void event::wait() throw(runtime_error)
{
	stackLock sl(m_mutex);
	while (!m_signaled)
	{
		m_conditionVariable.wait(m_mutex);
	}
	if (!m_manualReset)
	{
		m_signaled = false;
	}
}

bool event::timedWait(int milliseconds) throw(runtime_error)
{
	if (milliseconds == INFINITE)
	{
		wait();
		return true;
	}

	if (milliseconds == 0)
	{
		stackLock sl(m_mutex);
		bool result = m_signaled;
		if (m_signaled && !m_manualReset)
		{
			m_signaled = false;
		}
		return result;
	}

	stackLock sl(m_mutex);
	while(!m_signaled)
	{
		if (!m_conditionVariable.timedWait(m_mutex,milliseconds))
		{
			return false;
		}
	}

	if (!m_manualReset)
	{
		m_signaled = false;
	}
	return true;
}

void event::setEvent() throw(runtime_error)
{
	stackLock sl(m_mutex);
	m_signaled = true;
	m_conditionVariable.broadcast();
}

void event::resetEvent() throw(runtime_error)
{
	stackLock sl(m_mutex);
	m_signaled = false;
}

int WaitForSingleObject(Win32SyncObject &o, int milli_timeout) throw()
{
	int result = WAIT_ABANDONED;

	try
	{
		result = (o.syncObjectTimedWait(milli_timeout) ? WAIT_OBJECT_0 : WAIT_TIMEOUT);
	}
	catch(...) {}

	return result;
}

int WaitForMultipleObjects(int count, Win32SyncObjectPtr *objs, bool waitall, int milliseconds) throw()
{
	__uint64 start = time_now_ms();

	try
	{
		std::vector<bool> signaled(count,false);
		int sig_count = 0/*, ms = (milliseconds % 1000)*/,
			quantum = 100;/*(ms > 100 ? (ms / 10) : 100);*/

		// as milliseconds is only set as '1000' in main.cpp
		// then we don't need to do the checking and can just
		// hard-code the quantum value to give 100ms interval

		for (int x = 0; (x <= milliseconds) || (milliseconds == INFINITE); x += quantum)
		{
			for (int oo = 0; oo < count; ++oo)
			{
				if (!signaled[oo])
				{
					if (objs[oo]->syncObjectTimedWait(0))
					{
						signaled[oo] = true;
						++sig_count;
						if (!waitall)
						{
							return WAIT_OBJECT_0 + oo;
						}
						if (sig_count == count)
						{
							return WAIT_OBJECT_0;
						}
					}
				}
			} //for

			safe_sleep(0, quantum * 1000);
			
			__uint64 now = time_now_ms();
			if ((int)(now - start) >= milliseconds)
			{
				break;
			}
		} // for
		return WAIT_TIMEOUT;
	} //try
	catch(...) {}
	return WAIT_ABANDONED;
}
#endif
