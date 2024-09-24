#ifndef NULLSOFT_BACKGROUNDDOWNLOADERH
#define NULLSOFT_BACKGROUNDDOWNLOADERH

#include <windows.h>

class BackgroundDownloader
{
public:
	//void SetSpeed(int kilobytesPerSecond);

	void Download(const wchar_t *url, const wchar_t *savePath,
		const wchar_t *channel, const wchar_t *item, __time64_t publishDate);

	//void Shutdown();
};

extern BackgroundDownloader downloader;
#endif