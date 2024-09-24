#include "main.h"
#include "api__ml_wire.h"
#include "DownloadStatus.h"
#include "DownloadsDialog.h"
#include "./navigation.h"

#include <strsafe.h>

DownloadStatus downloadStatus;

using namespace Nullsoft::Utility;

DownloadStatus::Status::Status() 
{
	Init();
}

DownloadStatus::Status::Status( size_t _downloaded, size_t _maxSize, const wchar_t *_channel, const  wchar_t *_item, const wchar_t *_path )
{
	Init();

	downloaded = _downloaded;
	maxSize    = _maxSize;
	channel    = _wcsdup( _channel );
	item       = _wcsdup( _item );
	path       = _wcsdup( _path );
}

const DownloadStatus::Status &DownloadStatus::Status::operator =( const DownloadStatus::Status &copy )
{
	Reset();
	Init();

	downloaded = copy.downloaded;
	maxSize    = copy.maxSize;
	channel    = _wcsdup( copy.channel );
	item       = _wcsdup( copy.item );
	path       = _wcsdup( copy.path );
	killswitch = copy.killswitch;

	return *this;
}

DownloadStatus::Status::~Status()
{
	Reset();
}

void DownloadStatus::Status::Init()
{
	downloaded = 0;
	maxSize    = 0;
	killswitch = 0;
	channel    = 0;
	item       = 0;
	path       = 0;
}

void DownloadStatus::Status::Reset()
{
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

	if ( path )
	{
		free( path );
		path = 0;
	}
}

void DownloadStatus::AddDownloadThread(DownloadToken token, const wchar_t *channel, const wchar_t *item, const wchar_t *path)
{
	{
		AutoLock lock(statusLock);
		downloads[token] = Status(0,0,channel,item,path);
		DownloadsUpdated(downloads[token],token);
	}

	Navigation_ShowService(SERVICE_DOWNLOADS, SHOWMODE_AUTO);
}

void DownloadStatus::DownloadThreadDone(DownloadToken token)
{
	{
		AutoLock lock(statusLock);
		downloads.erase(token);
	}

	Navigation_ShowService(SERVICE_DOWNLOADS, SHOWMODE_AUTO);
}

bool DownloadStatus::UpdateStatus(DownloadToken token, size_t downloaded, size_t maxSize)
{
	AutoLock lock(statusLock);
	downloads[token].downloaded = downloaded;
	downloads[token].maxSize    = maxSize;

	return !!downloads[token].killswitch;
}

bool DownloadStatus::CurrentlyDownloading()
{
	AutoLock lock(statusLock);
	return !downloads.empty();
}

void DownloadStatus::GetStatusString( wchar_t *status, size_t len )
{
	AutoLock lock( statusLock );
	Downloads::iterator itr;
	size_t bytesDownloaded = 0, bytesTotal = 0, numDownloads = 0;
	bool unknownTotal = false;
	for ( itr = downloads.begin(); itr != downloads.end(); itr++ )
	{
		Status &dlstatus = itr->second;
		if ( dlstatus.maxSize )
		{
			numDownloads++;
			bytesDownloaded += dlstatus.downloaded;
			bytesTotal      += dlstatus.maxSize;
		}
		else // don't have a max size
		{
			if ( dlstatus.downloaded ) // if we've downloaded some then we just don't know the total
			{
				unknownTotal = true;
				numDownloads++;
				bytesDownloaded += dlstatus.downloaded;
			}
		}
	}

	if ( 0 == numDownloads )
	{
		status[ 0 ] = L'\0';
	}
	else
	{
		if ( unknownTotal || bytesTotal == 0 )
			StringCchPrintf( status, len, WASABI_API_LNGSTRINGW( IDS_DOWNLOADING_KB_COMPLETE ), numDownloads, bytesDownloaded / 1024);
		else
			StringCchPrintf( status, len, WASABI_API_LNGSTRINGW( IDS_DOWNLOADING_KB_PROGRESS ), numDownloads, bytesDownloaded / 1024, bytesTotal / 1024, MulDiv( (int)bytesDownloaded, 100, (int)bytesTotal ) );
	}
}
