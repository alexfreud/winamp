#ifndef NULLSOFT_WEBDEV_PLUGIN_SERVICE_HOST_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_SERVICE_HOST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <ifc_omservicehost.h>
#include <ifc_omserviceevent.h>
#include <bfc/multipatch.h>

#define MPIID_OMSVCHOST		10
#define MPIID_OMSVCEVENT		20

class WebDevServiceHost :	public MultiPatch<MPIID_OMSVCHOST, ifc_omservicehost>,
							public MultiPatch<MPIID_OMSVCEVENT, ifc_omserviceevent>
{

protected:
	WebDevServiceHost();
	~WebDevServiceHost();

public:
	static HRESULT CreateInstance(WebDevServiceHost **instance);
	static HRESULT GetCachedInstance(WebDevServiceHost **instance);
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

protected:
	ULONG ref;

protected:
	RECVS_MULTIPATCH;
};




#endif //NULLSOFT_WEBDEV_PLUGIN_SERVICE_HOST_HEADER