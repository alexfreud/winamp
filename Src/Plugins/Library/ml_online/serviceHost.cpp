#include "main.h"
#include "./serviceHost.h"
#include "./api__ml_online.h"
#include "./resource.h"
#include "./external.h"
#include "./navigation.h"
#include "./commands.h"
#include "./serviceHelper.h"

#include <ifc_omservice.h>
#include <ifc_omserviceeditor.h>
#include <ifc_omservicecommand.h>

#include <ifc_omstoragehelper.h>
#include <ifc_omstoragehandlerenum.h>
#include <ifc_omfilestorage.h>

#include "../winamp/IWasabiDispatchable.h"
#include "../winamp/JSAPI_Info.h"

#include <shlwapi.h>
#include <strsafe.h>


#define IS_INVALIDISPATCH(__disp) (((IDispatch *)1) == (__disp) || NULL == (__disp))

static ServiceHost *cachedInstance = NULL;

static void CALLBACK StorageHandler_ReadAuth(ifc_omservice *service, LPCWSTR pszKey, LPCWSTR pszValue)
{
	INT iVal;
	UINT flags = (NULL != pszValue && 
						FALSE != StrToIntEx(pszValue, STIF_SUPPORT_HEX, &iVal) && 
						0 != iVal) ?
						SVCF_USECLIENTOWEB : 0;

	ServiceHelper_SetFlags(service, flags, SVCF_USECLIENTOWEB);
}


static void CALLBACK StorageHandler_ReadBypass(ifc_omservice *service, LPCWSTR pszKey, LPCWSTR pszValue)
{
	INT iVal;
	UINT flags = (NULL != pszValue && 
						FALSE != StrToIntEx(pszValue, STIF_SUPPORT_HEX, &iVal) && 
						0 != iVal) ?
						SVCF_PREAUTHORIZED : 0;

	ServiceHelper_SetFlags(service, flags, SVCF_PREAUTHORIZED);
	
}

static void CALLBACK StorageHandler_ReadSubscribed(ifc_omservice *service, LPCWSTR pszKey, LPCWSTR pszValue)
{
	INT iVal;
	UINT flags = (NULL != pszValue && 
						FALSE != StrToIntEx(pszValue, STIF_SUPPORT_HEX, &iVal) && 
						0 != iVal) ?
						SVCF_SUBSCRIBED : 0;

	flags |= SVCF_AUTOUPGRADE;
	ServiceHelper_SetFlags(service, flags, SVCF_SUBSCRIBED | SVCF_AUTOUPGRADE);
	
}

static const ifc_omstoragehelper::TemplateRecord szStorageExtXml[] =
{
	{ L"auth", StorageHandler_ReadAuth },
	{ L"bypass", StorageHandler_ReadBypass },
};

static const ifc_omstoragehelper::TemplateRecord szStorageExtIni[] =
{
	{ L"subscribed", StorageHandler_ReadSubscribed },
};

ServiceHost::ServiceHost()
	: ref(1), storageExtXml(NULL), storageExtIni(NULL)
{

}

ServiceHost::~ServiceHost()
{
	if (NULL != storageExtXml)
		storageExtXml->Release();

	if (NULL != storageExtIni)
		storageExtIni->Release();
}

HRESULT ServiceHost::CreateInstance(ServiceHost **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new ServiceHost();
	if (NULL == instance) return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT ServiceHost::GetCachedInstance(ServiceHost **instance)
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

HRESULT ServiceHost::ReleseCache()
{
	if (NULL == cachedInstance)
		return S_FALSE;
	
	ServiceHost *t = cachedInstance;
	cachedInstance = NULL;
	t->Release();
	
	return S_OK;
}

size_t ServiceHost::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ServiceHost::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int ServiceHost::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmServiceHost))
		*object = static_cast<ifc_omservicehost*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmServiceEvent))
		*object = static_cast<ifc_omserviceevent*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmStorageExt))
		*object = static_cast<ifc_omstorageext*>(this);
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


HRESULT ServiceHost::GetExternal(ifc_omservice *service, IDispatch **ppDispatch)
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

HRESULT ServiceHost::GetBasePath(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) 
		return E_POINTER;
	
	return StringCchCopy(pszBuffer, cchBufferMax, L".\\Plugins\\ml\\omServices");
}

HRESULT ServiceHost::GetDefaultName(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	if (NULL == service) 
		return E_INVALIDARG;
	
	return StringCchPrintf(pszBuffer, cchBufferMax, L"omService_{%010u}.ini", service->GetId());
}

HRESULT ServiceHost::QueryCommandState(ifc_omservice *service, HWND hBrowser, const GUID *commandGroup, UINT commandId)
{
	if (NULL == service || NULL == commandGroup)
		return E_NOTIMPL;
	
	if (IsEqualGUID(*commandGroup, CMDGROUP_SERVICE))
	{
		switch(commandId)
		{
			case SVCCOMMAND_SHOWINFO:
			case SVCCOMMAND_REPORT:
			case SVCCOMMAND_UNSUBSCRIBE:
			case SVCCOMMAND_RATE:
				if (S_FALSE == ServiceHelper_IsSpecial(service))
				{
					return CMDSTATE_ENABLED;
				}
				return CMDSTATE_UNKNOWN;
			
			case SVCCOMMAND_BLOCKNAV:		
				{
					UINT flags;
					if (FAILED(service->GetFlags(&flags)))
						flags = 0;

					HRESULT state = (0 == (SVCF_VERSIONCHECK & flags)) ? 
						CMDSTATE_DISABLED : CMDSTATE_ENABLED;
					
					ServiceHelper_UpdateOperationInfo(hBrowser);
					return state;
				}
				break;
		}
	}
	return E_NOTIMPL;
}

HRESULT ServiceHost::ExecuteCommand(ifc_omservice *service, HWND hBrowser, const GUID *commandGroup, UINT commandId, ULONG_PTR commandArg)
{
	if (IsEqualGUID(CMDGROUP_SERVICE, *commandGroup))
	{
		if (S_OK != ServiceHelper_IsSpecial(service))
		{
			switch(commandId)
			{
				case SVCCOMMAND_SHOWINFO:		Command_ShowServiceInfo(service); return S_OK;
				case SVCCOMMAND_REPORT:			Command_ReportService(service); return S_OK;
				case SVCCOMMAND_UNSUBSCRIBE:	Command_UnsubscribeService(service); return S_OK;
				case SVCCOMMAND_RATE:			Command_SetServiceRating(service, (UINT)commandArg); return S_OK;
			}
		}
	}

	return E_NOTIMPL;
}

void ServiceHost::ServiceChange(ifc_omservice *service, UINT nModified)
{
	if (NULL == service) return;

	Navigation *navigation;
	if (SUCCEEDED(Plugin_GetNavigation(&navigation)))
	{
		navigation->UpdateService(service, nModified);
		navigation->Release();
	}
	
}

HRESULT ServiceHost::EnumerateStorageExt(const GUID *storageId, ifc_omstoragehandlerenum **enumerator)
{
	if (NULL == storageId) 
		return E_INVALIDARG;
	
	if (IsEqualGUID(SUID_OmStorageXml, *storageId))
	{
		if (NULL == storageExtXml)
		{
			ifc_omstoragehelper *storageHelper;
			if (NULL != OMUTILITY && SUCCEEDED(OMUTILITY->GetStorageHelper(&storageHelper)))
			{
				if (FAILED(storageHelper->CreateEnumerator(szStorageExtXml, ARRAYSIZE(szStorageExtXml), &storageExtXml)))
					storageExtXml = NULL;
				storageHelper->Release();

			}

			if (NULL == storageExtXml)
				return E_FAIL;
		}

		*enumerator = storageExtXml;
		storageExtXml->AddRef();
		return S_OK;
	}
	else if (IsEqualGUID(SUID_OmStorageIni, *storageId))
	{
		if (NULL == storageExtIni)
		{
			ifc_omstoragehelper *storageHelper;
			if (NULL != OMUTILITY && SUCCEEDED(OMUTILITY->GetStorageHelper(&storageHelper)))
			{
				if (FAILED(storageHelper->CreateEnumerator(szStorageExtIni, ARRAYSIZE(szStorageExtIni), &storageExtIni)))
					storageExtIni = NULL;
				storageHelper->Release();

			}

			if (NULL == storageExtIni)
				return E_FAIL;
		}

		*enumerator = storageExtIni;
		storageExtIni->AddRef();
		return S_OK;

	}
	return E_NOTIMPL;
}

HRESULT ServiceHost::GetUrl(ifc_omservice *service, LPWSTR pszBuffer, UINT cchBufferMax)
{
	UINT flags;
	if (NULL != service && 	SUCCEEDED(service->GetFlags(&flags)) && 
		0 != (SVCF_USECLIENTOWEB & flags) && NULL != AGAVE_API_AUTH)
	{
		LPWSTR pszUrl = Plugin_CopyString(pszBuffer);
		if (NULL == pszUrl) return E_OUTOFMEMORY;
		
		HRESULT hr(E_NOTIMPL);

		if (0 == AGAVE_API_AUTH->ClientToWeb(GUID_NULL, pszUrl, pszBuffer, cchBufferMax))
			hr = S_OK;

		Plugin_FreeString(pszUrl);
		return hr;
	}
	
	return E_NOTIMPL;
}

#define CBCLASS ServiceHost
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

 NEXT_PATCH(MPIID_OMSTRGEXT)
  M_CB(MPIID_OMSTRGEXT, ifc_omstorageext, ADDREF, AddRef);
  M_CB(MPIID_OMSTRGEXT, ifc_omstorageext, RELEASE, Release);
  M_CB(MPIID_OMSTRGEXT, ifc_omstorageext, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSTRGEXT, ifc_omstorageext, API_ENUMERATE, EnumerateStorageExt);
    
  
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS
