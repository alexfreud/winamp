#include "main.h"
#include "./serviceHost.h"
#include "./wasabi.h"
#include "./resource.h"
#include "./external.h"
#include "./navigation.h"
#include "./commands.h"
#include "./serviceHelper.h"

#include <ifc_omservice.h>
#include <ifc_omserviceeditor.h>
#include <ifc_omservicecommand.h>
#include <browserView.h>

#include "../winamp/wa_ipc.h"
#include "../winamp/IWasabiDispatchable.h"
#include "../winamp/JSAPI_Info.h"

#include <strsafe.h>


#define IS_INVALIDISPATCH(__disp) (((IDispatch *)1) == (__disp) || NULL == (__disp))

static WebDevServiceHost *cachedInstance = NULL;

WebDevServiceHost::WebDevServiceHost()
	: ref(1)
{

}

WebDevServiceHost::~WebDevServiceHost()
{

}

HRESULT WebDevServiceHost::CreateInstance(WebDevServiceHost **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new WebDevServiceHost();
	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT WebDevServiceHost::GetCachedInstance(WebDevServiceHost **instance)
{
	if (NULL == instance) 
		return E_POINTER;


	if (NULL == cachedInstance)
	{
		HRESULT hr = CreateInstance(&cachedInstance);
		if (FAILED(hr))
		{
			*instance = NULL;
			return hr;
		}
	}

	cachedInstance->AddRef();
	*instance = cachedInstance;
	return S_OK;
}

HRESULT WebDevServiceHost::ReleseCache()
{
	if (NULL == cachedInstance)
		return S_FALSE;
	
	WebDevServiceHost *t = cachedInstance;
	cachedInstance = NULL;
	t->Release();
	
	return S_OK;
}

size_t WebDevServiceHost::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t WebDevServiceHost::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int WebDevServiceHost::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmServiceHost))
		*object = static_cast<ifc_omservicehost*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmServiceEvent))
		*object = static_cast<ifc_omserviceevent*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT WebDevServiceHost::GetExternal(ifc_omservice *service, IDispatch **ppDispatch)
{
	if (NULL == ppDispatch) 
		return E_POINTER;
	
	if (NULL != *ppDispatch)
	{
		// try to connect our external
		IWasabiDispatchable *pWasabi;
		if (SUCCEEDED((*ppDispatch)->QueryInterface(IID_IWasabiDispatchable, (void**)&pWasabi)))
		{
			JSAPI::ifc_info *pInfo;
			if (SUCCEEDED(pWasabi->QueryDispatchable(JSAPI::IID_JSAPI_ifc_info, (Dispatchable**)&pInfo)))
			{
				ExternalDispatch *pExternal;
				if (SUCCEEDED(ExternalDispatch::CreateInstance(&pExternal)))
				{
					pInfo->AddAPI(pExternal->GetName(), pExternal);
					pExternal->Release();
				}
				pInfo->Release();
			}
			pWasabi->Release();
		}
	}

	return S_OK;
}

HRESULT WebDevServiceHost::GetBasePath(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) 
		return E_POINTER;
	
	return StringCchCopy(pszBuffer, cchBufferMax, L".\\Plugins\\webDev");
}

HRESULT WebDevServiceHost::GetDefaultName(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	if (NULL == service) 
		return E_INVALIDARG;
	
	return StringCchPrintf(pszBuffer, cchBufferMax, L"wdService_{%010u}.ini", service->GetId());
}

HRESULT WebDevServiceHost::QueryCommandState(ifc_omservice *service, HWND hBrowser, const GUID *commandGroup, UINT commandId)
{
	if (NULL == service || NULL == commandGroup)
		return E_NOTIMPL;
	
	if (IsEqualGUID(*commandGroup, CMDGROUP_ADDRESSBAR))
	{
		switch(commandId)
		{
			case ADDRESSCOMMAND_VISIBLE:
				if (S_FALSE == ServiceHelper_IsSpecial(service))
					return CMDSTATE_ENABLED;
				return CMDSTATE_UNKNOWN;
			case ADDRESSCOMMAND_READONLY:
				if (S_FALSE == ServiceHelper_IsSpecial(service))
					return CMDSTATE_DISABLED;
				return CMDSTATE_ENABLED;
		}
	}

	return E_NOTIMPL;
}

HRESULT WebDevServiceHost::ExecuteCommand(ifc_omservice *service, HWND hBrowser, const GUID *commandGroup, UINT commandId, ULONG_PTR commandArg)
{
	return E_NOTIMPL;
}

HRESULT WebDevServiceHost::GetUrl(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax)
{
	return E_NOTIMPL;
}


void WebDevServiceHost::ServiceChange(ifc_omservice *service, UINT nModified)
{
	if (NULL == service) return;
	Navigation *navigation;
	if (SUCCEEDED(Plugin_GetNavigation(&navigation)))
	{
		navigation->UpdateService(service, nModified);
		navigation->Release();
	}
	
	if ( 0 != (ifc_omserviceeditor::modifiedUrl & nModified))
	{
		Command_PostNavigateSvc(service, MAKEINTRESOURCE
		(NAVIGATE_HOME), TRUE);
	}
	if (0 != (ifc_omserviceeditor::modifiedFlags & nModified))
	{
		ServiceHelper_RegisterPreAuthorized(service);
	}
}

#define CBCLASS WebDevServiceHost
START_MULTIPATCH;
 START_PATCH(MPIID_OMSVCHOST)
  M_CB(MPIID_OMSVCHOST, ifc_omservicehost, ADDREF, AddRef);
  M_CB(MPIID_OMSVCHOST, ifc_omservicehost, RELEASE, Release);
  M_CB(MPIID_OMSVCHOST, ifc_omservicehost, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSVCHOST, ifc_omservicehost, API_GETEXTERNAL, GetExternal);
  M_CB(MPIID_OMSVCHOST, ifc_omservicehost, API_GETBASEPATH, GetBasePath);
  M_CB(MPIID_OMSVCHOST, ifc_omservicehost, API_GETDEFAULTNAME, GetDefaultName);
  M_CB(MPIID_OMSVCHOST, ifc_omservicehost, API_QUERYCOMMANDSTATE, QueryCommandState);
  M_CB(MPIID_OMSVCHOST, ifc_omservicehost, API_EXECUTECOMMAND, ExecuteCommand);
  M_CB(MPIID_OMSVCHOST, ifc_omservicehost, API_GETURL, GetUrl);
   
 NEXT_PATCH(MPIID_OMSVCEVENT)
  M_CB(MPIID_OMSVCEVENT, ifc_omserviceevent, ADDREF, AddRef);
  M_CB(MPIID_OMSVCEVENT, ifc_omserviceevent, RELEASE, Release);
  M_CB(MPIID_OMSVCEVENT, ifc_omserviceevent, QUERYINTERFACE, QueryInterface);
  M_VCB(MPIID_OMSVCEVENT, ifc_omserviceevent, API_SERVICECHANGE, ServiceChange);
    
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS
