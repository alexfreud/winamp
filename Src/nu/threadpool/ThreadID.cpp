#include "ThreadID.h"

DWORD ThreadID::thread_func_stub(LPVOID param)
{
	ThreadID *t = static_cast<ThreadID*>(param);
	if (t != NULL)
	{
		return t->ThreadFunction();
	}
	else return 0;
}

void ThreadID::Kill()
{
	if (threadHandle && threadHandle != INVALID_HANDLE_VALUE)
	{
		//cut: WaitForSingleObject(threadHandle, INFINITE);
		while (WaitForMultipleObjectsEx(1, &threadHandle, FALSE, INFINITE, TRUE) != WAIT_OBJECT_0)
		{
		}
	}
		
}

ThreadID::ThreadID(ThreadFunctions *t_f, HANDLE killswitch, HANDLE global_functions_semaphore,
									 ThreadPoolTypes::HandleList &inherited_handles, 
									 volatile LONG *thread_count, HANDLE _max_load_event, 
									 int _reserved, int _com_type) : ThreadFunctions(_reserved)
{
	/* initialize values */
	released = false;
	InitializeCriticalSection(&handle_lock);

	/* grab values passed to us */
	reserved = _reserved;
	com_type = _com_type;
	max_load_event = _max_load_event;
	global_functions = t_f;
	num_threads_available = thread_count;

	/* wait_handles[0] is kill switch */
	wait_handles.push_back(killswitch);

	/* wait_handles[1] is wake switch */
	wakeHandle = CreateSemaphore(0, 0, ThreadPoolTypes::MAX_SEMAPHORE_VALUE, 0);
	wait_handles.push_back(wakeHandle);

	if (reserved)
	{
		/* if thread is reserved,
		wait_handles[2] is a Funcion Call wake semaphore 
		for this thread only. */
		wait_handles.push_back(functions_semaphore); // WAIT_OBJECT_0+1 == per-thread queued functions
	}
	else
	{
		/* if thread is not reserved,
		wait_handles[2] is a Function Call wake semaphore
		global to all threads */	
		wait_handles.push_back(global_functions_semaphore); // WAIT_OBJECT_0+2 == any-thread queued functions
	}

	/* add inherited handles 
	(handles added to thread pool before this thread was created) */
	for ( ThreadPoolTypes::HandleList::iterator itr = inherited_handles.begin(); itr != inherited_handles.end(); itr++ )
	{
		wait_handles.push_back( *itr );
	}

	/* start thread */
	threadHandle = CreateThread(0, 0, thread_func_stub, this, 0, 0);
}

ThreadID::~ThreadID()
{
	CloseHandle(threadHandle);
	CloseHandle(wakeHandle);
	DeleteCriticalSection(&handle_lock);
}

bool ThreadID::TryAddHandle(HANDLE new_handle)
{
	// let's see if we get lucky and can access the handle list directly
	if (TryEnterCriticalSection(&handle_lock))
	{
		// made it
		wait_handles.push_back(new_handle);
		LeaveCriticalSection(&handle_lock);
		return true;
	}
	else
	{
		ReleaseSemaphore(wakeHandle, 1, 0); // kick the thread out of WaitForMultiple...
		return false;
	}
}

void ThreadID::WaitAddHandle(HANDLE handle)
{
	// wakeHandle already got released once by nature of this function being called
	EnterCriticalSection(&handle_lock);
	wait_handles.push_back(handle);
	LeaveCriticalSection(&handle_lock);
	ReleaseSemaphore(wakeHandle, 1, 0); // kick out the second wait
}

void ThreadID::AddHandle(HANDLE new_handle)
{
	if (!TryAddHandle(new_handle))
		WaitAddHandle(new_handle);
}

bool ThreadID::TryRemoveHandle(HANDLE handle)
{
	// let's see if we get lucky and can access the handle list directly
	if (TryEnterCriticalSection(&handle_lock))
	{
		RemoveHandle_Internal(handle);
		LeaveCriticalSection(&handle_lock);
		return true;
	}
	else
	{
		ReleaseSemaphore(wakeHandle, 1, 0); // kick the thread out of WaitForMultiple...
		return false;
	}
	return false;
}

void ThreadID::WaitRemoveHandle(HANDLE handle)
{
	// wakeHandle already got released once by nature of this function being called
	EnterCriticalSection(&handle_lock);
	RemoveHandle_Internal(handle);
	LeaveCriticalSection(&handle_lock);
	ReleaseSemaphore(wakeHandle, 1, 0); // kick out the second wait
}

void ThreadID::RemoveHandle(HANDLE handle)
{
	if (!TryRemoveHandle(handle))
		WaitRemoveHandle(handle);
}

void ThreadID::RemoveHandle_Internal(HANDLE handle)
{
	// first three handles are reserved, so start after that
	for (size_t i=3;i<wait_handles.size();i++)
	{
		if (wait_handles[i] == handle)
		{
			wait_handles.erase(wait_handles.begin() + i);
			i--;
		}
	}
}

bool ThreadID::IsReserved() const
{
	return !!reserved;
}

DWORD CALLBACK ThreadID::ThreadFunction()
{
	switch(com_type)
	{
	case api_threadpool::FLAG_REQUIRE_COM_MT:
		CoInitializeEx(0, COINIT_MULTITHREADED);
		break;
	case api_threadpool::FLAG_REQUIRE_COM_STA:
		CoInitialize(0);
		break;
	}

	while (1)
	{
		InterlockedIncrement(num_threads_available);
		EnterCriticalSection(&handle_lock);
		DWORD ret = WaitForMultipleObjectsEx((DWORD)wait_handles.size(), wait_handles.data(), FALSE, INFINITE, TRUE);
		// cut: LeaveCriticalSection(&handle_lock);
		if (InterlockedDecrement(num_threads_available) == 0 && !reserved) 
			SetEvent(max_load_event); // notify the watch dog if all the threads are used up

		if (ret == WAIT_OBJECT_0)
		{
			// killswitch
			LeaveCriticalSection(&handle_lock);
			break;
		}
		else if (ret == WAIT_OBJECT_0 + 1)
		{
			// we got woken up to release the handles lock
			// wait for the second signal
			LeaveCriticalSection(&handle_lock);
			InterlockedIncrement(num_threads_available);
			WaitForSingleObject(wakeHandle, INFINITE);
			InterlockedDecrement(num_threads_available);
		}
		else if (ret == WAIT_OBJECT_0 + 2)
		{
			LeaveCriticalSection(&handle_lock);
			api_threadpool::ThreadPoolFunc func;
			void *user_data;
			intptr_t id;
			if (reserved)
			{
				// per-thread queued functions
				if (PopFunction(&func, &user_data, &id))
				{
					func(0, user_data, id);
				}
			}
			else
			{
				// global queued functions
				if (global_functions->PopFunction(&func, &user_data, &id))
				{
					func(0, user_data, id);
				}
			}
		}
		else if (ret > WAIT_OBJECT_0 && ret < (WAIT_OBJECT_0 + wait_handles.size()))
		{
			DWORD index = ret - WAIT_OBJECT_0;
			HANDLE handle = wait_handles[index];
			LeaveCriticalSection(&handle_lock);
			/* !!! race condition here if someone calls ThreadPool::RemoveHandle and then CloseHandle() !!!
			  before calling RemoveHandle, caller needs to either 
				ensure that Event is unsignalled (And won't be signalled)
				or call RemoveHandle from within the function callback */
			api_threadpool::ThreadPoolFunc func;
			void *user_data;
			intptr_t id;
			if (global_functions->Get(handle, &func, &user_data, &id))
			{
				func(handle, user_data, id);
			}
		}
		else
		{
			LeaveCriticalSection(&handle_lock);
		}
	}
	if (com_type & api_threadpool::MASK_COM_FLAGS)
		CoUninitialize();
	return 0;
}

bool ThreadID::CanRunCOM(int flags) const
{
	switch(com_type)
	{
	case api_threadpool::FLAG_REQUIRE_COM_MT: // if we're a CONIT_MULTITHREADEX thread (default)
		return !(flags & api_threadpool::FLAG_REQUIRE_COM_STA); // don't let STA stuff run
	case api_threadpool::FLAG_REQUIRE_COM_STA: // if we're a CoInitialize(0) thread
		return !(flags & api_threadpool::FLAG_REQUIRE_COM_MT); // don't let MT stuff run
	}
	return false; // shouldn't get here
}

bool ThreadID::IsReleased() const
{
	return released;
}

void ThreadID::Reserve()
{
	released=false;
}

void ThreadID::Release()
{
	released=true;
}