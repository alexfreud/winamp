#include "main.h"
#include "Downloaded.h"

DownloadList downloadedFiles;
using namespace Nullsoft::Utility;
Nullsoft::Utility::LockGuard downloadedLock;

DownloadedFile::DownloadedFile()
{
	Init();
}

DownloadedFile::DownloadedFile( const wchar_t *_url, const wchar_t *_path, const wchar_t *_source, const wchar_t *_title, int downloadStatus, __time64_t downloadDate )
{
	Init();

	this->downloadStatus = downloadStatus;
	this->downloadDate   = downloadDate;

	SetSource( _source );
	SetTitle( _title );
	SetPath( _path );
	SetURL( _url );
}

DownloadedFile::DownloadedFile( const DownloadedFile &copy )
{
	Init();

	operator =( copy );
}

DownloadedFile::~DownloadedFile()
{
	Reset();
}


void DownloadedFile::Init()
{
	url             = 0;
	path            = 0;
	source          = 0;
	title           = 0;
	bytesDownloaded = 0;
	totalSize       = 0;
	downloadDate    = 0;
}

void DownloadedFile::Reset()
{
	if ( url )
	{
		free( url );
		url = 0;
	}

	if ( path )
	{
		free( path );
		path = 0;
	}

	if ( source )
	{
		free( source );
		source = 0;
	}

	if ( title )
	{
		free( title );
		title = 0;
	}
}

void DownloadedFile::SetPath( const wchar_t *_path )
{
	if ( path )
		free( path );

	path = _wcsdup( _path );
}

void DownloadedFile::SetURL( const wchar_t *_url )
{
	if ( url )
		free( url );

	url = _wcsdup( _url );
}

void DownloadedFile::SetTitle( const wchar_t *_title )
{
	if ( title )
		free( title );

	title = _wcsdup( _title );
}

void DownloadedFile::SetSource( const wchar_t *_source )
{
	if ( source )
		free( source );

	source = _wcsdup( _source );
}

const DownloadedFile &DownloadedFile::operator =( const DownloadedFile &copy )
{
	Reset();
	Init();

	SetSource( copy.source );
	SetTitle( copy.title );

	bytesDownloaded = copy.bytesDownloaded;
	totalSize       = copy.totalSize;
	downloadStatus  = copy.downloadStatus;
	downloadDate    = copy.downloadDate;

	SetPath( copy.path );
	SetURL( copy.url );

	return *this;
}

wchar_t *GetDownloadStatus( int downloadStatus )
{
	switch ( downloadStatus )
	{
		case DownloadedFile::DOWNLOAD_SUCCESS:
			return WASABI_API_LNGSTRINGW( IDS_DOWNLOAD_SUCCESS );
		case DownloadedFile::DOWNLOAD_FAILURE:
			return WASABI_API_LNGSTRINGW( IDS_DOWNLOAD_FAILURE );
		case DownloadedFile::DOWNLOAD_CANCELED:
			return WASABI_API_LNGSTRINGW( IDS_DOWNLOAD_CANCELED );
		default:
			return WASABI_API_LNGSTRINGW( IDS_DOWNLOAD_FAILURE );
	}
}