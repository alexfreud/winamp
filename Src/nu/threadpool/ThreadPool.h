#pragma once

#include <windows.h>
#include <bfc/platform/types.h>
#include <vector>
#include "../autolock.h"
#include "ThreadID.h"
#include "ThreadFunctions.h"
#include "threadpool_types.h"
/* random notes 

HANDLEs common to all threads

WaitForMultipleObjectsEx() around these
0 - killswitch
1 - shared APC event.  since threads might want to use APCs themselves, we'll use a different mechanism (thread-safe FIFO and an event).  the intention is that APCs that can go on any thread will use this handle
2 - per thread APC event.


parameters for "run my function" method
function pointer, user data, flags
flags:
interrupt - for very short non-locking functions where it is safe to interrupt another thread, uses QueueUserAPC
no_wait - spawn a new thread if all threads are busy
com_multithreaded - all threads are created with CoInitialize(0), if you need a COINIT_MULTITHREADED thread, use this flag

parameters for "add my handle" method
handle, function pointer, user data, flags
flags:
single_thread - only one thread in the pool will wait on your object, useful if your handle is not auto-reset

parameters for "function call repeat" - calls your function until you return 0
function pointer, user data, flags
flags:
single_thread - keep calling on the same thread
*/


class ThreadPool : public api_threadpool
{
public:
	static const char *getServiceName() { return "Thread Pool API"; }
	static const GUID getServiceGuid() { return ThreadPoolGUID; }	
public:
	// Owner API:
	ThreadPool();
	void Kill();

	// User API:
	/* If you have multiple events, APCs, etc and you need them to always run on the same thread
	you can reserve one */
	ThreadID *ReserveThread(int flags);
	/* Release a thread you've previously reserved */
	void ReleaseThread(ThreadID *thread_id);

	/* adds a waitable handle to the thread pool.  when the event is signalled, your function ptr will get called 
	user_data and id values get passed to your function.
	your function should return 1 to indicate that it can be removed
	flags, see api_threadpool	*/
	int AddHandle(ThreadID *threadid, HANDLE handle, api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id, int flags);
	void RemoveHandle(ThreadID *threadid, HANDLE handle);
	int RunFunction(ThreadID *threadid, api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id, int flags);

	size_t GetNumberOfThreads(); // total number of threads in the threadpool
	size_t GetNumberOfActiveThreads(); // number of threads that are currently being used (inside user function but not necessarily busy)

private:
	enum
	{
		TYPE_MT = 0,
		TYPE_STA = 1,
		TYPE_MT_RESERVED = 2,
		TYPE_STA_RESERVED = 3,		

		THREAD_TYPES = 4, // two thread types, single threaded apartment COM and multithreaded COM
	};
private:
	static DWORD CALLBACK WatchDogThreadProcedure_stub(LPVOID param);
	ThreadID *CreateNewThread_Internal(int thread_type = 0);
	DWORD CALLBACK WatchDogThreadProcedure();
	static int GetThreadType(int flags, int reserved = 0);
	static void GetThreadTypes(int flags, bool types[THREAD_TYPES]);
	void RemoveHandle_Internal(size_t start, HANDLE handle); // recursive helper function for RemoveHandle()
	void AddHandle_Internal(size_t start, HANDLE handle, int flags); // recursive helper function for RemoveHandle()

	Nullsoft::Utility::LockGuard guard; // guards threads, any_thread_handles, and non_reserved_handles data structures
	typedef std::vector<ThreadID*> ThreadList;
	ThreadList threads;
	ThreadPoolTypes::HandleList any_thread_handles[THREAD_TYPES];
	HANDLE killswitch;
	HANDLE watchdog_thread_handle;
	volatile LONG num_threads_available[THREAD_TYPES];
	ThreadFunctions thread_functions;
	HANDLE max_load_event[THREAD_TYPES];
protected:
	RECVS_DISPATCH;
};

