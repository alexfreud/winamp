#ifndef NULLSOFT_AUTH_LOGIN_DOWNLOAD_RESULT_HEADER
#define NULLSOFT_AUTH_LOGIN_DOWNLOAD_RESULT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../dlmgr/api_downloadmanager.h"

#define E_DWNLD_OK				S_OK
#define E_DWNLD_BUSY			E_PENDING
#define E_DWNLD_FAIL			E_FAIL
#define E_DWNLD_ABORT			E_ABORT
#define E_DWNLD_TIMEOUT			HRESULT_FROM_WIN32(ERROR_TIMEOUT)
#define E_DWNLD_CANT_CONNECT	HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED)
#define E_DWNLD_WRITE_ERROR		HRESULT_FROM_WIN32(ERROR_WRITE_FAULT)

class LoginStatus;

class LoginDownloadResult : public ifc_downloadManagerCallback
{
public:
	typedef void (CALLBACK *Callback)(LoginDownloadResult *result, void *data);

	typedef enum
	{
		stateMask = 0x00000FF00,
		stateReady = 0x00000000,
		stateInitializing = 0x00000100,
		stateConnecting = 0x00000200,
		stateReceiving = 0x00000300,
		stateCompleted = 0x00000400,
		stateAborting = 0x00000500,
	} States;

	typedef enum
	{
		typeMask = 0x0000000FF,
		typeUnknown	= 0x00000000,
		typeImage = 0x00000001,
		typeProviderList = 0x00000002,
	} Types;

	typedef enum
	{
		flagsMask = 0xFFFF0000,
		flagUserAbort = 0x00010000,
	} Flags;

protected:
	LoginDownloadResult(api_downloadManager *pManager, UINT uType, Callback fnCallback, void *pData, LoginStatus *pStatus);
	~LoginDownloadResult();

public:
	static HRESULT CreateInstance(api_downloadManager *pManager, UINT uType, Callback fnCallback, void *pData, LoginStatus *pStatus, LoginDownloadResult **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);
			
	HRESULT GetWaitHandle(HANDLE *handle);
	HRESULT GetData(void **data);
	HRESULT GetType(UINT *type);
	HRESULT GetState(UINT *state);
	HRESULT GetFile(LPCWSTR *ppszPath);

	HRESULT CreateDownloadFileName(LPSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetUrl(LPSTR pszBuffer, UINT cchBufferMax);
	HRESULT RequestAbort(BOOL fDrop);

protected:
	/* ifc_downloadManagerCallback */
	void Event_DownloadFinish(DownloadToken token);
	void Event_DownloadTick(DownloadToken token);
	void Event_DownloadError(DownloadToken token, int errorCode);
	void Event_DownloadCancel(DownloadToken token);
	void Event_DownloadConnect(DownloadToken token);
	void Event_DownloadInit(DownloadToken token);

	void DownloadCompleted(int errorCode);
	void SetState(UINT uState);
	void SetType(UINT uType);
	void SetFlags(UINT uFlags, UINT uMask);
	void SetStatus();
	void RemoveStatus();

	

protected:
	size_t ref;
	UINT flags;
	LPWSTR address;
	INT result;
	api_downloadManager *manager;
	DownloadToken cookie;
	HANDLE completed;
	Callback callback;
	void *data;
	LoginStatus *status;
	UINT		statusCookie;
	CRITICAL_SECTION lock;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_AUTH_LOGINDOWNLOADER_HEADER