#ifndef NULLSOFT_CLOUDH
#define NULLSOFT_CLOUDH

#include <windows.h>

#include "Feeds.h"
#include "../nu/threadpool/api_threadpool.h"
#include "../nu/threadpool/timerhandle.hpp"
#include "../nu/AutoLock.h"

class Cloud
{
public:
	Cloud();
	~Cloud();
	void Init();
	void Quit();
	void Refresh( Channel &channel );
	void GetStatus( wchar_t *status, size_t len );
	void RefreshAll();
	void Pulse()                                                      { SetEvent( cloudEvent ); }
	
private:
	static DWORD WINAPI CloudThread( void *param );
	void SetStatus( const wchar_t *newStatus );
	void Callback();
	ThreadID *cloudThread;
	wchar_t *statusText;
	Nullsoft::Utility::LockGuard statusGuard;
	HANDLE cloudEvent, cloudDone;
	TimerHandle cloudTimerEvent;
	static int CloudThreadPoolFunc( HANDLE handle, void *user_data, intptr_t param );
};

#endif
