#ifndef NULLSOFT_WINAMP_OMSERVICE_MANAGER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMSERVICE_MANAGER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {BB0A9154-6D31-413b-96FB-9466E535E0C4}
static const GUID IFC_OmServiceManager = 
{ 0xbb0a9154, 0x6d31, 0x413b, { 0x96, 0xfb, 0x94, 0x66, 0xe5, 0x35, 0xe0, 0xc4 } };

class ifc_omservice;
class ifc_omstorage;
class ifc_omstorageenumerator;
class ifc_omservicehost;
class ifc_omserviceeventmngr;

class __declspec(novtable) ifc_omservicemanager : public Dispatchable
{
protected:
	ifc_omservicemanager() {}
	~ifc_omservicemanager() {}

public:
	HRESULT RegisterStorage(ifc_omstorage *storage);
	HRESULT UnregisterStorage(const GUID *storageId);
	HRESULT QueryStorage(const GUID *storageId, ifc_omstorage **storageOut);
	HRESULT EnumStorage(const GUID *filterType, UINT filterCapabilities, ifc_omstorageenumerator **enumOut);
	
	HRESULT CreateService(unsigned int serviceId, ifc_omservicehost *host, ifc_omservice **serviceOut);


public:
	DISPATCH_CODES
	{	
		API_REGISTERSTORAGE = 10,
		API_UNREGISTERSTORAGE = 20,
		API_QUERYSTORAGE = 30,
		API_ENUMSTORAGE = 40,

		API_CREATESERVICE = 100,

	};
};

inline HRESULT ifc_omservicemanager::RegisterStorage(ifc_omstorage *storage)
{
	return _call(API_REGISTERSTORAGE, (HRESULT)E_NOTIMPL, storage);
}

inline HRESULT ifc_omservicemanager::UnregisterStorage(const GUID *storageId)
{
	return _call(API_UNREGISTERSTORAGE, (HRESULT)E_NOTIMPL, storageId);
}

inline HRESULT ifc_omservicemanager::QueryStorage(const GUID *storageId, ifc_omstorage **storageOut)
{
	return _call(API_QUERYSTORAGE, (HRESULT)E_NOTIMPL, storageId, storageOut);
}

inline HRESULT ifc_omservicemanager::EnumStorage(const GUID *filterType, UINT filterCapabilities, ifc_omstorageenumerator **enumOut)
{
	return _call(API_ENUMSTORAGE, (HRESULT)E_NOTIMPL, filterType, filterCapabilities, enumOut);
}

inline HRESULT ifc_omservicemanager::CreateService(UINT serviceId, ifc_omservicehost *host, ifc_omservice **serviceOut)
{
	return _call(API_CREATESERVICE, (HRESULT)E_NOTIMPL, serviceId, host, serviceOut);
}

#endif //NULLSOFT_WINAMP_OMSERVICE_MANAGER_INTERFACE_HEADER