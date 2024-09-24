#include "main.h"
#include "./browserUiHook.h"
#include "./browserUiCommon.h"
#include "./browserUiInternal.h"
#include "./browserPopup.h"
#include "./browserHost.h"

#include "./ifc_omservice.h"
#include "./ifc_omserviceeventmngr.h"
#include "./ifc_omservicecommand.h"
#include "./ifc_omtoolbarconfig.h"
#include "./ifc_omstatusbarconfig.h"
#include "./ifc_omserviceeditor.h"
#include "./ifc_omconfig.h"
#include "./obj_ombrowser.h"

#include "./toolbar.h"
#include "./statusbar.h"

#include "../winamp/IWasabiDispatchable.h"
#include "../winamp/JSAPI_Info.h"

#include <strsafe.h>

BrowserUiHook::BrowserUiHook(HWND hTarget, BOOL fPopupMode) 
	: ref(1), popupMode(fPopupMode), hwnd(hTarget), winampCookie(NULL), configCookie(NULL)
{
}

BrowserUiHook::~BrowserUiHook() 
{
}

HRESULT BrowserUiHook::CreateInstance(HWND hTarget, BOOL fPopupMode, BrowserUiHook **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	if (NULL == hTarget || FALSE == IsWindow(hTarget))
		return E_INVALIDARG;

	*instance = new BrowserUiHook(hTarget, fPopupMode);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t BrowserUiHook::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t BrowserUiHook::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int BrowserUiHook::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_WinampHook))
		*object = static_cast<ifc_winamphook*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmServiceEvent))
		*object = static_cast<ifc_omserviceevent*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmConfigCallback))
		*object = static_cast<ifc_omconfigcallback*>(this);
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


HRESULT BrowserUiHook::SkinChanging(void)
{
	if (FALSE != popupMode)
	{
		BrowserPopup_SkinRefreshing(hwnd);
		PostMessage(hwnd, NBPM_SKINREFRESHED, 0, 0L);
	}
	return S_OK;
}

HRESULT BrowserUiHook::SkinChanged(const wchar_t *skinName)
{
	PostMessage(hwnd, NBCM_UPDATESKIN, 0, TRUE);
	return S_OK;
}

HRESULT BrowserUiHook::SkinColorChange(const wchar_t *colorTheme)
{
	PostMessage(hwnd, NBCM_UPDATESKIN, 0, TRUE);
	return S_OK;
}

HRESULT BrowserUiHook::ResetFont(void)
{
	PostMessage(hwnd, NBCM_UPDATESKIN, 0, TRUE);
	return S_OK;
}

void BrowserUiHook::ServiceChange(ifc_omservice *service, UINT nModified)
{
	ifc_omservice *windowService;
	if (FALSE == (BOOL)SendMessage(hwnd, NBCM_GETSERVICE, 0, (LPARAM)&windowService) || NULL == windowService)
		return;

    if (windowService != service) return;

	if (0 != (ifc_omserviceeditor::modifiedName & nModified))
	{
		IDispatch *pDispatch;
		if (SUCCEEDED(windowService->GetExternal(&pDispatch) && NULL != pDispatch))
		{
			IWasabiDispatchable *pWasabi;
			if (SUCCEEDED(pDispatch->QueryInterface(IID_IWasabiDispatchable, (void**)&pWasabi)))
			{
				JSAPI::ifc_info *info = NULL;
				if (SUCCEEDED(pWasabi->QueryDispatchable(JSAPI::IID_JSAPI_ifc_info, (Dispatchable**)&info)))
				{
					WCHAR szName[512] = {0};
					if (FAILED(service->GetName(szName, ARRAYSIZE(szName))))
						StringCchCopy(szName, ARRAYSIZE(szName), L"Unknown");
					
					info->SetName(szName);
					info->Release();
				}
				pWasabi->Release();
			}
			pDispatch->Release();
		}
	
		if (FALSE != popupMode)
			PostMessage(hwnd, NBPM_REFRESHTITLE, 0, 0L);
	}

	if (0 != (ifc_omserviceeditor::modifiedRating & nModified))
	{
		HWND hToolbar = BrowserControl_GetToolbar(hwnd);
		if (NULL != hToolbar)
		{
			UINT rating;
			if(SUCCEEDED(service->GetRating(&rating)))
			{
				Toolbar_SetItemInt(hToolbar, TOOLITEM_USERRATING, rating);
			}
		}
	}

	if (0 != (ifc_omserviceeditor::modifiedGeneration & nModified))
	{
		HWND hHost = BrowserControl_GetHost(hwnd);
		if (NULL != hHost) PostMessage(hHost, NBHM_UPDATEEXTERNAL, 0 , 0L);
	}

}

void BrowserUiHook::CommandStateChange(ifc_omservice *service, const GUID *commandGroup, unsigned int commandId)
{
	if (NULL != commandGroup)
	{
		if (IsEqualGUID(CMDGROUP_SERVICE, *commandGroup))
		{
			switch(commandId)
			{
				case SVCCOMMAND_BLOCKNAV:
					CheckBlockedState(service);
					break;

			}

		}
	}
}

HRESULT BrowserUiHook::ValueChanged(const GUID *configUid, UINT valueId, ULONG_PTR value)
{
	if (NULL == configUid) 
		return E_UNEXPECTED;

	if (IsEqualIID(IFC_OmToolbarConfig, *configUid))
	{
		HWND hToolbar = (HWND)SendMessage(hwnd, NBCM_GETTOOLBAR, 0, 0L);
		if (NULL == hToolbar) return S_FALSE;

		switch(valueId)
		{
			case CFGID_TOOLBAR_BOTTOMDOCK:		Toolbar_EnableBottomDock(hToolbar, (BOOL)value); break;
			case CFGID_TOOLBAR_AUTOHIDE:		Toolbar_EnableAutoHide(hToolbar, (BOOL)value); break;
			case CFGID_TOOLBAR_TABSTOP:			Toolbar_EnableTabStop(hToolbar, (BOOL)value); break;
			case CFGID_TOOLBAR_FORCEADDRESS:	Toolbar_EnableForceAddress(hToolbar, (BOOL)value); break;
			case CFGID_TOOLBAR_FANCYADDRESS:	Toolbar_EnableFancyAddress(hToolbar, (BOOL)value); break;
		}
	}
	else if (IsEqualIID(IFC_OmStatusbarConfig, *configUid))
	{
		HWND hStatusbar = (HWND)SendMessage(hwnd, NBCM_GETSTATUSBAR, 0, 0L);
		if (NULL == hStatusbar) return S_FALSE;

		switch(valueId)
		{
			case CFGID_STATUSBAR_ENABLED:	Statusbar_Enable(hStatusbar, (BOOL)value); break; 
		}
	}
	 
	return S_OK;
}

HRESULT BrowserUiHook::Register(obj_ombrowser *browserManager, ifc_omservice *service)
{
	
	Plugin_RegisterWinampHook(this, &winampCookie);

	ifc_omconfig *config;
	if (NULL != browserManager && SUCCEEDED(browserManager->GetConfig(NULL, (void**)&config)))
	{
		config->RegisterCallback(this, &configCookie);
		config->Release();
	}

	ifc_omserviceeventmngr *eventManager;
	if (NULL != service && SUCCEEDED(service->QueryInterface(IFC_OmServiceEventMngr, (void**)&eventManager)))
	{
		eventManager->RegisterHandler(this);
		eventManager->Release();
	}

	return S_OK;
}

HRESULT BrowserUiHook::Unregister(obj_ombrowser *browserManager, ifc_omservice *service)
{
	if (0 != configCookie)
	{
		ifc_omconfig *config;
		if (NULL != browserManager && SUCCEEDED(browserManager->GetConfig(NULL, (void**)&config)))
		{
			config->UnregisterCallback(configCookie);
			config->Release();
		}
		configCookie = 0;
	}

	ifc_omserviceeventmngr *eventManager;
	if (NULL != service && SUCCEEDED(service->QueryInterface(IFC_OmServiceEventMngr, (void**)&eventManager)))
	{
		eventManager->UnregisterHandler(this);
		eventManager->Release();
	}
	
	
	if (0 != winampCookie)
	{
		Plugin_UnregisterWinampHook(winampCookie);
		winampCookie = 0;
	}

	return S_OK;
}

HRESULT BrowserUiHook::CheckBlockedState(ifc_omservice *service)
{
	if (NULL == service)
		return E_INVALIDARG;

	HRESULT hr;

	ifc_omservicecommand *serviceCommand;
	hr = service->QueryInterface(IFC_OmServiceCommand, (void**)&serviceCommand);
	if (SUCCEEDED(hr))
	{
		HRESULT state = serviceCommand->QueryState(hwnd, &CMDGROUP_SERVICE, SVCCOMMAND_BLOCKNAV);
		PostMessage(hwnd, NBCM_BLOCK, 0, (CMDSTATE_ENABLED == state));
		serviceCommand->Release();
	}
	return hr;
}

#define CBCLASS BrowserUiHook
START_MULTIPATCH;
 START_PATCH(MPIID_WINAMPHOOK)
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, ADDREF, AddRef);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, RELEASE, Release);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, API_SKINCHANGING, SkinChanging);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, API_SKINCHANGED, SkinChanged);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, API_SKINCOLORCHANGE, SkinColorChange);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, API_RESETFONT, ResetFont);
 NEXT_PATCH(MPIID_SERVICEEVENT)
  M_CB(MPIID_SERVICEEVENT, ifc_omserviceevent, ADDREF, AddRef);
  M_CB(MPIID_SERVICEEVENT, ifc_omserviceevent, RELEASE, Release);
  M_CB(MPIID_SERVICEEVENT, ifc_omserviceevent, QUERYINTERFACE, QueryInterface);
  M_VCB(MPIID_SERVICEEVENT, ifc_omserviceevent, API_SERVICECHANGE, ServiceChange);
  M_VCB(MPIID_SERVICEEVENT, ifc_omserviceevent, API_COMMANDSTATECHANGE, CommandStateChange);

 NEXT_PATCH(MPIID_CONFIGCALLBACK)
  M_CB(MPIID_CONFIGCALLBACK, ifc_omconfigcallback, ADDREF, AddRef);
  M_CB(MPIID_CONFIGCALLBACK, ifc_omconfigcallback, RELEASE, Release);
  M_CB(MPIID_CONFIGCALLBACK, ifc_omconfigcallback, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_CONFIGCALLBACK, ifc_omconfigcallback, API_VALUECHANGED, ValueChanged);
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS



