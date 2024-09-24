#include "ThreadPool.h"

ThreadPool::ThreadPool()
{
	for ( int i = 0; i < THREAD_TYPES; i++ )
	{
		num_threads_available[ i ] = 0;
		max_load_event[ i ]        = CreateEvent( NULL, TRUE, FALSE, NULL );
	}

	killswitch = CreateEvent( NULL, TRUE, FALSE, NULL );

	// one thread of each type to start
	for ( int i = 0; i < 2; i++ )
		CreateNewThread_Internal( i );

	watchdog_thread_handle = CreateThread( 0, 0, WatchDogThreadProcedure_stub, this, 0, 0 );
}

void ThreadPool::Kill()
{
	SetEvent( killswitch );
	WaitForSingleObject( watchdog_thread_handle, INFINITE );
	CloseHandle( watchdog_thread_handle );

	for ( ThreadID *l_thread : threads )
	{
		l_thread->Kill();
		delete l_thread;
	}

	CloseHandle( killswitch );

	for ( int i = 0; i < THREAD_TYPES; i++ )
		CloseHandle( max_load_event[ i ] );
}

DWORD ThreadPool::WatchDogThreadProcedure_stub( LPVOID param )
{
	ThreadPool *_this = (ThreadPool *)param;
	return _this->WatchDogThreadProcedure();
}


/* 
watchdog will get woken up when number of available threads hits zero
it creates a new thread, sleeps for a bit to let things "settle" and then reset the event
it will need a copy of all "any-thread" handles to build the new thread, and will need to manage in a thread safe way 
(so a new thread doesn't "miss" a handle that is added in the interim)
*/
DWORD CALLBACK ThreadPool::WatchDogThreadProcedure()
{
	// we ignore the max load event for reserved threads
	HANDLE events[ 3 ] = { killswitch, max_load_event[ TYPE_MT ], max_load_event[ TYPE_STA ] };

	while ( 1 )
	{
		DWORD ret = WaitForMultipleObjects( 3, events, FALSE, INFINITE );
		if ( ret == WAIT_OBJECT_0 )
		{
			break;
		}
		else if ( ret == WAIT_OBJECT_0 + 1 || ret == WAIT_OBJECT_0 + 2 )
		{
			int thread_type = ret - ( WAIT_OBJECT_0 + 1 );
			// this signal is for "full thread load reached"

			// lets make sure we're actually at max capacity
			Sleep( 10 ); // sleep a bit
			if ( num_threads_available[ thread_type ] != 0 ) // see if we're still fully-loaded
				continue;

			guard.Lock();
			CreateNewThread_Internal( thread_type );
			guard.Unlock();

			Sleep( 250 ); // give the system time to 'settle down' so we don't spawn a ton of threads in a row

			ResetEvent( max_load_event[ thread_type ] );
		}
	}

	return 0;
}

ThreadID *ThreadPool::ReserveThread( int flags )
{

	// first, check to see if there's any released threads we can grab
	Nullsoft::Utility::AutoLock threadlock( guard );
	for ( ThreadID *t : threads )
	{
		if ( t->IsReserved() && t->IsReleased() && t->CanRunCOM( flags ) )
		{
			t->Reserve();
			return t;
		}
	}

	// TODO: if there are enough free threads available, mark one as reserved
// this will involve signalling the thread to switch to 'reserved' mode
// swapping out the 'function list' semaphore with a local one
// and removing all 'busy handles' from the queue
// can probably use the 'wake' handle to synchronize this

/*
int thread_type = GetThreadType(flags);
if (num_threads_available[thread_type > 2])
{
	for (size_t i=0;i!=threads.size();i++)
	{
		if (threads[i]->IsReserved() == false && threads[i]->CanRunCOM(flags))
		{

		}
	}
}
*/

	ThreadID *new_thread = CreateNewThread_Internal( GetThreadType( flags, 1 ) );

	return new_thread;
}

void ThreadPool::ReleaseThread( ThreadID *thread_id )
{
	if ( thread_id )
	{
		// lock so there's no race condition between ReserveThread() and ReleaseThread()
		Nullsoft::Utility::AutoLock threadlock( guard );
		thread_id->Release();
	}
}


int ThreadPool::AddHandle( ThreadID *thread_id, HANDLE handle, api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id, int flags )
{
	// TODO: need to ensure that handles are not duplicated
	thread_functions.Add( handle, func, user_data, id );

	if ( thread_id )
	{
		if ( thread_id->CanRunCOM( flags ) )
			thread_id->AddHandle( handle );
		else
			return 1;
		return 0;
	}
	else
	{
		/* increment thread counts temporarily - because the massive wake-up
		causes thread counts to go to 0 */
		for ( int i = 0; i < THREAD_TYPES; i++ )
			InterlockedIncrement( &num_threads_available[ i ] );

		guard.Lock();

		AddHandle_Internal( 0, handle, flags );

		bool thread_types[ THREAD_TYPES ];
		GetThreadTypes( flags, thread_types );

		for ( int i = 0; i < THREAD_TYPES; i++ )
		{
			if ( thread_types[ i ] )
				any_thread_handles[ i ].push_back( handle );
		}

		guard.Unlock();

		for ( int i = 0; i < THREAD_TYPES; i++ )
			InterlockedDecrement( &num_threads_available[ i ] );


	}

	return 0;
}

/* helper functions for adding/removing handles,
we keep going through the list as long as we can add/remove immediately.
once we have to block, we recurse the function starting at the next handle
when the function returns, we wait.
this lets us do some work rather than sit and wait for each thread's lock */
void ThreadPool::RemoveHandle_Internal(size_t start, HANDLE handle)
{
	for (;start!=threads.size();start++)
	{
		ThreadID *t = threads[start];
		if (!t->TryRemoveHandle(handle)) // try to remove
		{
			// have to wait
			RemoveHandle_Internal(start+1, handle); // recurse start with the next thread
			t->WaitRemoveHandle(handle); // finish the job
			return;
		}
	}	
}

void ThreadPool::AddHandle_Internal(size_t start, HANDLE handle, int flags)
{
	for (;start<threads.size();start++)
	{
		ThreadID *t = threads[start];
		if ((flags & api_threadpool::FLAG_LONG_EXECUTION) && t->IsReserved())
			continue;

		if (!t->CanRunCOM(flags))
			continue;

		if (!t->TryAddHandle(handle)) // try to add
		{
			// have to wait, 
			AddHandle_Internal(start+1, handle, flags); // recurse start with the next thread
			t->WaitAddHandle(handle); // finish the job
			return;
		}
	}	
}

void ThreadPool::RemoveHandle(ThreadID *thread_id, HANDLE handle)
{
	if (thread_id)
	{
		thread_id->RemoveHandle(handle);
	}
	else
	{
		/* increment thread counts temporarily - because the massive wake-up
		causes thread counts to go to 0 */
		for (int i=0;i<THREAD_TYPES;i++)
			InterlockedIncrement(&num_threads_available[i]);
		guard.Lock();
		RemoveHandle_Internal(0, handle);

		for (int j=0;j<THREAD_TYPES;j++)
		{
			//for (ThreadPoolTypes::HandleList::iterator itr = any_thread_handles[j].begin();
			//	itr != any_thread_handles[j].end();
			//	itr++)

			ThreadPoolTypes::HandleList::iterator itr = any_thread_handles[j].begin();
			while(itr != any_thread_handles[j].end())
			{
				if (*itr == handle)
				{
					itr = any_thread_handles[j].erase(itr);
				}
				else
				{
					itr++;
				}
			}
		}
		guard.Unlock();
		for (int i=0;i<THREAD_TYPES;i++)
			InterlockedDecrement(&num_threads_available[i]);
	}
}

int ThreadPool::RunFunction(ThreadID *threadid, api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id, int flags)
{
	if (threadid)
		threadid->QueueFunction(func, user_data, id);
	else
		thread_functions.QueueFunction(func, user_data, id);
	return 0;
}

ThreadID *ThreadPool::CreateNewThread_Internal(int thread_type)
{
	int reserved=0;
	int com_type = api_threadpool::FLAG_REQUIRE_COM_MT; // default
	switch(thread_type)
	{
	case TYPE_STA_RESERVED:
		reserved=1;
	case TYPE_STA:
		com_type = api_threadpool::FLAG_REQUIRE_COM_STA; 
		break;
	case TYPE_MT_RESERVED:
		reserved=1;
	case TYPE_MT:	
		com_type = api_threadpool::FLAG_REQUIRE_COM_MT; 
		break;
	}

	Nullsoft::Utility::AutoLock threadlock(guard); // lock here (rather than after new ThreadID) to protect any_thread_handles
	ThreadID *new_thread = new ThreadID(&thread_functions, killswitch, thread_functions.functions_semaphore, 
		any_thread_handles[thread_type], 
		&num_threads_available[thread_type], max_load_event[thread_type], 
		reserved, com_type);	
	threads.push_back(new_thread);
	return new_thread;
}

size_t ThreadPool::GetNumberOfThreads()
{
	Nullsoft::Utility::AutoLock threadlock(guard); 
	return threads.size();
}

size_t ThreadPool::GetNumberOfActiveThreads()
{
	size_t numThreads = GetNumberOfThreads();
	for (int i=0;i<THREAD_TYPES;i++)
		numThreads -= num_threads_available[i];
	return numThreads;
}

int ThreadPool::GetThreadType(int flags, int reserved)
{
	flags &= api_threadpool::MASK_COM_FLAGS;
	int thread_type=TYPE_MT;
	switch(flags)
	{
	case api_threadpool::FLAG_REQUIRE_COM_STA:
		thread_type = reserved?TYPE_STA_RESERVED:TYPE_STA;
		break;
	case 0: // default
	case api_threadpool::FLAG_REQUIRE_COM_MT:
		thread_type = reserved?TYPE_MT_RESERVED:TYPE_MT;
		break;
	}

	return thread_type;
}

void ThreadPool::GetThreadTypes(int flags, bool types[THREAD_TYPES])
{
	for (int i=0;i<THREAD_TYPES;i++)
	{
		types[i]=true;
	}

	if (flags & api_threadpool::FLAG_REQUIRE_COM_STA)
	{
		types[TYPE_MT] = false;
		types[TYPE_MT] = false;
	}

	if (flags & api_threadpool::FLAG_REQUIRE_COM_STA)
	{
		types[TYPE_STA] = false;
		types[TYPE_STA_RESERVED] = false;
	}

	if (flags & api_threadpool::FLAG_LONG_EXECUTION)
	{
		types[TYPE_STA_RESERVED] = false;
		types[TYPE_MT_RESERVED] = false;
	}

}
#define CBCLASS ThreadPool
START_DISPATCH;
CB(RESERVETHREAD, ReserveThread)
VCB(RELEASETHREAD, ReleaseThread)
CB(ADDHANDLE, AddHandle)
VCB(REMOVEHANDLE, RemoveHandle)
CB(RUNFUNCTION, RunFunction)
CB(GETNUMBEROFTHREADS, GetNumberOfThreads)
CB(GETNUMBEROFACTIVETHREADS, GetNumberOfActiveThreads)
END_DISPATCH;
#undef CBCLASS
