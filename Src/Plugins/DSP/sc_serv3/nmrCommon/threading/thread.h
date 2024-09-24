#pragma once
#ifndef _THREAD_H_
#define _THREAD_H_

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <pthread.h>
#else
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#endif
#include <stdexcept>
#include "macros.h"

///////////////////////////////////////////////////////////////////////////////
/// Various thread classes for applications that need to do threading and
/// aren't going to be including various Microsoft class libraries
//////////////////////////////////////////////////////////////////////////////


//********************************************************************
//* Tthread and thread
//*
//* allows you to run code on a thread via a template. Create a class
//* which has a method const unsigned operator()() and use it to instantiate
//* the template. If you want to use a bare function, encapsulate it in the
//* class pointer_to_thread_function. The Vthread class uses the traditional
//* virtual function approach
/*
	Example:
	
	class foo
	{
		const unsigned operator()()
		{
			for (int x = 0; x < 4: ++x)
			{
				cout << x << endl;
			}
			return 1;
		}
	};

	unsigned bar()
	{
		for (int x = 0; x < 4; ++x)
			cout << x << endl;
		return 1;
	}

	class narf: public Vthread
	{
		const unsigned operator()()
		{
			for (int x = 0; x < 4; ++x)
			{
				cout << x << endl;
			}
			return 1;
		}
	};

	main()
	{
		Tthread<foo> f;
		Tthread<pointer_to_thread_function> b(bar);
		narf n;
		n.start();
		f.start();
		b.start();
		::WaitForSingleObject(f,INFINITE); // or f.join();
		::WaitForSingleObject(b,INFINITE); // or b.join();
		::WaitForSingleObject(n,INFINITE); // or n.join();
	}

*/

//**********************************************************************

#ifdef _WIN32
#ifndef ASSERT
#define ASSERT(x) { if (!(x)) ::MessageBoxW(0,L"Assert failure",L"ASSERT",MB_OK); }
#endif
#else
#ifndef ASSERT
#define ASSERT(x) assert(x)
#endif
#endif

#ifdef _WIN32
#define THREAD_FUNC unsigned __stdcall
#else
#define THREAD_FUNC void*
#endif

#ifdef _WIN32
class thread_CORE
{
#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4100)
	nocopy(thread_CORE)
#pragma warning(pop)

protected:
	HANDLE	 m_threadHandle;

public:
	static int standard_signal_block() throw(){return 0;} // unix only

	thread_CORE() : m_threadHandle(0) {}

	~thread_CORE() throw()
	{
		if (m_threadHandle)
		{
			::WaitForSingleObject(m_threadHandle, INFINITE);
			::CloseHandle(m_threadHandle);
		}
		m_threadHandle = 0;
	}

	inline void join()
	{
		if (m_threadHandle)
		{
			::WaitForSingleObject(m_threadHandle, INFINITE);
		}
	}
	inline operator HANDLE() const throw() { return m_threadHandle; }
    static unsigned long getCurrentThreadID()   { return (unsigned long)GetCurrentThreadId(); }
};

template<class Handler>
class Tthread: public thread_CORE, public Handler
{
	static inline unsigned __stdcall _start_func(void *arg)
		{ return reinterpret_cast<Tthread<Handler> *>(arg)->operator()(); }

public:
	Tthread(){}
	template <class P1> Tthread( P1 &p1):Handler(p1){}
	template <class P1,class P2> Tthread( P1 &p1, P2 &p2):Handler(p1,p2){}
	template <class P1,class P2,class P3> Tthread( P1 &p1, P2 &p2, P3 &p3):Handler(p1,p2,p3){}
	template <class P1,class P2,class P3,class P4> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4):Handler(p1,p2,p3,p4){}
	template <class P1,class P2,class P3,class P4,class P5> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5):Handler(p1,p2,p3,p4,p5){}
	template <class P1,class P2,class P3,class P4,class P5,class P6> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5,P6 &p6):Handler(p1,p2,p3,p4,p5,p6){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6,P7 &p7):Handler(p1,p2,p3,p4,p5,p6,p7){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7,P8 &p8):Handler(p1,p2,p3,p4,p5,p6,p7,p8){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7, P8 &p8, P9 &p9):Handler(p1,p2,p3,p4,p5,p6,p7,p8,p9){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10> Tthread(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7, P8 &p8, P9 &p9, P10 &p10):Handler(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class P11> Tthread(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7, P8 &p8, P9 &p9, P10 &p10,P11 &p11):Handler(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class P11,class P12> Tthread(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7, P8 &p8, P9 &p9, P10 &p10,P11 &p11,P12 &p12):Handler(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12){}

	void start(int __unused = 1) throw(std::runtime_error)
	{
		unsigned m_threadIdentifier = 0;
		m_threadHandle = (HANDLE)::_beginthreadex(NULL, 0, _start_func, this, 0, &m_threadIdentifier);
		if (!m_threadHandle)
		{
			throw std::runtime_error("Could not start thread");
		}
	}
};

class SimpleThread
{
protected:
	HANDLE m_threadHandle;

public:
	SimpleThread(unsigned (__stdcall *_start_func) (void *arg) = 0, void *user = 0) : m_threadHandle(0)
	{
		if (_start_func)
		{
			start(_start_func, user);
		}
	}

	~SimpleThread() throw()
	{
		if (m_threadHandle)
		{
			::CloseHandle(m_threadHandle);
		}
		m_threadHandle = 0;
	}

	void start(unsigned (__stdcall *_start_func) (void *arg), void *user) throw(std::runtime_error)
	{
		unsigned m_threadIdentifier = 0;
		m_threadHandle = (HANDLE)::_beginthreadex(NULL, 0, _start_func, user, 0, &m_threadIdentifier);
		if (!m_threadHandle)
		{
			throw std::runtime_error("Could not start thread");
		}
	}
};

#else

class thread_CORE
{
	nocopy(thread_CORE)

protected:
	pthread_t	m_threadHandle;

public:
	static int standard_signal_block() throw()
	{
		sigset_t catchset;
		sigemptyset(&catchset);
		sigaddset(&catchset,SIGPIPE);
		sigaddset(&catchset,SIGTERM);
		sigaddset(&catchset,SIGHUP);
		sigaddset(&catchset,SIGINT);
		sigaddset(&catchset,SIGQUIT);
		sigaddset(&catchset,SIGTSTP); // ^Z allow this
		sigaddset(&catchset,SIGCHLD);
		sigaddset(&catchset,SIGWINCH);
		sigaddset(&catchset,SIGUSR1);
		sigaddset(&catchset,SIGUSR2);
		return pthread_sigmask(SIG_BLOCK,&catchset,NULL);
	}

	thread_CORE() : m_threadHandle(0) {}

	~thread_CORE() throw()
	{
		if (m_threadHandle)
		{
			::pthread_join(m_threadHandle, NULL);
		}
		m_threadHandle = 0;
	}

	inline void join()
	{
		if (m_threadHandle)
		{
			::pthread_join(m_threadHandle,NULL);
			m_threadHandle = 0;
		}
	}
	inline operator pthread_t() const throw() { return m_threadHandle; }
    static unsigned long getCurrentThreadID()   { return (unsigned long)::pthread_self(); }
};

template<class Handler>
class Tthread: public thread_CORE, public Handler
{
	static inline void* _start_func(void *arg)
	{
		standard_signal_block();
		long x = (long)(reinterpret_cast<Tthread<Handler> *>(arg))->operator()();
		return (void*)x;
	}

public:
	Tthread(){}
	template <class P1> Tthread( P1 &p1):Handler(p1){}
	template <class P1,class P2> Tthread( P1 &p1, P2 &p2):Handler(p1,p2){}
	template <class P1,class P2,class P3> Tthread( P1 &p1, P2 &p2, P3 &p3):Handler(p1,p2,p3){}
	template <class P1,class P2,class P3,class P4> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4):Handler(p1,p2,p3,p4){}
	template <class P1,class P2,class P3,class P4,class P5> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5):Handler(p1,p2,p3,p4,p5){}
	template <class P1,class P2,class P3,class P4,class P5,class P6> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5,P6 &p6):Handler(p1,p2,p3,p4,p5,p6){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6,P7 &p7):Handler(p1,p2,p3,p4,p5,p6,p7){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7,P8 &p8):Handler(p1,p2,p3,p4,p5,p6,p7,p8){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9> Tthread( P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7, P8 &p8, P9 &p9):Handler(p1,p2,p3,p4,p5,p6,p7,p8,p9){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10> Tthread(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7, P8 &p8, P9 &p9, P10 &p10):Handler(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class P11> Tthread(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7, P8 &p8, P9 &p9, P10 &p10,P11 &p11):Handler(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11){}
	template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class P8,class P9,class P10,class P11,class P12> Tthread(P1 &p1, P2 &p2, P3 &p3, P4 &p4, P5 &p5, P6 &p6, P7 &p7, P8 &p8, P9 &p9, P10 &p10,P11 &p11,P12 &p12):Handler(p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12){}

	void start(int joined = 1) throw(std::runtime_error)
	{
		if (joined && m_threadHandle)
		{
			throw std::runtime_error("Thread already exists");
		}
		int ret = pthread_create(&m_threadHandle, NULL, _start_func, this);
		if (ret)
		{
			throw std::runtime_error("Could not start thread" + std::string(ret == EAGAIN ? " [Increase the open files (ulimit -n) limit]" : ""));
		}
	}
};


class SimpleThread
{
public:
	SimpleThread(void *(*_start_func) (void *arg) = 0, void *user = 0)
	{
		if (_start_func)
		{
			start(_start_func, user);
		}
	}

	void start(void *(*_start_func) (void *arg), void *user) throw(std::runtime_error)
	{
		pthread_t m_threadHandle = 0;
		pthread_attr_t attr;
		int ret = pthread_attr_init(&attr);
		if (ret)
		{
			throw std::runtime_error("Could not start thread - pthread_attr_init failure");
		}

		ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if (ret)
		{
			pthread_attr_destroy(&attr);
			throw std::runtime_error("Could not start thread - pthread_attr_setdetachstate failure");
		}

		ret = pthread_create(&m_threadHandle,&attr,_start_func,user);
		if (ret)
		{
			pthread_attr_destroy(&attr);
			throw std::runtime_error("Could not start thread" + std::string(ret == EAGAIN ? " [Increase the open files (ulimit -n) limit]" : ""));
		}

		pthread_attr_destroy(&attr);
	}
};
#endif

class pointer_to_thread_function
{
public:
	typedef unsigned (*func_t)();

	inline pointer_to_thread_function(func_t f):m_function(f){}
private:
	func_t	m_function;
protected:
	inline const unsigned operator()() { return (*m_function)(); }
};

class vthread_stub
{
	protected:
		virtual const unsigned operator()() = 0;
		virtual ~vthread_stub(){}
};

typedef Tthread<vthread_stub> Vthread;

#ifdef _WIN32
class event
{
#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4100)
	nocopy(event)
#pragma warning(pop)

private:
	HANDLE	m_event;

public:
	event(BOOL bManualReset) throw(std::runtime_error);
	~event() throw();

	inline operator HANDLE() const throw() { return m_event; }
	void wait() throw(std::runtime_error);
	void setEvent() throw(std::runtime_error);
	void resetEvent() throw(std::runtime_error);
};

#else

class Win32SyncObject
{
public:
	virtual void syncObjectWait() throw(std::runtime_error) = 0;
	virtual bool syncObjectTimedWait(int milliseconds) throw(std::runtime_error) = 0;
	virtual ~Win32SyncObject(){}
};
typedef Win32SyncObject* Win32SyncObjectPtr;

/// these provide basic resource mgmt for typical uses
#endif

class conditionVariable;

namespace AOL_namespace {
#pragma pack(push, 1)
// conflicts with headers in solaris
#ifndef _WIN32
class mutex : public Win32SyncObject
#else
class mutex
#endif
{
	nocopy(mutex)

private:
	pthread_mutex_t	m_mutex;

public:
	mutex() throw(std::runtime_error);
	~mutex() throw();
	void lock() throw(std::runtime_error);
	bool timedLock(int milliseconds) throw(std::runtime_error);
	void unlock() throw(std::runtime_error);
	void syncObjectWait() throw(std::runtime_error) { lock(); }
	bool syncObjectTimedWait(int milliseconds) throw(std::runtime_error) { return timedLock(milliseconds); }
#ifndef _WIN32
	inline operator Win32SyncObject*() throw() { return this; }
#endif
	friend class ::conditionVariable;
};
#pragma pack(pop)

class rwLock
{
    pthread_rwlock_t m_lock;

public:
    rwLock();
    ~rwLock();
    bool tryRdLock();
    void lock();
    void rdLock();
    void unlock();
};

}

class conditionVariable
{
	nocopy(conditionVariable)

private:
	pthread_cond_t	m_conditionVariable;

public:
	conditionVariable() throw(std::runtime_error);
	~conditionVariable() throw();
	void wait(AOL_namespace::mutex &m) throw(std::runtime_error);
	bool timedWait(AOL_namespace::mutex &m,int milliseconds) throw(std::runtime_error);
	void signal() throw(std::runtime_error);
	void broadcast() throw(std::runtime_error);
};

#ifndef _WIN32
class event : public Win32SyncObject
{
	nocopy(event)

private:
	conditionVariable		m_conditionVariable;
	AOL_namespace::mutex	m_mutex;
	bool m_manualReset;
	bool m_signaled;

public:
	event(bool bManualReset) throw(std::runtime_error);
	void wait() throw(std::runtime_error);
	bool timedWait(int milliseconds) throw(std::runtime_error);
	void setEvent() throw(std::runtime_error);
	void resetEvent() throw(std::runtime_error);
	void syncObjectWait() throw(std::runtime_error) { wait(); }
	bool syncObjectTimedWait(int milliseconds) throw(std::runtime_error) { return timedWait(milliseconds); }
	inline operator Win32SyncObject*() throw() { return this; }

};
#endif

/* class stackLock

	Stack based mutex locker/unlocker for unwrapped Win32 mutexes.
	Equivalent to CAutoLock for CCritSec objects.
*/

class stackLock
{
    AOL_namespace::mutex &m_m;
public:
    stackLock(AOL_namespace::mutex &m) : m_m(m)
    {
	m_m.lock();
    }
    ~stackLock()
    {
	m_m.unlock();
    }
};

class stackRWLock
{
    AOL_namespace::rwLock &m_rw;
    bool m_locked;

public:
    stackRWLock(AOL_namespace::rwLock &l, bool reader = true, bool lockNow = true) : m_rw(l), m_locked(lockNow)
    {
	if (lockNow == false)
	    return;
	if (reader)
	    m_rw.rdLock();
	else
	    m_rw.lock();
    }
    ~stackRWLock()
    {
        if (m_locked)
            m_rw.unlock();
    }
    bool tryRdLock()
    {
	if (m_locked == false)
	{
	    if (m_rw.tryRdLock())
	    {
		m_locked = true;
		return true;
	    }
	}
	return false;
    }
    void lock()         { if (m_locked == false) { m_rw.lock(); m_locked = true; } }
    void rdLock()       { if (m_locked == false) { m_rw.rdLock(); m_locked = true; } }
    void unlock()       { if (m_locked) { m_rw.unlock(); m_locked = false; } }

};

#ifndef _WIN32

#define WAIT_ABANDONED (-1)
#define WAIT_TIMEOUT (0)
#define WAIT_OBJECT_0 (1)
#define INFINITE (-1)

int WaitForSingleObject(Win32SyncObject &o,int milli_timeout) throw();
inline int WaitForSingleObject(Win32SyncObject *o,int milli_timeout) throw() { return WaitForSingleObject(*o,milli_timeout); }
int WaitForMultipleObjects(int count,Win32SyncObjectPtr *objs,bool waitall,int milliseconds) throw();
inline void CloseHandle(Win32SyncObject *o) { delete o; }
typedef Win32SyncObject *HANDLE;

#endif

inline bool _SetEvent(event &e) { bool result=false; try {e.setEvent(); result=true; }catch(...){} return result; }
inline bool _SetEvent(event *e) { return _SetEvent(*e); }
inline bool _ResetEvent(event &e) { bool result=false; try {e.resetEvent();result=true;}catch(...){} return result; }
inline bool _ResetEvent(event *e) { return _ResetEvent(*e); }

namespace AOL_namespace
{
	template <typename T> class synchronizedPrimitive
	{
	private:
		mutable AOL_namespace::mutex m_lock;
		T m_t;

	public:
		synchronizedPrimitive(){}
		synchronizedPrimitive(const T &t):m_t(t){}
		T get() const throw() { stackLock sl(m_lock); return m_t; }
		void set(const T &t) throw() { stackLock sl(m_lock); m_t = t; }
	};
}

// thread safe sleep function
void safe_sleep(int sec, int usec = 0);

#endif
