#pragma once

#include "../Components/wac_downloadManager/DownloadCallbackT.h"

class SongDownloader : public DownloadCallbackT<SongDownloader>
{
public:
	SongDownloader(const wchar_t *filename, HANDLE done_event, void (*callback)(void *callbackContext, wchar_t *status), void *context);
	~SongDownloader();

	void OnInit(DownloadToken token);
	void OnData(DownloadToken token, void *data, size_t datalen);
	void OnCancel(DownloadToken token);
	void OnError(DownloadToken token, int error);
	void OnFinish(DownloadToken token);	

private:
	void (*callback)(void *callbackContext, wchar_t *status);
	void *context;
	HANDLE hFile, done_event;
	uint64_t content_length;
	uint64_t bytes_downloaded;

};
