#ifndef NULLSOFT_WINAMP_OMSTORAGE_URL_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_URL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omwebstorage.h"

class api_downloadManager;

class OmStorageUrl : public ifc_omstorage
{
protected:
	OmStorageUrl();
	~OmStorageUrl();

public:
	static HRESULT CreateInstance(OmStorageUrl **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omstorage */
	HRESULT GetId(GUID *storageUid);
	HRESULT GetType(GUID *storageType);
	UINT GetCapabilities();
	HRESULT GetDescription(LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT Load(LPCWSTR pszAddress, ifc_omservicehost *host, ifc_omserviceenum **ppEnum);
	HRESULT Save(ifc_omservice **serviceList, ULONG listCount, UINT saveFlags, ULONG *savedCount);
	HRESULT Delete(ifc_omservice **serviceList, ULONG listCount, ULONG *deletedCount);
	HRESULT BeginLoad(LPCWSTR pszAddress, ifc_omservicehost *serviceHost, ifc_omstorageasync::AsyncCallback callback, void *data, ifc_omstorageasync **async);
	HRESULT EndLoad(ifc_omstorageasync *async, ifc_omserviceenum **ppEnum);
	HRESULT RequestAbort(ifc_omstorageasync *async, BOOL fDrop);
	
protected:
	ULONG ref;
	api_downloadManager *manager;

protected:
	RECVS_DISPATCH;
};

#endif //NULLSOFT_WINAMP_OMSTORAGE_URL_HEADER