#ifndef NULLSOFT_WAC_DOWNLOAD_MANAGER_API_DOWNLOADMANAGER_H
#define NULLSOFT_WAC_DOWNLOAD_MANAGER_API_DOWNLOADMANAGER_H

#include "bfc/dispatch.h"

typedef void *DownloadToken;

class ifc_downloadManagerCallback;
class api_httpreceiver;

class ifc_downloadManagerCallback : public Dispatchable
{
public:
	void OnFinish( DownloadToken token );
	void OnTick( DownloadToken token );
	void OnError( DownloadToken token, int error );
	void OnCancel( DownloadToken token );
	void OnConnect( DownloadToken token );
	void OnInit( DownloadToken token );
	void OnData( DownloadToken token, void *data, size_t datalen );

	int  GetSource( wchar_t *source, size_t source_cch );
	int  GetTitle( wchar_t *title, size_t title_cch );
	int  GetLocation( wchar_t *location, size_t location_cch );

	DISPATCH_CODES
	{
		IFC_DOWNLOADMANAGERCALLBACK_ONFINISH    =  10,
		IFC_DOWNLOADMANAGERCALLBACK_ONTICK      =  20,
		IFC_DOWNLOADMANAGERCALLBACK_ONERROR     =  30,
		IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL    =  40,
		IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT   =  50,
		IFC_DOWNLOADMANAGERCALLBACK_ONINIT      =  60,
		IFC_DOWNLOADMANAGERCALLBACK_ONDATA      =  70,
		IFC_DOWNLOADMANAGERCALLBACK_GETSOURCE   =  80,
		IFC_DOWNLOADMANAGERCALLBACK_GETTITLE    =  90,
		IFC_DOWNLOADMANAGERCALLBACK_GETLOCATION = 100,
	};
};

inline void ifc_downloadManagerCallback::OnFinish( DownloadToken token )
{
	_voidcall( IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, token );
}

inline void ifc_downloadManagerCallback::OnTick( DownloadToken token )
{
	_voidcall( IFC_DOWNLOADMANAGERCALLBACK_ONTICK, token );
}

inline void ifc_downloadManagerCallback::OnError( DownloadToken token, int error )
{
	_voidcall( IFC_DOWNLOADMANAGERCALLBACK_ONERROR, token, error );
}

inline void ifc_downloadManagerCallback::OnCancel( DownloadToken token )
{
	_voidcall( IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL, token );
}

inline void ifc_downloadManagerCallback::OnConnect( DownloadToken token )
{
	_voidcall( IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT, token );
}

inline void ifc_downloadManagerCallback::OnInit( DownloadToken token )
{
	_voidcall( IFC_DOWNLOADMANAGERCALLBACK_ONINIT, token );
}

inline void ifc_downloadManagerCallback::OnData( DownloadToken token, void *data, size_t datalen )
{
	_voidcall( IFC_DOWNLOADMANAGERCALLBACK_ONDATA, token, data, datalen );
}

inline int ifc_downloadManagerCallback::GetSource( wchar_t *source, size_t source_cch )
{
	return _call( IFC_DOWNLOADMANAGERCALLBACK_GETSOURCE, (int)1, source, source_cch );
}

inline int ifc_downloadManagerCallback::GetTitle( wchar_t *title, size_t title_cch )
{
	return _call( IFC_DOWNLOADMANAGERCALLBACK_GETTITLE, (int)1, title, title_cch );
}

inline int ifc_downloadManagerCallback::GetLocation( wchar_t *location, size_t location_cch )
{
	return _call( IFC_DOWNLOADMANAGERCALLBACK_GETLOCATION, (int)1, location, location_cch );
}


class api_downloadManager : public Dispatchable
{
public:
	DownloadToken     Download( const char *url, ifc_downloadManagerCallback *callback );
	DownloadToken     DownloadEx( const char *url, ifc_downloadManagerCallback *callback, int flags );

	api_httpreceiver *GetReceiver( DownloadToken token );
	const wchar_t    *GetLocation( DownloadToken token );
	void              SetLocation( DownloadToken token, const wchar_t *p_location );


	const char       *GetExtention( DownloadToken token );
	const char       *GetUrl( DownloadToken token );

	int               GetBuffer( DownloadToken token, void **buffer, size_t *bufferLength );
	uint64_t          GetBytesDownloaded( DownloadToken token );

	void              ResumePendingDownload( DownloadToken token );
	void              CancelDownload( DownloadToken token );
	void              RetainDownload( DownloadToken token );
	void              ReleaseDownload( DownloadToken token );

	/* added in 5.58 */
	void              RegisterStatusCallback( ifc_downloadManagerCallback *callback );
	void              UnregisterStatusCallback( ifc_downloadManagerCallback *callback );

	const wchar_t    *GetSource( DownloadToken token );
	const wchar_t    *GetTitle( DownloadToken token );

	bool              IsPending( DownloadToken token );

	DISPATCH_CODES
	{
		API_DOWNLOADMANAGER_DOWNLOAD                 =  10,
		API_DOWNLOADMANAGER_DOWNLOADEX               =  20,
		API_DOWNLOADMANAGER_GETRECEIVER              = 100,
		API_DOWNLOADMANAGER_GETLOCATION              = 110,
		API_DOWNLOADMANAGER_SETLOCATION              = 112,
		API_DOWNLOADMANAGER_GETEXTENTION             = 115,
		API_DOWNLOADMANAGER_GETURL                   = 117,
		API_DOWNLOADMANAGER_GETBYTESDOWNLOADED       = 120,
		API_DOWNLOADMANAGER_GETBUFFER                = 130,
		API_DOWNLOADMANAGER_CANCELDOWNLOAD           = 140,
		API_DOWNLOADMANAGER_RETAINDOWNLOAD           = 150,
		API_DOWNLOADMANAGER_RELEASEDOWNLOAD          = 160,
		API_DOWNLOADMANAGER_REGISTERSTATUSCALLBACK   = 170,
		API_DOWNLOADMANAGER_UNREGISTERSTATUSCALLBACK = 180,
		API_DOWNLOADMANAGER_GETSOURCE                = 190,
		API_DOWNLOADMANAGER_GETTITLE                 = 200,
		API_DOWNLOADMANAGER_RESUMEPENDINGDOWNLOAD    = 210,
		API_DOWNLOADMANAGER_ISPENDING                = 220,
	};

	enum
	{
		DOWNLOADEX_TEMPFILE            = 0,      // download as a temporary file
		DOWNLOADEX_BUFFER              = 1,      // download to memory
		DOWNLOADEX_CALLBACK            = 2,      // send data to OnData callback
		DOWNLOADEX_MASK_DOWNLOADMETHOD = 0x3,
		DOWNLOADEX_PENDING             = 0xF00,
		DOWNLOADEX_UI                  = 0xF000, // show up in the download manager UI
	};

	enum
	{
		TICK_NODATA       = -2, // not necessarily an error, just means no data this time around
		TICK_FINISHED     = -1,
		TICK_SUCCESS      = 0,
		TICK_FAILURE      = 1,
		TICK_TIMEOUT      = 2,
		TICK_CANT_CONNECT = 3,
		TICK_WRITE_ERROR  = 4,
		TICK_CONNECTING   = 5,
		TICK_CONNECTED    = 6,

	};
};


inline DownloadToken api_downloadManager::Download( const char *url, ifc_downloadManagerCallback *callback )
{
	return _call( API_DOWNLOADMANAGER_DOWNLOAD, (DownloadToken *)0, url, callback );
}

inline DownloadToken api_downloadManager::DownloadEx( const char *url, ifc_downloadManagerCallback *callback, int flags )
{
	return _call( API_DOWNLOADMANAGER_DOWNLOADEX, (DownloadToken *)0, url, callback, flags );
}

inline api_httpreceiver *api_downloadManager::GetReceiver( DownloadToken token )
{
	return _call( API_DOWNLOADMANAGER_GETRECEIVER, (api_httpreceiver *)0, token );
}

inline const wchar_t *api_downloadManager::GetLocation( DownloadToken token )
{
	return _call( API_DOWNLOADMANAGER_GETLOCATION, (const wchar_t *)0, token );
}

inline void api_downloadManager::SetLocation( DownloadToken token, const wchar_t *p_location )
{
	_voidcall( API_DOWNLOADMANAGER_SETLOCATION, token, p_location );
}

inline const char *api_downloadManager::GetExtention( DownloadToken token )
{
	return _call( API_DOWNLOADMANAGER_GETEXTENTION, (const char *)0, token );
}

inline const char *api_downloadManager::GetUrl( DownloadToken token )
{
	return _call( API_DOWNLOADMANAGER_GETURL, (const char *)0, token );
}

inline uint64_t api_downloadManager::GetBytesDownloaded( DownloadToken token )
{
	return _call( API_DOWNLOADMANAGER_GETBYTESDOWNLOADED, (uint64_t)0, token );
}

inline int api_downloadManager::GetBuffer( DownloadToken token, void **buffer, size_t *bufferLength )
{
	return _call( API_DOWNLOADMANAGER_GETBUFFER, (int)1, token, buffer, bufferLength );
}

inline void api_downloadManager::ResumePendingDownload( DownloadToken token )
{
	_voidcall( API_DOWNLOADMANAGER_RESUMEPENDINGDOWNLOAD, token );
}

inline void api_downloadManager::CancelDownload( DownloadToken token )
{
	_voidcall( API_DOWNLOADMANAGER_CANCELDOWNLOAD, token );
}

inline void api_downloadManager::RetainDownload( DownloadToken token )
{
	_voidcall( API_DOWNLOADMANAGER_RETAINDOWNLOAD, token );
}

inline void api_downloadManager::ReleaseDownload( DownloadToken token )
{
	_voidcall( API_DOWNLOADMANAGER_RELEASEDOWNLOAD, token );
}


inline void api_downloadManager::RegisterStatusCallback( ifc_downloadManagerCallback *callback )
{
	_voidcall( API_DOWNLOADMANAGER_REGISTERSTATUSCALLBACK, callback );
}

inline void api_downloadManager::UnregisterStatusCallback( ifc_downloadManagerCallback *callback )
{
	_voidcall( API_DOWNLOADMANAGER_UNREGISTERSTATUSCALLBACK, callback );
}


inline const wchar_t *api_downloadManager::GetSource( DownloadToken token )
{
	return _call( API_DOWNLOADMANAGER_GETSOURCE, (const wchar_t *)0, token );
}

inline const wchar_t *api_downloadManager::GetTitle( DownloadToken token )
{
	return _call( API_DOWNLOADMANAGER_GETTITLE, (const wchar_t *)0, token );
}

inline bool api_downloadManager::IsPending( DownloadToken token )
{
	return _call( API_DOWNLOADMANAGER_ISPENDING, (bool)0, token );
}


// {9E5E732A-C612-489d-AB52-1501E1AF1710}
static const GUID DownloadManagerGUID =
{ 0x9e5e732a, 0xc612, 0x489d, { 0xab, 0x52, 0x15, 0x1, 0xe1, 0xaf, 0x17, 0x10 } };

extern api_downloadManager *g_downloadManagerApi;

#ifndef WAC_API_DOWNLOADMANAGER
#define WAC_API_DOWNLOADMANAGER g_downloadManagerApi
#endif // !WAC_API_DOWNLOADMANAGER

#endif // !NULLSOFT_WAC_DOWNLOAD_MANAGER_API_DOWNLOADMANAGER_H
