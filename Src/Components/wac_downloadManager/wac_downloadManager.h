#ifndef NULLSOFT_WAC_DOWNLOAD_MANAGER_DOWNLOAD_MANAGER_H
#define NULLSOFT_WAC_DOWNLOAD_MANAGER_DOWNLOAD_MANAGER_H

#include <vector>
#include <map>
#include <string> // for string class
#include <atomic>

#include <QtCore>
#include <QAuthenticator>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QUrl>
#include <QUrlQuery>

#include "../nu/GrowBuf.h"

//#include "main.h"

//#include "wac_downloadManager_Headers.h"

#include "wac_download_http_receiver.h"
//#include "wac_download_http_receiver_api.h"

#include "wac_downloadManager_api.h"

//#include "../wac_network/wac_network_http_receiver_api.h"

#define MAX_SIMULTANEOUS_DOWNLOADS    2

static std::map<std::string, std::string> _CONTENT_TYPES_EXTENSIONS = {
	{ "audio/aac",               "aac" },
	{ "audio/mp4",               "mp4" },
	{ "audio/MPA",               "MPA" },
	{ "audio/mpeg",              "mp3" },
	{ "audio/opus",              "opus" },
	{ "audio/ogg",               "ogg" },
	{ "audio/vorbis",            "vorbis" },
	{ "audio/wav",               "wav" },
	{ "video/mp2t",              "ts" },
	{ "video/mp4",               "mp4" },
	{ "video/mpeg",              "mpeg" },
	{ "video/ogg",               "ogv" },
	{ "video/x-matroska",        "mkv" },
	{ "video/x-msvideo",         "avi" }
};



namespace wa
{
	namespace Components
	{
		class WAC_DownloadData : public QObject
		{
			Q_OBJECT

		public:
			WAC_DownloadData( api_wac_download_manager_http_receiver *p_http, const char *p_url, int p_flags, ifc_downloadManagerCallback *p_callback );
			~WAC_DownloadData();

			void Retain();
			void Release();

			void Close( ifc_downloadManagerCallback **callbackCopy );

			bool getExtention();

			api_wac_download_manager_http_receiver *_http     = Q_NULLPTR;
			ifc_downloadManagerCallback            *_callback = Q_NULLPTR;

			int                          _flags = 0;

			HANDLE                       _hFile;                // handle to the open file where data is written to

			DWORD                        _lastDownloadTick;     // last time we got data out of the connection. used for timeout
			DWORD                        _connectionStart;      // used for when the connection starts, to help us calculate a k/s download rate

			wchar_t                      _filepath[ MAX_PATH ]; // where file is downloaded to. probably a temp path
			char *_fileext = 0;
			char                         _url[ 1024 ];
			wchar_t                      _source[ 1024 ];
			wchar_t                      _title[ 1024 ];

			int                          _replyCode = 0;        // HTTP 200, 404, etc.
			uint64_t                     _bytesDownloaded = 0;
			int                          _last_status;          // from http->get_status()
			bool                         _pending;

			GrowBuf                      _buffer;


		private:
			std::atomic<std::size_t>     _refCount = 1;


		};

		class WAC_DownloadManager : public QNetworkAccessManager, public api_downloadManager
		{
			Q_OBJECT

		public:
			WAC_DownloadManager( QObject *parent = Q_NULLPTR );
			~WAC_DownloadManager();

			static const char *getServiceName()                                { return "Download Manager Service"; }
			static GUID        getServiceGuid()                                { return DownloadManagerGUID; }

			DownloadToken      Download( const char *p_url, ifc_downloadManagerCallback *p_callback );
			DownloadToken      DownloadEx( const char *p_url, ifc_downloadManagerCallback *p_callback, int p_flags );


		private:
			void init();

			QNetworkReply *createRequest( Operation p_operation, const QNetworkRequest &p_request, QIODevice *p_outgoing_data = Q_NULLPTR );

			QMetaObject::Connection                                      _connection_authentication_required;

			std::map<std::string, wa::Components::WAC_Download_HTTP_Receiver *> _runing_downloads;
			std::map<std::string, wa::Components::WAC_Download_HTTP_Receiver *> _pending_downloads;


		private Q_SLOTS:
			void on_s_authentication_required( QNetworkReply *p_reply, QAuthenticator *p_authenticator );
		};
	}
}






#pragma region "DownloadData"
struct DownloadData
{
	DownloadData( api_httpreceiver *http, const char *url, int flags, ifc_downloadManagerCallback *callback );
	~DownloadData();

	void Retain();
	void Release();

	void Close( ifc_downloadManagerCallback **callbackCopy );

	bool getExtention();

	api_httpreceiver            *http     = NULL;
	ifc_downloadManagerCallback *callback = NULL;

	wchar_t           filepath[ MAX_PATH ]; // where file is downloaded to. probably a temp path
	char             *fileext         = 0;
	char              url[ 1024 ];
	wchar_t           source[ 1024 ];
	wchar_t           title[ 1024 ];

	HANDLE            hFile;                // handle to the open file where data is written to
	DWORD             lastDownloadTick;     // last time we got data out of the connection. used for timeout
	DWORD             connectionStart;      // used for when the connection starts, to help us calculate a k/s download rate

	int               replyCode       = 0;  // HTTP 200, 404, etc.
	uint64_t          bytesDownloaded = 0;
	int               last_status;          // from http->get_status()
	bool              pending;

	int               flags;
	GrowBuf           buffer;

private:
	std::atomic<std::size_t> _refCount = 1;
};
#pragma endregion


/* method ideas

Download(url, owner_token, callback, user_data)
Lock()  - call before enumerating and doing anything
Unlock()
CancelDownload()
Enum(owner_token)

*/

class DownloadManager : public api_downloadManager
{
public:
	static const char *getServiceName()                               { return "Download Manager"; }
	static GUID        getServiceGuid()                               { return DownloadManagerGUID; }

	DownloadManager();

	void           Kill();

	DownloadToken  Download( const char *url, ifc_downloadManagerCallback *callback );
	DownloadToken  DownloadEx( const char *url, ifc_downloadManagerCallback *callback, int flags );

	void           ResumePendingDownload( DownloadToken token );
	void           CancelDownload( DownloadToken token );
	void           RetainDownload( DownloadToken token );
	void           ReleaseDownload( DownloadToken token );

	void           RegisterStatusCallback( ifc_downloadManagerCallback *callback );
	void           UnregisterStatusCallback( ifc_downloadManagerCallback *callback );

	bool           IsPending( DownloadToken token );

	/*
		only call these during a callback!
	*/
	api_httpreceiver *GetReceiver( DownloadToken token )
	{
		if ( token )
			return ( (DownloadData *)token )->http;
		else
			return 0;
	}

	const wchar_t    *GetLocation( DownloadToken token )
	{
		if ( token )
			return ( (DownloadData *)token )->filepath;
		else
			return 0;
	}

	const wchar_t    *GetSource( DownloadToken p_token )
	{
		if ( !p_token )
			return 0;

		DownloadData *data = (DownloadData *)p_token;
		if ( data )
			return data->source;
		else
			return 0;
	}

	const wchar_t    *GetTitle( DownloadToken p_token )
	{
		if ( !p_token )
			return 0;

		DownloadData *data = (DownloadData *)p_token;
		if ( data )
			return data->title;
		else
			return 0;
	}

	void              SetLocation( DownloadToken token, const wchar_t *p_location )
	{
		if ( token )
			wcscpy( ( (DownloadData *)token )->filepath, p_location );
	}

	const char       *GetExtention( DownloadToken token )
	{
		if ( token )
			return ( (DownloadData *)token )->fileext;
		else
			return 0;
	}

	const char       *GetUrl( DownloadToken token )
	{
		if ( token )
			return ( (DownloadData *)token )->url;
		else
			return 0;
	}

	int               GetBuffer( DownloadToken token, void **buffer, size_t *bufferlen )
	{
		if ( token )
		{
			*buffer    = ( (DownloadData *)token )->buffer.get();
			*bufferlen = ( (DownloadData *)token )->buffer.getlen();

			return 0;
		}
		else
			return 1;
	}

	uint64_t          GetBytesDownloaded( DownloadToken token )
	{
		if ( token )
			return ( (DownloadData *)token )->bytesDownloaded;
		else
			return 0;
	}

private:
	CRITICAL_SECTION downloadsCS;

	bool       DownloadThreadTick();
	static int DownloadTickThreadPoolFunc( HANDLE handle, void *user_data, intptr_t id );

	bool       InitDownloadThread();
	void       FinishDownload( DownloadData *data, int code );
	int        Tick( DownloadData *thisDownload, void *buffer, int bufferSize );

	HANDLE download_thread;
	HANDLE killswitch;

	std::vector<DownloadData *> downloads;
	typedef std::vector<ifc_downloadManagerCallback *> StatusList;
	StatusList status_callbacks;

protected:
	RECVS_DISPATCH;
};

#endif // !NULLSOFT_WAC_DOWNLOAD_MANAGER_DOWNLOAD_MANAGER_H
