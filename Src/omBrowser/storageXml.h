#ifndef NULLSOFT_WINAMP_OMSTORAGE_XML_HEADER
#define NULLSOFT_WINAMP_OMSTORAGE_XML_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omfilestorage.h"
#include <bfc/multipatch.h>

#define MPIID_OMSTORAGE			10
#define MPIID_OMFILESTORAGE		20

class OmStorageXml : public MultiPatch<MPIID_OMSTORAGE, ifc_omstorage>,
					public MultiPatch<MPIID_OMFILESTORAGE, ifc_omfilestorage>
{
protected:
	OmStorageXml();
	~OmStorageXml();

public:
	static HRESULT CreateInstance(OmStorageXml **instance);

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
	
	/* ifc_omfilestorage */
	HRESULT GetFilter(LPWSTR pszBuffer, UINT cchBufferMax);

protected:
	RECVS_MULTIPATCH;

protected:
	ULONG ref;
};

#endif //NULLSOFT_WINAMP_OMSTORAGE_XML_HEADER