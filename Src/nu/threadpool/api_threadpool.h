#pragma once

#include <windows.h>
#include <bfc/platform/types.h>
#include <bfc/dispatch.h>

class ThreadID;

class api_threadpool : public Dispatchable
{
protected:
	api_threadpool() {}
	~api_threadpool() {}
public:
	typedef int (*ThreadPoolFunc)(HANDLE handle, void *user_data, intptr_t id);
	enum
	{
		// pass this flag to AddHandle or RunFunction indicate that your thread function
		// might run for a long time
		FLAG_LONG_EXECUTION  = 0x1,
		FLAG_REQUIRE_COM_STA = 0x2,
		FLAG_REQUIRE_COM_MT  = 0x4,
		MASK_COM_FLAGS       = 0x6,
	};

public:
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

	enum
	{
		RESERVETHREAD            = 0,
		RELEASETHREAD            = 1,
		ADDHANDLE                = 2,
		REMOVEHANDLE             = 3,
		RUNFUNCTION              = 4,
		GETNUMBEROFTHREADS       = 5,
		GETNUMBEROFACTIVETHREADS = 6,
	};
};

inline ThreadID *api_threadpool::ReserveThread(int flags)
{
	return _call(RESERVETHREAD, (ThreadID *)0, flags);
}

inline void api_threadpool::ReleaseThread(ThreadID *thread_id)
{
	_voidcall(RELEASETHREAD, thread_id);
}

inline int api_threadpool::AddHandle(ThreadID *threadid, HANDLE handle, api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id, int flags)
{
	return _call(ADDHANDLE, (int)1, threadid, handle, func, user_data, id, flags);
}

inline void api_threadpool::RemoveHandle(ThreadID *threadid, HANDLE handle)
{
	_voidcall(REMOVEHANDLE, threadid, handle);
}

inline int api_threadpool::RunFunction(ThreadID *threadid, api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id, int flags)
{
	return _call(RUNFUNCTION, (int)1, threadid, func, user_data, id, flags);
}

inline size_t api_threadpool::GetNumberOfThreads()
{
	return _call(GETNUMBEROFTHREADS, (size_t)0);
}

inline size_t api_threadpool::GetNumberOfActiveThreads()
{
	return _call(GETNUMBEROFACTIVETHREADS, (size_t)0);
}

// {4DE015D3-11D8-4ac6-A3E6-216DF5252107}
static const GUID ThreadPoolGUID = 
{ 0x4de015d3, 0x11d8, 0x4ac6, { 0xa3, 0xe6, 0x21, 0x6d, 0xf5, 0x25, 0x21, 0x7 } };

