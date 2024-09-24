#ifndef NU_THREADPOOL_TIMERHANDLE_H
#define NU_THREADPOOL_TIMERHANDLE_H

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x400)
#error Must define _WIN32_WINNT >= 0x400 to use TimerHandle
#endif

#include <windows.h>
#include <bfc/platform/types.h>
/*
TimerHandle() constructor will make a new timer handle
TimerHandle(existing_handle) will "take over" an existing handle
~TimerHandle() DOES NOT CloseHandle as this object is meant as a helper
call Close() to kill the timer handle

The timer will be "one shot" auto-reset.
Because it is meant to be compatible with the threadpool, manual-reset timers and periodic timers
are not recommended!!  You will have re-entrancy problems
If you want "periodic" behavior, call Wait() at the end of your ThreadPoolFunc
*/
class TimerHandle
{
public:
	TimerHandle()                                                     { timerHandle = CreateWaitableTimer( 0, FALSE, 0 ); }
	TimerHandle( HANDLE p_handle )                                    { timerHandle = p_handle; }

	void Close()                                                      { CloseHandle( timerHandle ); }

	void Wait( uint64_t p_milliseconds )
	{
		/* MSDN notes about SetWaitableTimer: 100 nanosecond resolution, Negative values indicate relative time*/
		LARGE_INTEGER timeout = { 0 };
		timeout.QuadPart = -( (int64_t)p_milliseconds * 1000LL /*to microseconds*/ * 10LL /* to 100 nanoseconds */ );
		SetWaitableTimer( timerHandle, &timeout, 0, 0, 0, FALSE );
	}

	void Poll( uint64_t p_milliseconds ) // only use on a reserved thread!!! 
	{
		/* MSDN notes about SetWaitableTimer: 100 nanosecond resolution, Negative values indicate relative time*/
		LARGE_INTEGER timeout = { 0 };
		timeout.QuadPart = -( (int64_t)p_milliseconds * 1000LL /*to microseconds*/ * 10LL /* to 100 nanoseconds */ );
		SetWaitableTimer( timerHandle, &timeout, (LONG)p_milliseconds, 0, 0, FALSE );
	}

	/* TODO: WaitUntil method for absolute times */

	void Cancel()                                                     { CancelWaitableTimer( timerHandle ); }
	operator HANDLE()                                                 { return timerHandle; }

private:
	HANDLE timerHandle;
};

#endif  // !NU_THREADPOOL_TIMERHANDLE_H