#ifndef NULLSOFT_ONLINEMEDIA_PLUGIN_SERVICE_HOST_HEADER
#define NULLSOFT_ONLINEMEDIA_PLUGIN_SERVICE_HOST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <ifc_omservicehost.h>
#include <ifc_omserviceevent.h>
#include <ifc_omstorageext.h>
#include <bfc/multipatch.h>

#define SVCF_SUBSCRIBED		0x00000001

// runtime flags
#define SVCF_SPECIAL		0x00010000
#define SVCF_USECLIENTOWEB	0x00020000
#define SVCF_VALIDATED		0x00040000
#define SVCF_VERSIONCHECK	0x00080000
#define SVCF_PREAUTHORIZED	0x00100000
#define SVCF_AUTOUPGRADE	0x00200000

#define MPIID_OMSVCHOST		10
#define MPIID_OMSVCEVENT	20
#define MPIID_OMSTRGEXT		30

class ifc_omstoragehandlerenum;

class ServiceHost :	public MultiPatch<MPIID_OMSVCHOST, ifc_omservicehost>,
					public MultiPatch<MPIID_OMSVCEVENT, ifc_omserviceevent>,
					public MultiPatch<MPIID_OMSTRGEXT, ifc_omstorageext>
{

protected:
	ServiceHost();
	~ServiceHost();

public:
	static HRESULT CreateInstance(ServiceHost **instance);
	static HRESULT GetCachedInstance(ServiceHost **instance);
	static HRESULT ReleseCache();

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omservicehost */
	HRESULT GetExternal(ifc_omservice *service, IDispatch **ppDispatch);
	HRESULT GetBasePath(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT GetDefaultName(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax);
	HRESULT QueryCommandState(ifc_omservice *service, HWND hBrowser, const GUID *commandGroup, UINT commandId);
	HRESULT ExecuteCommand(ifc_omservice *service, HWND hBrowser, const GUID *commandGroup, UINT commandId, ULONG_PTR commandArg);
	HRESULT GetUrl(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax);

	/* ifc_omsvceventhandler */
	void ServiceChange(ifc_omservice *service, UINT nModified);

	/* ifc_omstorageext */
	HRESULT EnumerateStorageExt(const GUID *storageId, ifc_omstoragehandlerenum **enumerator);

protected:
	ULONG ref;
	ifc_omstoragehandlerenum *storageExtXml;
	ifc_omstoragehandlerenum *storageExtIni;

protected:
	RECVS_MULTIPATCH;
};




#endif //NULLSOFT_ONLINEMEDIA_PLUGIN_SERVICE_HOST_HEADER