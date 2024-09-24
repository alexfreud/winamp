#include "main.h"
#include "Downloaded.h"

DownloadList downloadedFiles;
using namespace Nullsoft::Utility;
Nullsoft::Utility::LockGuard downloadedLock;

DownloadedFile::DownloadedFile()
{
	Init();
}

DownloadedFile::DownloadedFile(const wchar_t *_url, const wchar_t *_path, const wchar_t *_channel, const wchar_t *_item, __time64_t publishDate)
{
	Init();

	this->publishDate = publishDate;

	SetChannel( _channel );
	SetItem( _item );
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
	channel         = 0;
	item            = 0;
	bytesDownloaded = 0;
	totalSize       = 0;
	publishDate     = 0;
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

	if ( channel )
	{
		free( channel );
		channel = 0;
	}

	if ( item )
	{
		free( item );
		item = 0;
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

void DownloadedFile::SetItem( const wchar_t *_item )
{
	free( item );
	item = _wcsdup( _item );
}

void DownloadedFile::SetChannel( const wchar_t *_channel )
{
	free( channel );
	channel = _wcsdup( _channel );
}

const DownloadedFile &DownloadedFile::operator =( const DownloadedFile &copy )
{
	Reset();
	Init();

	SetChannel( copy.channel );
	SetItem( copy.item );

	bytesDownloaded = copy.bytesDownloaded;
	totalSize       = copy.totalSize;
	publishDate     = copy.publishDate;
	downloadDate    = copy.downloadDate;

	SetPath( copy.path );
	SetURL( copy.url );

	return *this;
}