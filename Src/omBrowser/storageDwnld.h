#ifndef NULLSOFT_WINAMP_OMSTORAGE_DOWNLOADER_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_DOWNLOADER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omstorageasync.h"
#include "../Components/wac_downloadManager/wac_downloadManager_api.h"
#include <bfc/multipatch.h>

class ifc_omservicehost;

#define MPIID_OMSTORAGEASYNC		10
#define MPIID_DOWNLOADCALLBACK		20

class OmStorageDwnld : public MultiPatch<MPIID_OMSTORAGEASYNC, ifc_omstorageasync>,
					   public MultiPatch<MPIID_DOWNLOADCALLBACK, ifc_downloadManagerCallback>
{
public:
	typedef enum
	{		
		flagEnableCompression      = 0x00000001,
		flagUserAbort              = 0x00010000,
		flagCompletedSynchronously = 0x00020000,
	} Flags;

protected:
	OmStorageDwnld(api_downloadManager *downloadManager, BOOL enableCompression);
	~OmStorageDwnld();

public:
	static HRESULT CreateInstance(api_downloadManager *downloadManager, BOOL enableCompression, OmStorageDwnld **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);
	
	/* ifc_omstorageasync */
	HRESULT GetState(UINT *state);
	HRESULT GetWaitHandle(HANDLE *handle);
	HRESULT GetData(void **data);
	
	/* ifc_downloadManagerCallback */
	void OnFinish(DownloadToken token);
	void OnTick(DownloadToken token);
	void OnError(DownloadToken token, int errorCode);
	void OnCancel(DownloadToken token);
	void OnConnect(DownloadToken token);
	void OnInit(DownloadToken token);

	HRESULT GetResultCode();
	HRESULT GetBuffer(void **buffer, size_t *bufferSize);

	HRESULT SetData(void *data);
	HRESULT SetCallback(AsyncCallback callback);
	HRESULT GetCallback(AsyncCallback *callback);

	HRESULT RequestAbort(BOOL fDrop);

	HRESULT SetServiceHost(ifc_omservicehost *host);
	HRESULT GetServiceHost(ifc_omservicehost **host);

protected:
	void DownloadCompleted(INT errorCode);

	size_t ref;
	AsyncCallback userCallback;
	void *userData;
	HANDLE completed;
	UINT flags;
	UINT opState;
	INT	 resultCode;
	CRITICAL_SECTION lock;
	DownloadToken cookie;
	api_downloadManager *manager;
	ifc_omservicehost *serviceHost;

	RECVS_MULTIPATCH;
};

#endif //NULLSOFT_WINAMP_OMSTORAGE_DOWNLOADER_HEADER