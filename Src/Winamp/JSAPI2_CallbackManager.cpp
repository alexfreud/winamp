#include "JSAPI2_CallbackManager.h"
#include "JSAPI2_TransportAPI.h"
#include "JSAPI2_AsyncDownloader.h"
#include "JSAPI2_MediaCore.h"
#include "api.h"

JSAPI2::CallbackManager JSAPI2::callbackManager;

JSAPI2::CallbackManager::CallbackManager() : callbackGuard("JSAPI2::CallbackManager::callbackGuard")
{}

void JSAPI2::CallbackManager::Register( JSAPI2::TransportAPI *me )
{
	/* benski> important note:
	even thought JSAPI2::Transport inherits from IUnknown,
	we don't call AddRef here!
	because this would introduce a circular reference.
	JSAPI2::TransportAPI will call Deregister during it's
	destructor.
	*/
	Nullsoft::Utility::AutoLock lock( callbackGuard );
	transports.push_back( new TransportCallback( me ) );
}

void JSAPI2::CallbackManager::Deregister( JSAPI2::TransportAPI *me )
{
	/* benski> important note:
	even thought JSAPI2::Transport inherits from IUnknown,
	we don't call Release here!
	because this would introduce a circular reference.
	JSAPI2::TransportAPI will call Deregister during it's
	destructor.
	*/
	Nullsoft::Utility::AutoLock lock( callbackGuard );
	for ( size_t i = 0; i != transports.size(); i++ )
	{
		TransportCallback *callback = transports[ i ];
		if ( callback->api == me )
		{
			delete callback;
			transports.erase( transports.begin() + i );
			i--;
		}
	}
}


void JSAPI2::CallbackManager::Register( JSAPI2::MediaCoreAPI *me )
{
	Nullsoft::Utility::AutoLock lock( callbackGuard );

	//if (!mediaCores.contains(me))
	if ( mediaCores.end() == std::find( mediaCores.begin(), mediaCores.end(), me ) )
	{
		mediaCores.push_back( me );
	}
}

void JSAPI2::CallbackManager::Deregister( JSAPI2::MediaCoreAPI *me )
{
	Nullsoft::Utility::AutoLock lock( callbackGuard );

	auto it = mediaCores.begin();
	while ( it != mediaCores.end() )
	{
		if ( *it != me )
		{
			it++;
			continue;
		}

		it = mediaCores.erase( it );
	}
}


void JSAPI2::CallbackManager::Register( JSAPI2::AsyncDownloaderAPI *me )
{
	Nullsoft::Utility::AutoLock lock( callbackGuard );
	asyncDownloaders.push_back( new AsyncDownloaderCallback( me ) );
}

void JSAPI2::CallbackManager::Deregister( JSAPI2::AsyncDownloaderAPI *me )
{
	Nullsoft::Utility::AutoLock lock( callbackGuard );
	for ( size_t i = 0; i != asyncDownloaders.size(); i++ )
	{
		AsyncDownloaderCallback *callback = asyncDownloaders[ i ];
		if ( callback->api == me )
		{
			delete callback;
			asyncDownloaders.erase( asyncDownloaders.begin() + i );
			i--;
		}
	}
}

/* --- OnStop --- */
struct OnStopAPCData
{
	JSAPI2::TransportAPI *transport;
	int position;
	int is_full_stop;
};

static void CALLBACK CMGR_OnStopAPC(ULONG_PTR param)
{
	OnStopAPCData *data = (OnStopAPCData *)param;
	data->transport->OnStop(data->position, data->is_full_stop);
	data->transport->Release();
	delete data;
}

void JSAPI2::CallbackManager::OnStop(int position, int is_full_stop)
{
	DWORD threadId = GetCurrentThreadId();
	Nullsoft::Utility::AutoLock lock(callbackGuard);

	for ( TransportCallback *l_transport : transports )
	{
		OnStopAPCData *data = new OnStopAPCData;
		data->transport    = l_transport->api;
		data->position     = position;
		data->is_full_stop = is_full_stop;

		data->transport->AddRef(); // so it doesn't disappear while we're switching threads

		if ( threadId == l_transport->threadId )
		{
			// same thread! huzzah but I wonder how that happened :)
			CMGR_OnStopAPC( (ULONG_PTR)data );
		}
		else
		{
			// different thread, do an APC
			if ( QueueUserAPC( CMGR_OnStopAPC, l_transport->threadHandle, (ULONG_PTR)data ) == 0 )
			{
				data->transport->Release();
				delete data;
			}
		}
	}
}
/* --- --- */

/* --- OnPlay --- */
struct OnPlayAPC
{
	JSAPI2::TransportAPI *transport;
	wchar_t *filename;
};

static void CALLBACK CMGR_OnPlayAPC(ULONG_PTR param)
{
	OnPlayAPC *data = (OnPlayAPC *)param;
	data->transport->OnPlay(data->filename);

	free(data->filename);

	data->transport->Release();
	delete data;
}

void JSAPI2::CallbackManager::OnPlay(const wchar_t *filename)
{
	DWORD threadId = GetCurrentThreadId();
	Nullsoft::Utility::AutoLock lock(callbackGuard);

	for ( TransportCallback *l_transport : transports )
	{
		OnPlayAPC *data = new OnPlayAPC;
		data->transport = l_transport->api;
		data->filename  = _wcsdup(filename);

		data->transport->AddRef(); // so it doesn't disappear while we're switching threads

		if ( threadId == l_transport->threadId )
		{
			// same thread! huzzah but I wonder how that happened :)
			CMGR_OnPlayAPC( (ULONG_PTR)data );
		}
		else
		{
			// different thread, do an APC
			if ( QueueUserAPC( CMGR_OnPlayAPC, l_transport->threadHandle, (ULONG_PTR)data ) == 0 )
			{
				data->transport->Release();
				free( data->filename );
				delete data;
			}
		}
	}
}

/* --- --- */


struct OnPauseAPC
{
	JSAPI2::TransportAPI *transport;
	bool pause_state;
};

static void CALLBACK CMGR_OnPauseAPC(ULONG_PTR param)
{
	OnPauseAPC *data = (OnPauseAPC *)param;
	data->transport->OnPause(data->pause_state);
	data->transport->Release();
	delete data;
}

void JSAPI2::CallbackManager::OnPause(bool pause_state)
{
	DWORD threadId = GetCurrentThreadId();
	Nullsoft::Utility::AutoLock lock( callbackGuard );

	for ( TransportCallback *l_transport : transports )
	{
		OnPauseAPC *data = new OnPauseAPC;
		data->transport   = l_transport->api;
		data->pause_state = pause_state;

		data->transport->AddRef(); // so it doesn't disappear while we're switching threads

		if (threadId == l_transport->threadId)
		{
			// same thread! huzzah but I wonder how that happened :)
			CMGR_OnPauseAPC((ULONG_PTR)data);
		}
		else
		{
			// different thread, do an APC
			if (QueueUserAPC(CMGR_OnPauseAPC, l_transport->threadHandle, (ULONG_PTR)data) == 0)
			{
				data->transport->Release();
				delete data;
			}
		}
	}
}

/* --- --- */
bool JSAPI2::CallbackManager::OverrideMetadata( const wchar_t *filename, const wchar_t *tag, wchar_t *out, size_t outCch )
{
	if ( NULL != filename && NULL != tag && NULL != out )
	{
		Nullsoft::Utility::AutoLock lock( callbackGuard );
		for ( MediaCoreAPI *l_mediaCore : mediaCores )
		{
			if ( l_mediaCore->OverrideMetadata( filename, tag, out, outCch ) )
				return true;
		}
	}

	return false;
}


/* --- OnInit --- */
struct OnInitAPC
{
	JSAPI2::AsyncDownloaderAPI *asyncDownloader;
	wchar_t *url;
};

static void CALLBACK CMGR_OnInitAPC(ULONG_PTR param)
{
	OnInitAPC *data = (OnInitAPC *)param;
	data->asyncDownloader->OnInit(data->url);
	free(data->url);
	data->asyncDownloader->Release();
	delete data;
}

void JSAPI2::CallbackManager::OnInit( const wchar_t *url, const wchar_t *onlinesvcId )
{
	DWORD threadId = GetCurrentThreadId();
	Nullsoft::Utility::AutoLock lock( callbackGuard );

	for ( AsyncDownloaderCallback *l_downloader : asyncDownloaders )
	{
		if ( wcscmp( onlinesvcId, l_downloader->api->GetKey() ) )
			continue;	//only call back to the same online service that issued the download reqeust

		OnInitAPC *data = new OnInitAPC;
		data->asyncDownloader = l_downloader->api;
		data->url             = _wcsdup( url );

		data->asyncDownloader->AddRef(); // so it doesn't disappear while we're switching threads

		if ( threadId == l_downloader->threadId )
		{
			// same thread! huzzah but I wonder how that happened :)
			CMGR_OnInitAPC( (ULONG_PTR)data );
		}
		else
		{
			// different thread, do an APC
			if ( QueueUserAPC( CMGR_OnInitAPC, l_downloader->threadHandle, (ULONG_PTR)data ) == 0 )
			{
				data->asyncDownloader->Release();
				free( data->url );
				delete data;
			}
		}

	}
}


/* --- OnConnect --- */
struct OnConnectAPC
{
	JSAPI2::AsyncDownloaderAPI *asyncDownloader;
	wchar_t *url;
};

static void CALLBACK CMGR_OnConnectAPC(ULONG_PTR param)
{
	OnConnectAPC *data = (OnConnectAPC *)param;
	data->asyncDownloader->OnConnect(data->url);
	free(data->url);
	data->asyncDownloader->Release();
	delete data;
}

void JSAPI2::CallbackManager::OnConnect( const wchar_t *url, const wchar_t *onlinesvcId )
{
	DWORD threadId = GetCurrentThreadId();
	Nullsoft::Utility::AutoLock lock( callbackGuard );

	for ( AsyncDownloaderCallback *l_downloader : asyncDownloaders )
	{
		if ( wcscmp( onlinesvcId, l_downloader->api->GetKey() ) )
			continue;	//only call back to the same online service that issued the download reqeust

		OnConnectAPC *data = new OnConnectAPC;
		data->asyncDownloader = l_downloader->api;
		data->url             = _wcsdup( url );

		data->asyncDownloader->AddRef(); // so it doesn't disappear while we're switching threads

		if ( threadId == l_downloader->threadId )
		{
			// same thread! huzzah but I wonder how that happened :)
			CMGR_OnConnectAPC( (ULONG_PTR)data );
		}
		else
		{
			// different thread, do an APC
			if ( QueueUserAPC( CMGR_OnConnectAPC, l_downloader->threadHandle, (ULONG_PTR)data ) == 0 )
			{
				data->asyncDownloader->Release();
				free( data->url );
				delete data;
			}
		}
	}
}


/* --- OnCancel --- */
struct OnCancelAPC
{
	JSAPI2::AsyncDownloaderAPI *asyncDownloader;
	wchar_t *url;
};

static void CALLBACK CMGR_OnCancelAPC(ULONG_PTR param)
{
	OnCancelAPC *data = (OnCancelAPC *)param;
	data->asyncDownloader->OnCancel(data->url);
	free(data->url);
	data->asyncDownloader->Release();
	delete data;
}

void JSAPI2::CallbackManager::OnCancel( const wchar_t *url, const wchar_t *onlinesvcId )
{
	DWORD threadId = GetCurrentThreadId();
	Nullsoft::Utility::AutoLock lock( callbackGuard );

	for ( AsyncDownloaderCallback *l_downloader : asyncDownloaders )
	{
		if ( wcscmp( onlinesvcId, l_downloader->api->GetKey() ) )
			continue;	//only call back to the same online service that issued the download reqeust

		OnCancelAPC *data = new OnCancelAPC;
		data->asyncDownloader = l_downloader->api;
		data->url             = _wcsdup( url );

		data->asyncDownloader->AddRef(); // so it doesn't disappear while we're switching threads

		if ( threadId == l_downloader->threadId )
		{
			// same thread! huzzah but I wonder how that happened :)
			CMGR_OnCancelAPC( (ULONG_PTR)data );
		}
		else
		{
			// different thread, do an APC
			if ( QueueUserAPC( CMGR_OnCancelAPC, l_downloader->threadHandle, (ULONG_PTR)data ) == 0 )
			{
				data->asyncDownloader->Release();
				free( data->url );
				delete data;
			}
		}
	}
}


/* --- OnData --- */
struct OnDataAPC
{
	JSAPI2::AsyncDownloaderAPI *asyncDownloader;
	wchar_t *url;
	size_t downloadedlen;
	size_t totallen;
};

static void CALLBACK CMGR_OnDataAPC(ULONG_PTR param)
{
	OnDataAPC *data = (OnDataAPC *)param;
	data->asyncDownloader->OnData(data->url, data->downloadedlen, data->totallen);
	free(data->url);
	data->asyncDownloader->Release();
	delete data;
}

void JSAPI2::CallbackManager::OnData(const wchar_t *url, size_t downloadedlen, size_t totallen, const wchar_t *onlinesvcId)
{
	DWORD threadId = GetCurrentThreadId();
	Nullsoft::Utility::AutoLock lock(callbackGuard);

	for ( AsyncDownloaderCallback *downloader : asyncDownloaders )
	{
		if ( wcscmp(onlinesvcId, downloader->api->GetKey()) )
			continue;	//only call back to the same online service that issued the download reqeust

		OnDataAPC *data = new OnDataAPC;
		data->asyncDownloader = downloader->api;
		data->url = _wcsdup(url);
		data->downloadedlen = downloadedlen;
		data->totallen = totallen;
		data->asyncDownloader->AddRef(); // so it doesn't disappear while we're switching threads
		if (threadId == downloader->threadId)
		{
			// same thread! huzzah but I wonder how that happened :)
			CMGR_OnDataAPC((ULONG_PTR)data);
		}
		else
		{
			// different thread, do an APC
			if (QueueUserAPC(CMGR_OnDataAPC, downloader->threadHandle, (ULONG_PTR)data) == 0)
			{
				data->asyncDownloader->Release();
				free(data->url);
				delete data;
			}
		}
	}
}


/* --- OnError --- */
struct OnErrorAPC
{
	JSAPI2::AsyncDownloaderAPI *asyncDownloader;
	wchar_t *url;
	int error;
};

static void CALLBACK CMGR_OnErrorAPC(ULONG_PTR param)
{
	OnErrorAPC *data = (OnErrorAPC *)param;
	data->asyncDownloader->OnError(data->url, data->error);
	free(data->url);
	data->asyncDownloader->Release();
	delete data;
}

void JSAPI2::CallbackManager::OnError(const wchar_t *url, int error, const wchar_t *onlinesvcId)
{
	DWORD threadId = GetCurrentThreadId();
	Nullsoft::Utility::AutoLock lock(callbackGuard);

	for ( AsyncDownloaderCallback *downloader : asyncDownloaders )
	{
		if ( wcscmp(onlinesvcId, downloader->api->GetKey()) )
			continue;	//only call back to the same online service that issued the download reqeust

		OnErrorAPC *data = new OnErrorAPC;
		data->asyncDownloader = downloader->api;
		data->url = _wcsdup(url);
		data->error = error;
		data->asyncDownloader->AddRef(); // so it doesn't disappear while we're switching threads
		if (threadId == downloader->threadId)
		{
			// same thread! huzzah but I wonder how that happened :)
			CMGR_OnErrorAPC((ULONG_PTR)data);
		}
		else
		{
			// different thread, do an APC
			if (QueueUserAPC(CMGR_OnErrorAPC, downloader->threadHandle, (ULONG_PTR)data) == 0)
			{
				data->asyncDownloader->Release();
				free(data->url);
				delete data;
			}
		}
	}
}


/* --- OnFinish --- */
struct OnFinishAPC
{
	JSAPI2::AsyncDownloaderAPI *asyncDownloader;
	wchar_t *url;
	wchar_t *destfilename;
};

static void CALLBACK CMGR_OnFinishAPC(ULONG_PTR param)
{
	OnFinishAPC *data = (OnFinishAPC *)param;
	data->asyncDownloader->OnFinish(data->url, data->destfilename);
	free(data->url);
	free(data->destfilename);
	data->asyncDownloader->Release();
	delete data;
}

void JSAPI2::CallbackManager::OnFinish(const wchar_t *url, const wchar_t *destfilename, const wchar_t *onlinesvcId)
{
	DWORD threadId = GetCurrentThreadId();
	Nullsoft::Utility::AutoLock lock(callbackGuard);

	for ( AsyncDownloaderCallback *downloader : asyncDownloaders )
	{
		if ( wcscmp(onlinesvcId, downloader->api->GetKey()) )
			continue;	//only call back to the same online service that issued the download reqeust

		OnFinishAPC *data = new OnFinishAPC;
		data->asyncDownloader = downloader->api;
		data->url = _wcsdup(url);
		data->destfilename = _wcsdup(destfilename);
		data->asyncDownloader->AddRef(); // so it doesn't disappear while we're switching threads
		if (threadId == downloader->threadId)
		{
			// same thread! huzzah but I wonder how that happened :)
			CMGR_OnFinishAPC((ULONG_PTR)data);
		}
		else
		{
			// different thread, do an APC
			if (QueueUserAPC(CMGR_OnFinishAPC, downloader->threadHandle, (ULONG_PTR)data) == 0)
			{
				data->asyncDownloader->Release();
				free(data->url);
				free(data->destfilename);
				delete data;
			}
		}
	}
}
