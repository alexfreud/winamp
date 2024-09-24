#ifndef NULLSOFT_WAC_DOWNLOAD_MANAGER_BACKGROUND_DOWNLOADER_H
#define NULLSOFT_WAC_DOWNLOAD_MANAGER_BACKGROUND_DOWNLOADER_H

#include "bfc/platform/types.h"

class Downloader
{
public:
	typedef int ( *DownloadCallback )( void *userdata, void *buffer, size_t bufferSize );
	bool Download( const char *url, DownloadCallback &callback, void *userdata, uint64_t startPosition = 0 );

};

#endif // !NULLSOFT_WAC_DOWNLOAD_MANAGER_BACKGROUND_DOWNLOADER_H
