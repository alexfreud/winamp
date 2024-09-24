#pragma once
#include <bfc/platform/types.h>

class api_httpreceiver;

class Downloader
{
public:
	class DownloadCallback
	{
	public:
		virtual int OnConnect(api_httpreceiver *http)=0;
		virtual int OnData(void *buffer, size_t bufferSize)=0;
	};


	bool Download(const char *url, DownloadCallback *callback, uint64_t startPosition = 0);

};