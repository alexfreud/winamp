#ifndef NULLSOFT_WINAMP_OMSERVICE_MANAGER_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_MANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omservicemanager.h"
#include <vector>

class OmServiceManager : public ifc_omservicemanager
{
protected:
	OmServiceManager();
	~OmServiceManager();

public:
	static OmServiceManager *CreateInstance();

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();

	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omservice */
	HRESULT RegisterStorage(ifc_omstorage *storage);
	HRESULT UnregisterStorage(const GUID *storageId);
	HRESULT QueryStorage(const GUID *storageId, ifc_omstorage **storageOut);
	HRESULT EnumStorage(const GUID *filterType, UINT filterCapabilities, ifc_omstorageenumerator **enumOut);
	HRESULT CreateService(UINT serviceId, ifc_omservicehost *host, ifc_omservice **serviceOut);

protected:
	HRESULT RegisterDefaultStorage();

protected:
	RECVS_DISPATCH;

protected:
	typedef std::vector<ifc_omstorage*> StorageList;

protected:
	ULONG ref;
	BOOL registerStorage;
	StorageList storageList;
};

#endif //NULLSOFT_WINAMP_OMSERVICE_MANAGER_HEADER