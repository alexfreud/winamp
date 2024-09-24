#pragma once

#include "../nu/refcount.h"
#include "../Components/wac_downloadManager/DownloadCallbackT.h"

class WifiDevice;

class PairDownloader : public DownloadCallbackT<PairDownloader>
{
public:
	PairDownloader(WifiDevice *device);
	~PairDownloader();

	void OnInit(DownloadToken token);
	void OnData(DownloadToken token, void *data, size_t datalen);
	void OnCancel(DownloadToken token);
	void OnError(DownloadToken token, int error);
	void OnFinish(DownloadToken token);

private:
	WifiDevice *device;
};

bool IsPaired(uint64_t id);
void SetPaired(uint64_t id, bool status);