#ifndef NULLSOFT_WINAMP_CACHE_DOWNLOADER_HEADER
#define NULLSOFT_WINAMP_CACHE_DOWNLOADER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#include "../Components/wac_downloadManager/wac_downloadManager_api.h"

class CacheCallback;
class CacheRecord;


class CacheDownloader : public ifc_downloadManagerCallback
{
public:
	typedef enum
	{
		stateReady        = 0,
		stateInitializing = 1,
		stateConnecting   = 2,
		stateReceiving    = 3,
		stateCompleted    = 4,
		stateAborting     = 5,
	} States;

protected:
	CacheDownloader(CacheRecord *record, BOOL fEnableCompression);
	~CacheDownloader();

public:
	static HRESULT CreateInstance(CacheRecord *record, LPCWSTR pszAddress, BOOL fEnableCompression, CacheDownloader **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	HRESULT Abort();
	UINT GetState();

	HRESULT SetOwner(CacheRecord *record);

protected:
	HRESULT Start(LPCWSTR pszAddress);
	HRESULT DownloadCompleted(INT errorCode);

	/* ifc_downloadManagerCallback */
	void OnFinish(DownloadToken token);
	void OnTick(DownloadToken token);
	void OnError(DownloadToken token, int errorCode);
	void OnCancel(DownloadToken token);
	void OnConnect(DownloadToken token);
	void OnInit(DownloadToken token);

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
	api_downloadManager *manager;
	DownloadToken cookie;
	CacheRecord *owner; // owner not ref counted
	UINT state;
	BOOL enableCompression;
	CRITICAL_SECTION lock;
};


#endif //NULLSOFT_WINAMP_CACHE_DOWNLOADER_HEADER