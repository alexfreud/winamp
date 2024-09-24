#include "Main.h"
#include "DownloadsParse.h"
#include "Downloaded.h"
#include "Defaults.h"
#include "ParseUtil.h"
#include <wchar.h>
#include <locale.h>

static __time64_t filetime( const wchar_t *file )
{
	if ( !file || !*file )
		return 0;

	WIN32_FIND_DATAW f = { 0 };

	HANDLE h = FindFirstFileW( file, &f );
	if ( h == INVALID_HANDLE_VALUE )
	{
		/*wchar_t tmp[128] = {0};
		wsprintf(tmp,L"%d",GetLastError());
		MessageBox(0,file,tmp,0);*/
		return 0;
	}

	FindClose( h );
	SYSTEMTIME s = { 0 };
	FileTimeToSystemTime( &f.ftCreationTime, &s );

	tm t = { 0 };
	t.tm_year = s.wYear - 1900;
	t.tm_mon  = s.wMonth - 1;
	t.tm_mday = s.wDay;
	t.tm_hour = s.wHour;
	t.tm_min  = s.wMinute;
	t.tm_sec  = s.wMinute;
	__time64_t ret = _mktime64(&t);
	/*wchar_t tmp[128] = {0};
	wsprintf(tmp,L"%d",ret);
	MessageBox(0,tmp,0,0);*/

	return ret;
}

size_t GetFileSize(LPCWSTR path)
{
	WIN32_FIND_DATAW data = {0};
	if (path && *path)
	{
		HANDLE h = FindFirstFileW(path, &data);
		if (h == INVALID_HANDLE_VALUE)
			return -1;
		FindClose(h);
	}
    return data.nFileSizeLow/* | (__int64)data.nFileSizeHigh << 32*/;
}

static void ReadDownload( const XMLNode *item )
{
	DownloadedFile newDownloaded;

	const wchar_t *source = GetContent( item, L"source" );
	if ( !source ) source = GetContent( item, L"channel" );
	newDownloaded.SetSource( source );

	const wchar_t *title = GetContent( item, L"title" );
	if ( !title ) title = GetContent( item, L"item" );;
	newDownloaded.SetTitle( title );

	const wchar_t *url = GetContent( item, L"url" );
	newDownloaded.SetURL( url );

	const wchar_t *path = GetContent( item, L"path" );
	newDownloaded.SetPath( path );

	// TODO ideally should be able to cope with __int64
	//		but the db is setup for int currently...
	const wchar_t *size = GetContent( item, L"size" );
	if ( size && size[ 0 ] )
	{
		size_t val = _wtoi( size );
		if ( val > 0 ) newDownloaded.totalSize = val;
		else if ( !val )
		{
			val = GetFileSize( path );
			if ( val > 0 ) newDownloaded.totalSize = val;
			else newDownloaded.totalSize = -1;
			dirty++;
		}
	}
	else
	{
		size_t val = GetFileSize( path );
		if ( val > 0 ) newDownloaded.totalSize = val;
		else newDownloaded.totalSize = -1;
		dirty++;
	}

	const wchar_t *downloadDate = GetContent( item, L"downloadDate" );
	if ( downloadDate && downloadDate[ 0 ] )
	{
		__time64_t val = (__time64_t)_wtoi( downloadDate );
		if ( time > 0 ) newDownloaded.downloadDate = val;
	}
	else
	{
		__time64_t val = filetime( newDownloaded.path );
		if ( time > 0 ) newDownloaded.downloadDate = val;
	}

	const wchar_t *status = GetContent( item, L"downloadStatus" );
	if ( status && status[ 0 ] )
	{
		newDownloaded.downloadStatus = _wtoi( status );
	}
	else
	{
		newDownloaded.downloadStatus = DownloadedFile::DOWNLOAD_SUCCESS;
	}

	downloadedFiles.downloadList.push_back( newDownloaded );
}

void DownloadsParse::ReadNodes( const wchar_t *url )
{
	XMLNode::NodeList::const_iterator itr;
	const XMLNode *curNode = xmlDOM.GetRoot();

	if ( curNode->Present( L"winamp:preferences" ) )
		curNode = curNode->Get( L"winamp:preferences" );

	curNode = curNode->Get( L"downloads" );
	if ( curNode )
	{
		const wchar_t *prop = curNode->GetProperty( L"downloadsTitleWidth" );
		if ( prop && prop[ 0 ] )
			downloadsTitleWidth = _wtoi( prop );
		if ( downloadsTitleWidth <= 0 )
			downloadsTitleWidth = DOWNLOADSTITLEWIDTHDEFAULT;

		prop = curNode->GetProperty( L"downloadsProgressWidth" );
		if ( prop && prop[ 0 ] )
			downloadsProgressWidth = _wtoi( prop );
		if ( downloadsProgressWidth <= 0 )
			downloadsProgressWidth = DOWNLOADSPROGRESSWIDTHDEFAULT;

		prop = curNode->GetProperty( L"downloadsDateWidth" );
		if ( prop && prop[ 0 ] )
			downloadsDateWidth = _wtoi( prop );
		if ( downloadsDateWidth <= 0 )
			downloadsDateWidth = DOWNLOADSDATEWIDTHDEFAULTS;

		prop = curNode->GetProperty( L"downloadsSourceWidth" );
		if ( prop && prop[ 0 ] )
			downloadsSourceWidth = _wtoi( prop );
		if ( downloadsSourceWidth <= 0 )
			downloadsSourceWidth = DOWNLOADSSOURCEWIDTHDEFAULT;

		prop = curNode->GetProperty( L"downloadsPathWidth" );
		if ( prop && prop[ 0 ] )
			downloadsPathWidth = _wtoi( prop );
		if ( downloadsPathWidth <= 0 )
			downloadsPathWidth = DOWNLOADSPATHWIDTHDEFAULTS;

		prop = curNode->GetProperty( L"downloadsItemSort" );
		if ( prop && prop[ 0 ] )
			downloadsItemSort = _wtoi( prop );

		prop = curNode->GetProperty( L"downloadsSortAscending" );
		if ( prop && prop[ 0 ] )
			downloadsSortAscending = !PropertyIsFalse( curNode, L"downloadsSortAscending" );

		const XMLNode::NodeList *downloadsList = curNode->GetList( L"download" );
		if ( downloadsList )
		{
			for ( itr = downloadsList->begin(); itr != downloadsList->end(); itr++ )
				ReadDownload( *itr );
		}
	}
}
