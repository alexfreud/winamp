#include "Main.h"

//#include "../Components/wac_downloads/wac_downloads_download_manager.h"

using namespace Nullsoft::Utility;

DownloadViewCallback::DownloadViewCallback()
{}

void DownloadViewCallback::OnInit( DownloadToken token )
{
	// ---- Inform the download status service of our presence----
	downloadStatus.AddDownloadThread( token, WAC_API_DOWNLOADMANAGER->GetSource( token ), WAC_API_DOWNLOADMANAGER->GetTitle( token ), WAC_API_DOWNLOADMANAGER->GetLocation( token ) );
}

void DownloadViewCallback::OnConnect( DownloadToken token )
{}

void DownloadViewCallback::OnData( DownloadToken token, void *data, size_t datalen )
{
	if ( token == NULL )
		return;

	api_httpreceiver *http       = WAC_API_DOWNLOADMANAGER->GetReceiver( token );
	size_t            totalSize  = http->content_length();
	size_t            downloaded = (size_t)WAC_API_DOWNLOADMANAGER->GetBytesDownloaded( token );

	// if killswitch is turned on, then cancel download
	downloadStatus.UpdateStatus( token, downloaded, totalSize );
	dirty++;
}

void DownloadViewCallback::OnCancel( DownloadToken token )
{
	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );

	DownloadedFile   *data = new DownloadedFile( AutoWide( http->get_url() ), WAC_API_DOWNLOADMANAGER->GetLocation( token ), WAC_API_DOWNLOADMANAGER->GetSource( token ), WAC_API_DOWNLOADMANAGER->GetTitle( token ), DownloadedFile::DOWNLOAD_CANCELED, _time64( 0 ) );

	data->totalSize       = http->content_length();
	data->bytesDownloaded = (size_t)WAC_API_DOWNLOADMANAGER->GetBytesDownloaded( token );

	{
		AutoLock lock( downloadedFiles );
		downloadedFiles.downloadList.push_back( *data );
		DownloadsUpdated( token, &downloadedFiles.downloadList[ downloadedFiles.downloadList.size() - 1 ] );
	}

	downloadStatus.DownloadThreadDone( token );
	delete data;
	dirty++;
}

void DownloadViewCallback::OnError( DownloadToken token, int error )
{
	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );

	DownloadedFile   *data = new DownloadedFile( AutoWide( http->get_url() ), WAC_API_DOWNLOADMANAGER->GetLocation( token ), WAC_API_DOWNLOADMANAGER->GetSource( token ), WAC_API_DOWNLOADMANAGER->GetTitle( token ), DownloadedFile::DOWNLOAD_FAILURE, _time64( 0 ) );

	data->totalSize       = http->content_length();
	data->bytesDownloaded = (size_t)WAC_API_DOWNLOADMANAGER->GetBytesDownloaded( token );

	{
		AutoLock lock( downloadedFiles );
		downloadedFiles.downloadList.push_back( *data );
		DownloadsUpdated( token, &downloadedFiles.downloadList[ downloadedFiles.downloadList.size() - 1 ] );
	}

	downloadStatus.DownloadThreadDone( token );
	delete data;
	dirty++;
}

void DownloadViewCallback::OnFinish( DownloadToken token )
{
	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );

	DownloadedFile   *data = new DownloadedFile( AutoWide( http->get_url() ), WAC_API_DOWNLOADMANAGER->GetLocation( token ), WAC_API_DOWNLOADMANAGER->GetSource( token ), WAC_API_DOWNLOADMANAGER->GetTitle( token ), DownloadedFile::DOWNLOAD_SUCCESS, _time64( 0 ) );

	data->totalSize       = http->content_length();
	data->bytesDownloaded = (size_t)WAC_API_DOWNLOADMANAGER->GetBytesDownloaded( token );

	{
		AutoLock lock( downloadedFiles );
		downloadedFiles.downloadList.push_back( *data );

		//AddDownloadData(*data);

		DownloadsUpdated( token, &downloadedFiles.downloadList[ downloadedFiles.downloadList.size() - 1 ] );
	}

	downloadStatus.DownloadThreadDone( token );
	delete data;
	dirty++;
}


size_t DownloadViewCallback::AddRef()
{
	return this->_ref_count.fetch_add( 1 );
}

size_t DownloadViewCallback::Release()
{
	std::size_t l_ref_countr = this->_ref_count.fetch_sub( 1 );
	if ( l_ref_countr == 0 )
		delete( this );

	return l_ref_countr;
}

DownloadViewCallback::~DownloadViewCallback()
{}

#define CBCLASS DownloadViewCallback
START_DISPATCH;
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONFINISH,  OnFinish )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL,  OnCancel )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONERROR,   OnError )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONDATA,    OnData )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT, OnConnect )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONINIT,    OnInit )
CB( ADDREF,  AddRef )
CB( RELEASE, Release )
END_DISPATCH;
#undef CBCLASS