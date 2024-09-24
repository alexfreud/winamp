#ifndef NULLSOFT_WINAMP_OMSTORAGE_INI_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_INI_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omfilestorage.h"
#include <bfc/multipatch.h>

#define MPIID_OMSTORAGE			10
#define MPIID_OMFILESTORAGE		20

class OmStorageIni : public MultiPatch<MPIID_OMSTORAGE, ifc_omstorage>,
					public MultiPatch<MPIID_OMFILESTORAGE, ifc_omfilestorage>
{
protected:
	OmStorageIni();
	~OmStorageIni();

public:
	static HRESULT CreateInstance(OmStorageIni **instance);

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
	HRESULT Reload(ifc_omservice **serviceList, ULONG listCount, ULONG *reloadedCount);
	HRESULT BeginLoad(LPCWSTR pszAddress, ifc_omservicehost *serviceHost, ifc_omstorageasync::AsyncCallback callback, void *data, ifc_omstorageasync **async);
	HRESULT EndLoad(ifc_omstorageasync *async, ifc_omserviceenum **ppEnum);
	HRESULT RequestAbort(ifc_omstorageasync *async, BOOL fDrop);

	/* ifc_omfilestorage */
	HRESULT GetFilter(LPWSTR pszBuffer, UINT cchBufferMax);
	
protected:
	RECVS_MULTIPATCH;

protected:
	ULONG ref;
};

#endif //NULLSOFT_WINAMP_OMSTORAGE_INI_HEADER