#include <string.h>
#include <strsafe.h>

#include <iostream>
#include <cstdio>

#include <QtCore/qglobal.h>

#include <QWebEngineProfile>
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include <QWebEnginePage>
#include <QWebEngineSettings>

#include "api__wac_downloadManager.h"

#include "wac_downloadManager.h"
#include "wac_download_http_receiver_api.h"

#include "..\wac_network\wac_network_http_receiver_api.h"

#include "api/service/waservicefactory.h"

#include "../nu/threadname.h"
#include "../nu/AutoChar.h"
#include "../nu/threadpool/timerhandle.hpp"

#include "..\WAT\WAT.h"

#include "..\Winamp\buildType.h"

static const GUID internetConfigGroupGUID =
{
	0xc0a565dc, 0xcfe, 0x405a, { 0xa2, 0x7c, 0x46, 0x8b, 0xc, 0x8a, 0x3a, 0x5c }
};

#define DOWNLOAD_TIMEOUT_MS    60000 // 60 second timeout
#define DOWNLOAD_SLEEP_MS         50
#define DOWNLOAD_BUFFER_SIZE 1310720 // gives a maximum download rate of 25 mb/sec per file


/**********************************************************************************
 **********************************************************************************/


/**********************************************************************************
 *                                    PUBLIC                                      *
 **********************************************************************************/
wa::Components::WAC_DownloadData::WAC_DownloadData( api_wac_download_manager_http_receiver *p_http, const char *p_url, int p_flags, ifc_downloadManagerCallback *p_callback )
{
	_http          = p_http;

	strcpy_s( this->_url, 1024, p_url );

	_flags         = p_flags;
	_callback      = p_callback;

	if ( _callback )
		_callback->AddRef();

	_hFile         = INVALID_HANDLE_VALUE;
	_filepath[ 0 ] = 0;
	_fileext       = 0;

	int download_method = ( api_downloadManager::DOWNLOADEX_MASK_DOWNLOADMETHOD & _flags );
	switch ( download_method )
	{
		case api_downloadManager::DOWNLOADEX_TEMPFILE:
		{
			wchar_t temppath[ MAX_PATH - 14 ] = { 0 }; // MAX_PATH-14 'cause MSDN said so
			GetTempPathW( MAX_PATH - 14, temppath );
			GetTempFileNameW( temppath, L"wdl", 0, _filepath );
			_hFile = CreateFileW( _filepath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0 );
		}
		break;
		case api_downloadManager::DOWNLOADEX_CALLBACK:
			if ( _callback )
				_callback->GetLocation( _filepath, MAX_PATH );
			break;
	}

	

	_source[ 0 ] = 0;
	_title[ 0 ]  = 0;

	if ( _flags & api_downloadManager::DOWNLOADEX_CALLBACK )
	{
		if ( _callback )
		{
			_callback->GetSource( _source, 1024 );
			_callback->GetTitle( _title, 1024 );
		}
	}

	_connectionStart = _lastDownloadTick = GetTickCount();
	_last_status     = HTTP_RECEIVER_STATUS_ERROR;
	_pending         = ( _flags & api_downloadManager::DOWNLOADEX_PENDING ) > 0;
}

wa::Components::WAC_DownloadData::~WAC_DownloadData()
{
	ServiceRelease( _http, httpreceiverGUID2 );

	_http = NULL;

	if ( _fileext )
		delete _fileext;

	int download_method = ( api_downloadManager::DOWNLOADEX_MASK_DOWNLOADMETHOD & _flags );
	if ( download_method == api_downloadManager::DOWNLOADEX_TEMPFILE && _filepath[ 0 ] )
		DeleteFileW( _filepath );

	if ( _callback )
		_callback->Release();

	_callback = NULL;
}


void wa::Components::WAC_DownloadData::Retain()
{
	this->_refCount.fetch_add( 1 );
}

void wa::Components::WAC_DownloadData::Release()
{
	if ( this->_refCount.fetch_sub( 1 ) == 0 )
		delete this;
}


void wa::Components::WAC_DownloadData::Close( ifc_downloadManagerCallback **callbackCopy )
{
	if ( _hFile != INVALID_HANDLE_VALUE )
		CloseHandle( _hFile );

	_hFile = INVALID_HANDLE_VALUE;

	if ( callbackCopy != NULL )
	{
		*callbackCopy = _callback;
		if ( _callback != NULL )
			_callback->AddRef();
	}
	else if ( _callback != NULL )
	{
		_callback->Release();
		_callback = NULL;
	}

	// don't want to close http here, because someone might still want to get the headers out of it
}

bool wa::Components::WAC_DownloadData::getExtention()
{
	if ( _fileext && *_fileext )
		return _fileext;

	char l_header_name_content_type[] = "Content-Type";

	char *l_content_type = _http->getheader( l_header_name_content_type );
	if ( l_content_type && *l_content_type )
	{
		if ( _CONTENT_TYPES_EXTENSIONS.count( l_content_type ) == 1 )
			_fileext = _strdup( _CONTENT_TYPES_EXTENSIONS.find( l_content_type )->second.c_str() );
	}

	return _fileext;
}




/**********************************************************************************
 *                                    PRIVATE                                     *
 **********************************************************************************/



/**********************************************************************************
 **********************************************************************************/

/**********************************************************************************
 *                                    PUBLIC                                      *
 **********************************************************************************/
wa::Components::WAC_DownloadManager::WAC_DownloadManager( QObject *parent )	: QNetworkAccessManager( parent )
{
	this->setObjectName( "DownloadManagerService" );

	this->init();
}

wa::Components::WAC_DownloadManager::~WAC_DownloadManager()
{
	disconnect( _connection_authentication_required );
}


DownloadToken wa::Components::WAC_DownloadManager::Download( const char *p_url, ifc_downloadManagerCallback *p_callback )
{
	return DownloadEx( p_url, p_callback, api_downloadManager::DOWNLOADEX_TEMPFILE );
}

DownloadToken wa::Components::WAC_DownloadManager::DownloadEx( const char *p_url, ifc_downloadManagerCallback *p_callback, int p_flags )
{



	return DownloadToken();
}





/**********************************************************************************
 *                                    PRIVATE                                     *
 **********************************************************************************/
void wa::Components::WAC_DownloadManager::init()
{
	QString l_winamp_user_agent = QString( "%1 Winamp/%2" ).arg( QWebEngineProfile::defaultProfile()->httpUserAgent(), STR_WINAMP_PRODUCTVER ).replace( ",", "." );

	QWebEngineProfile::defaultProfile()->setHttpUserAgent( l_winamp_user_agent );


	_connection_authentication_required = connect( this, &QNetworkAccessManager::authenticationRequired,   this, &wa::Components::WAC_DownloadManager::on_s_authentication_required );


}


QNetworkReply *wa::Components::WAC_DownloadManager::createRequest( Operation p_operation, const QNetworkRequest &p_request, QIODevice *p_outgoing_data )
{
	return QNetworkAccessManager::createRequest( p_operation, p_request, p_outgoing_data );
}



/**********************************************************************************
 *                                 PRIVATE SLOTS                                  *
 **********************************************************************************/
void wa::Components::WAC_DownloadManager::on_s_authentication_required( QNetworkReply *p_reply, QAuthenticator *p_authenticator )
{
	Q_UNUSED( p_reply );
	Q_UNUSED( p_authenticator );
}





/**********************************************************************************
 **********************************************************************************/
DownloadData::DownloadData( api_httpreceiver *p_http, const char *p_url, int p_flags, ifc_downloadManagerCallback *p_callback )
{
	flags    = p_flags;
	http     = p_http;
	callback = p_callback;

	if ( callback )
		callback->AddRef();

	hFile         = INVALID_HANDLE_VALUE;
	filepath[ 0 ] = 0;
	fileext       = 0;

	int download_method = ( api_downloadManager::DOWNLOADEX_MASK_DOWNLOADMETHOD & flags );
	switch ( download_method )
	{
		case api_downloadManager::DOWNLOADEX_TEMPFILE:
		{
			wchar_t temppath[ MAX_PATH - 14 ] = { 0 }; // MAX_PATH-14 'cause MSDN said so
			GetTempPathW( MAX_PATH - 14, temppath );
			GetTempFileNameW( temppath, L"wdl", 0, filepath );
			hFile = CreateFileW( filepath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0 );
		}
		break;
		case api_downloadManager::DOWNLOADEX_CALLBACK:
			if ( callback )
				callback->GetLocation( filepath, MAX_PATH );
			break;
	}

	strcpy_s( this->url, 1024, p_url );
	source[ 0 ] = 0;
	title[ 0 ] = 0;
	if ( flags & api_downloadManager::DOWNLOADEX_CALLBACK )
	{
		if ( callback )
		{
			callback->GetSource( source, 1024 );
			callback->GetTitle( title, 1024 );
		}
	}

	connectionStart = lastDownloadTick = GetTickCount();
	last_status     = HTTPRECEIVER_STATUS_ERROR;
	pending         = ( flags & api_downloadManager::DOWNLOADEX_PENDING ) > 0;
}

DownloadData::~DownloadData()
{
	ServiceRelease( http, httpreceiverGUID );

	http = NULL;

	if ( fileext )
		delete fileext;

	int download_method = ( api_downloadManager::DOWNLOADEX_MASK_DOWNLOADMETHOD & flags );
	if ( download_method == api_downloadManager::DOWNLOADEX_TEMPFILE && filepath[ 0 ] )
		DeleteFileW( filepath );

	if ( callback )
		callback->Release();

	callback = NULL;
}


void DownloadData::Retain()
{
	this->_refCount.fetch_add( 1 );
}

void DownloadData::Release()
{
	if ( this->_refCount.fetch_sub( 1 ) == 0 )
		delete this;
}


void DownloadData::Close( ifc_downloadManagerCallback **callbackCopy )
{
	if ( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile );

	hFile = INVALID_HANDLE_VALUE;

	if ( callbackCopy != NULL )
	{
		*callbackCopy = callback;
		if ( callback != NULL )
			callback->AddRef();
	}
	else if ( callback != NULL )
	{
		callback->Release();
		callback = NULL;
	}

	// don't want to close http here, because someone might still want to get the headers out of it
}

bool DownloadData::getExtention()
{
	if ( fileext && *fileext )
		return fileext;

	char l_header_name_content_type[] = "Content-Type";
	char *l_content_type = http->getheader( l_header_name_content_type );
	if ( l_content_type && *l_content_type )
	{
		if ( _CONTENT_TYPES_EXTENSIONS.count( l_content_type ) == 1 )
			fileext = _strdup( _CONTENT_TYPES_EXTENSIONS.find( l_content_type )->second.c_str() );
	}

	return fileext;
}



/**********************************************************************************
 **********************************************************************************/

 /**********************************************************************************
 *                                    PUBLIC                                      *
 **********************************************************************************/
DownloadManager::DownloadManager()
{
	download_thread = NULL;
	killswitch      = CreateEvent( NULL, TRUE, FALSE, NULL );

	InitializeCriticalSection( &downloadsCS );
}


void DownloadManager::Kill()
{
	SetEvent( killswitch );
	if ( download_thread )
	{
		WaitForSingleObject( download_thread, 3000 );
		CloseHandle( download_thread );
	}

	DeleteCriticalSection( &downloadsCS );
	CloseHandle( killswitch );
}


static void SetUserAgent( api_httpreceiver *p_http )
{
	char agent[ 256 ] = { 0 };
	StringCchPrintfA( agent, 256, "User-Agent: %S/%S", WASABI_API_APP->main_getAppName(), WASABI_API_APP->main_getVersionNumString() );
	p_http->addheader( agent );

	//QString l_winamp_user_agent = QString( "User-Agent: %1 Winamp/%2" ).arg( QWebEngineProfile::defaultProfile()->httpUserAgent(), STR_WINAMP_PRODUCTVER ).replace( ",", "." );

	//http->addheader( l_winamp_user_agent.toStdString().c_str() );
}


DownloadToken DownloadManager::Download( const char *url, ifc_downloadManagerCallback *callback )
{
	return DownloadEx( url, callback, api_downloadManager::DOWNLOADEX_TEMPFILE );
}

DownloadToken DownloadManager::DownloadEx( const char *url, ifc_downloadManagerCallback *callback, int flags )
{
	if ( InitDownloadThread() )
	{
		api_httpreceiver *http = NULL;

		ServiceBuild( http, httpreceiverGUID );

		if ( http )
		{
			DownloadData *downloadData = new DownloadData( http, url, flags, callback );

			int  use_proxy = 1;
			bool proxy80   = AGAVE_API_CONFIG->GetBool( internetConfigGroupGUID, L"proxy80", false );

			if ( proxy80 && strstr( url, ":" ) && ( !strstr( url, ":80/" ) && strstr( url, ":80" ) != ( url + strlen( url ) - 3 ) ) )
				use_proxy = 0;

			const wchar_t *proxy = use_proxy ? AGAVE_API_CONFIG->GetString( internetConfigGroupGUID, L"proxy", 0 ) : 0;

			http->open( API_DNS_AUTODNS, DOWNLOAD_BUFFER_SIZE, ( proxy && proxy[ 0 ] ) ? (const char *)AutoChar( proxy ) : NULL );

			SetUserAgent( http );

			if ( callback )
				callback->OnInit( downloadData );

			if ( downloadData->flags & api_downloadManager::DOWNLOADEX_UI )
			{
				for ( ifc_downloadManagerCallback *l_status : status_callbacks )
					l_status->OnInit( downloadData );
			}

			//only call http->connect when it is not pending download request
			if ( !( flags & DOWNLOADEX_PENDING ) )
				http->connect( url, 1 );

			//http->run(); // let's get this party started
			EnterCriticalSection( &downloadsCS );
			downloads.push_back( downloadData );
			LeaveCriticalSection( &downloadsCS );

			return downloadData;
		}
	}

	return 0;
}


void DownloadManager::ResumePendingDownload( DownloadToken p_token )
{
	if ( !p_token )
		return;

	DownloadData *data = (DownloadData *)p_token;
	if ( data->pending )
	{
		data->pending = false;
		data->connectionStart = data->lastDownloadTick = GetTickCount();

		data->http->connect( data->url );
	}
}

void DownloadManager::CancelDownload( DownloadToken p_token )
{
	if ( !p_token )
		return;

	DownloadData *data = (DownloadData *)p_token;
	EnterCriticalSection( &downloadsCS );

	if ( downloads.end() != std::find( downloads.begin(), downloads.end(), data ) )
	{
		ifc_downloadManagerCallback *callback;
		data->Close( &callback );

		//downloads.eraseObject(p_data);
		auto it = std::find( downloads.begin(), downloads.end(), data );
		if ( it != downloads.end() )
		{
			downloads.erase( it );
		}

		LeaveCriticalSection( &downloadsCS );

		if ( callback )
		{
			callback->OnCancel( p_token );
			if ( data->flags & api_downloadManager::DOWNLOADEX_UI )
			{
				for ( ifc_downloadManagerCallback *l_status : status_callbacks )
					l_status->OnCancel( p_token );
			}

			callback->Release();
		}

		data->Release();
	}
	else
		LeaveCriticalSection( &downloadsCS );
}

void DownloadManager::RetainDownload( DownloadToken p_token )
{
	if ( !p_token )
		return;

	DownloadData *data = (DownloadData *)p_token;
	if ( data )
		data->Retain();
}

void DownloadManager::ReleaseDownload( DownloadToken p_token )
{
	if ( !p_token )
		return;

	DownloadData *data = (DownloadData *)p_token;
	if ( data )
		data->Release();
}


void DownloadManager::RegisterStatusCallback( ifc_downloadManagerCallback *callback )
{
	EnterCriticalSection( &downloadsCS );
	status_callbacks.push_back( callback );
	LeaveCriticalSection( &downloadsCS );
}

void DownloadManager::UnregisterStatusCallback( ifc_downloadManagerCallback *callback )
{
	EnterCriticalSection( &downloadsCS );

	auto it = std::find( status_callbacks.begin(), status_callbacks.end(), callback );
	if ( it != status_callbacks.end() )
		status_callbacks.erase( it );

	LeaveCriticalSection( &downloadsCS );
}


bool DownloadManager::IsPending( DownloadToken token )
{
	DownloadData *data = (DownloadData *)token;
	if ( data )
		return data->pending;
	else
		return FALSE;
}


/**********************************************************************************
 *                                    PRIVATE                                     *
 **********************************************************************************/
bool DownloadManager::DownloadThreadTick()
{
	unsigned int i = 0;
	char *downloadBuffer = (char *)malloc( DOWNLOAD_BUFFER_SIZE );

	while ( WaitForSingleObject( killswitch, 0 ) != WAIT_OBJECT_0 )
	{
		EnterCriticalSection( &downloadsCS );
		if ( downloads.empty() )
		{
			// TODO: might be nice to dynamically increase the sleep time if this happens
			// (maybe to INFINITE and have Download() wake us?)
			LeaveCriticalSection( &downloadsCS );

			free( downloadBuffer );

			return true;
		}

		if ( i >= downloads.size() )
		{
			LeaveCriticalSection( &downloadsCS );
			free( downloadBuffer );

			return true;
		}

		DownloadData *thisDownload = downloads[ i ];
		if ( thisDownload->pending )
		{
			LeaveCriticalSection( &downloadsCS );
		}
		else
		{
			thisDownload->Retain();
			LeaveCriticalSection( &downloadsCS );


			INT tick = Tick( thisDownload, downloadBuffer, DOWNLOAD_BUFFER_SIZE );
			switch ( tick )
			{
				case TICK_NODATA:
					// do nothing
					break;

				case TICK_CONNECTING:
					break;

				case TICK_CONNECTED:
					if ( thisDownload->callback )
						thisDownload->callback->OnConnect( thisDownload );
					if ( thisDownload->flags & api_downloadManager::DOWNLOADEX_UI )
					{
						for ( ifc_downloadManagerCallback *l_status : status_callbacks )
							l_status->OnConnect( thisDownload );
					}
					break;

				case TICK_SUCCESS:
					if ( thisDownload->callback )
						thisDownload->callback->OnTick( thisDownload );
					if ( thisDownload->flags & api_downloadManager::DOWNLOADEX_UI )
					{
						for ( ifc_downloadManagerCallback *l_status : status_callbacks )
							l_status->OnTick( thisDownload );
					}

					// TODO: send out update l_callback
					break;

				case TICK_FINISHED:
				case TICK_FAILURE:
				case TICK_TIMEOUT:
				case TICK_CANT_CONNECT:
				case TICK_WRITE_ERROR:
					FinishDownload( thisDownload, tick );
					break;
			}
			thisDownload->Release();
		}

		i++;
	}

	free( downloadBuffer );

	return false; // we only get here when killswitch is set
}

int DownloadManager::DownloadTickThreadPoolFunc( HANDLE handle, void *user_data, intptr_t id )
{
	DownloadManager *dlmgr = (DownloadManager *)user_data;
	if ( dlmgr->DownloadThreadTick() )
	{
		TimerHandle t( handle );
		t.Wait( DOWNLOAD_SLEEP_MS );
	}
	else
	{
		WASABI_API_THREADPOOL->RemoveHandle( 0, handle );
		SetEvent( dlmgr->download_thread );
	}

	return 0;
}

bool DownloadManager::InitDownloadThread()
{
	if ( download_thread == NULL )
	{
		download_thread = CreateEvent( 0, FALSE, FALSE, 0 );
		TimerHandle t;
		WASABI_API_THREADPOOL->AddHandle( 0, t, DownloadTickThreadPoolFunc, this, 0, api_threadpool::FLAG_LONG_EXECUTION );
		t.Wait( DOWNLOAD_SLEEP_MS );
	}

	return ( download_thread != NULL );
}

void DownloadManager::FinishDownload( DownloadData *p_data, int code )
{
	if ( p_data == NULL )
		return;

	ifc_downloadManagerCallback *l_callback = NULL;

	EnterCriticalSection( &downloadsCS );
	p_data->Close( &l_callback );
	LeaveCriticalSection( &downloadsCS );

	if ( l_callback != NULL )
	{
		if ( code == TICK_FINISHED )
		{
			l_callback->OnFinish( p_data );
			if ( p_data->flags & api_downloadManager::DOWNLOADEX_UI )
			{
				for ( ifc_downloadManagerCallback *l_data : status_callbacks )
					l_data->OnFinish( p_data );
			}
		}
		else
		{
			l_callback->OnError( p_data, code );
			if ( p_data->flags & api_downloadManager::DOWNLOADEX_UI )
			{
				for ( ifc_downloadManagerCallback *l_data : status_callbacks )
					l_data->OnError( p_data, code );
			}
		}

		l_callback->Release();
	}

	EnterCriticalSection( &downloadsCS );

	auto it = std::find( downloads.begin(), downloads.end(), p_data );
	if ( it != downloads.end() )
		downloads.erase( it );

	LeaveCriticalSection( &downloadsCS );

	p_data->Release();
}

int DownloadManager::Tick( DownloadData *thisDownload, void *buffer, int bufferSize )
{
	if ( !thisDownload )
		return api_downloadManager::TICK_FAILURE;

	int state = thisDownload->http->run();
	if ( state == HTTPRECEIVER_RUN_ERROR || thisDownload == NULL )
		return api_downloadManager::TICK_FAILURE;

	if ( !thisDownload->fileext )
		thisDownload->getExtention();


	int downloaded = thisDownload->http->get_bytes( buffer, bufferSize );
	if ( downloaded )
	{
		switch ( thisDownload->flags & DOWNLOADEX_MASK_DOWNLOADMETHOD )
		{
			case api_downloadManager::DOWNLOADEX_BUFFER:
			{
				thisDownload->buffer.reserve( thisDownload->http->content_length() );
				thisDownload->buffer.add( buffer, downloaded );
			}
			break;
			case api_downloadManager::DOWNLOADEX_TEMPFILE:
			{
				DWORD written = 0;
				WriteFile( thisDownload->hFile, buffer, downloaded, &written, NULL );
				if ( written != downloaded )
					return api_downloadManager::TICK_WRITE_ERROR;
			}
			break;
			case api_downloadManager::DOWNLOADEX_CALLBACK:
			{
				if ( thisDownload->flags & api_downloadManager::DOWNLOADEX_UI )
				{
					for ( ifc_downloadManagerCallback *l_status : status_callbacks )
						l_status->OnData( thisDownload, buffer, downloaded );
				}

				if ( thisDownload->callback )
					thisDownload->callback->OnData( thisDownload, buffer, downloaded );
			}
		}

		thisDownload->lastDownloadTick = GetTickCount();
		thisDownload->bytesDownloaded += downloaded;

		return api_downloadManager::TICK_SUCCESS;
	}
	else // nothing in the buffer
	{
		if ( state == HTTPRECEIVER_RUN_CONNECTION_CLOSED ) // see if the connection is closed
		{
			return api_downloadManager::TICK_FINISHED; // yay we're done
		}

		if ( GetTickCount() - thisDownload->lastDownloadTick > DOWNLOAD_TIMEOUT_MS ) // check for timeout
			return api_downloadManager::TICK_TIMEOUT;

		switch ( thisDownload->http->get_status() )
		{
			case HTTPRECEIVER_STATUS_CONNECTING:
				if ( thisDownload->last_status != HTTPRECEIVER_STATUS_CONNECTING )
				{
					thisDownload->last_status = HTTPRECEIVER_STATUS_CONNECTING;
					return api_downloadManager::TICK_CONNECTING;
				}
				else
				{
					return api_downloadManager::TICK_NODATA;
				}
			case HTTPRECEIVER_STATUS_READING_HEADERS:
				if ( thisDownload->last_status != HTTPRECEIVER_STATUS_READING_HEADERS )
				{
					thisDownload->last_status = HTTPRECEIVER_STATUS_READING_HEADERS;
					return api_downloadManager::TICK_CONNECTED;
				}
				else
				{
					return api_downloadManager::TICK_NODATA;
				}
		}

		if ( !thisDownload->replyCode )
			thisDownload->replyCode = thisDownload->http->getreplycode();

		switch ( thisDownload->replyCode )
		{
			case 0:
			case 100:
			case 200:
			case 201:
			case 202:
			case 203:
			case 204:
			case 205:
			case 206:
				return api_downloadManager::TICK_NODATA;
			default:
				return api_downloadManager::TICK_CANT_CONNECT;
		}
	}
}





#define CBCLASS DownloadManager
START_DISPATCH;
CB(  API_DOWNLOADMANAGER_DOWNLOAD,                 Download )
CB(  API_DOWNLOADMANAGER_DOWNLOADEX,               DownloadEx )
CB(  API_DOWNLOADMANAGER_GETRECEIVER,              GetReceiver )
CB(  API_DOWNLOADMANAGER_GETLOCATION,              GetLocation )
VCB( API_DOWNLOADMANAGER_SETLOCATION,              SetLocation )
CB(  API_DOWNLOADMANAGER_GETEXTENTION,             GetExtention )
CB(  API_DOWNLOADMANAGER_GETURL,                   GetUrl )
CB(  API_DOWNLOADMANAGER_GETBYTESDOWNLOADED,       GetBytesDownloaded );
CB(  API_DOWNLOADMANAGER_GETBUFFER,                GetBuffer );
VCB( API_DOWNLOADMANAGER_RESUMEPENDINGDOWNLOAD,    ResumePendingDownload );
VCB( API_DOWNLOADMANAGER_CANCELDOWNLOAD,           CancelDownload );
VCB( API_DOWNLOADMANAGER_RETAINDOWNLOAD,           RetainDownload );
VCB( API_DOWNLOADMANAGER_RELEASEDOWNLOAD,          ReleaseDownload );
VCB( API_DOWNLOADMANAGER_REGISTERSTATUSCALLBACK,   RegisterStatusCallback );
VCB( API_DOWNLOADMANAGER_UNREGISTERSTATUSCALLBACK, UnregisterStatusCallback );
CB(  API_DOWNLOADMANAGER_GETSOURCE,                GetSource );
CB(  API_DOWNLOADMANAGER_GETTITLE,                 GetTitle );
CB(  API_DOWNLOADMANAGER_ISPENDING,                IsPending );
END_DISPATCH;
#undef CBCLASS
