#include "main.h"
#include "./commands.h"
#include "./wasabi.h"
#include "./resource.h"
#include "./navigation.h"
#include "./import.h"
#include "./serviceHelper.h"
#include "../../General/gen_ml/ml_ipc_0313.h"
#include <ifc_omservice.h>
#include <browserView.h>
#include <strsafe.h>

HRESULT Command_NavigateService(ifc_omservice *service, LPCWSTR pszUrl, BOOL fActiveOnly)
{
	if (NULL == service) 
		return E_INVALIDARG;

	Navigation *navigation;
	if (FAILED(Plugin_GetNavigation(&navigation))) 
		return E_UNEXPECTED;

	ifc_omservice *activeService;
	HWND hView = navigation->GetActiveView(&activeService);
	if (NULL == hView || activeService->GetId() != service->GetId())
		hView = NULL;

	if (NULL != activeService)
		activeService->Release();

	HRESULT hr = S_OK;

	if (NULL != hView)
	{
		if (FALSE == BrowserView_Navigate(hView, pszUrl, TRUE))
			hr = E_FAIL;
	}
	else
	{
		hr = (FALSE == fActiveOnly) ?
			navigation->ShowService(service->GetId(), pszUrl) : E_NOTIMPL;
	}		

	navigation->Release();
	return hr;
}
static void CALLBACK ThreadCallback_NavigateService(Dispatchable *object, ULONG_PTR param1, ULONG_PTR param2)
{
	ifc_omservice *service = (ifc_omservice*)object;
	LPWSTR pszUrl = (LPWSTR)param1;
	
	Command_NavigateService(service, pszUrl, (BOOL)param2);

	Plugin_FreeResString(pszUrl);
}

HRESULT Command_PostNavigateSvc(ifc_omservice *service, LPCWSTR pszUrl, BOOL fActiveOnly)
{
	if (NULL == OMUTILITY)
		return E_FAIL;
	
	if (NULL == service) 
		return E_INVALIDARG;
	
	LPWSTR pszUrlCopy = Plugin_DuplicateResString(pszUrl);
	HRESULT hr = OMUTILITY->PostMainThreadCallback2(ThreadCallback_NavigateService, service, (ULONG_PTR)pszUrlCopy, fActiveOnly);
	if (FAILED(hr))
	{
		Plugin_FreeResString(pszUrlCopy);
	}

	return hr;

}

HRESULT Command_EditService(ifc_omservice *service)
{
	if (NULL == service)
		return E_INVALIDARG;

	WCHAR szBuffer[2048];

	HRESULT hr = Plugin_MakeResourcePath(szBuffer, ARRAYSIZE(szBuffer), RT_HTML, MAKEINTRESOURCE(IDR_HTML_EDITOR), RESPATH_TARGETIE | RESPATH_COMPACT);
	if (FAILED(hr)) return hr;

	INT cchUrl = lstrlen(szBuffer);
	LPWSTR pszParam = szBuffer + cchUrl;
	INT cchParamMax = ARRAYSIZE(szBuffer) - cchUrl;
	hr = StringCchPrintf(pszParam, cchParamMax, L"?serviceId=%u", service->GetId());
	if (FAILED(hr)) return hr;
	
	return Command_NavigateService(service, szBuffer, FALSE);
}

HRESULT Command_ReloadService(ifc_omservice *service)
{
	if (NULL == service) 
		return E_INVALIDARG;
	
	HRESULT hr = ServiceHelper_Reload(service);
	return hr;
}

HRESULT Command_ResetPermissions(ifc_omservice *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	if (NULL == AGAVE_API_JSAPI2_SECURITY)
		return E_UNEXPECTED;
	
	WCHAR szBuffer[64] = {0};
	if (FAILED(StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), L"%u", service->GetId())))
		return E_FAIL;
	
	AGAVE_API_JSAPI2_SECURITY->ResetAuthorization(szBuffer);
	return S_OK;
}

HRESULT Command_LocateService(ifc_omservice *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	WCHAR szPath[512];
	
	HRESULT hr = service->GetAddress(szPath, ARRAYSIZE(szPath));
	if (FAILED(hr)) return hr;

	if (L'\0' == szPath[0]) 
		return E_FAIL;

	Navigation *navigation;
	if (FAILED(Plugin_GetNavigation(&navigation))) 
		return E_UNEXPECTED;

	navigation->Release();

	if (WASABI_API_EXPLORERFINDFILE)
	{
		WASABI_API_EXPLORERFINDFILE->AddFile(szPath);
		WASABI_API_EXPLORERFINDFILE->ShowFiles();
	}
	return E_UNEXPECTED;
}

HRESULT Command_EditServiceExternal(ifc_omservice *service)
{
	if (NULL == service) 
		return E_INVALIDARG;
	
	WCHAR szPath[512] = {0};
	
	HRESULT hr = service->GetAddress(szPath, ARRAYSIZE(szPath));
	if (FAILED(hr)) return hr;

	if (L'\0' == szPath[0]) 
		return E_FAIL;

	Navigation *navigation;
	if (FAILED(Plugin_GetNavigation(&navigation))) 
		return E_UNEXPECTED;

	HWND hOwner = navigation->GetActiveView(NULL);
	navigation->Release();

	if (NULL == hOwner) 
		hOwner = Plugin_GetLibrary();

	HINSTANCE hInst = ShellExecute(hOwner, L"open", szPath, NULL, NULL, SW_SHOWNORMAL);
	hr = ((INT_PTR)hInst > 32) ? S_OK : E_FAIL;
	return hr;
}

HRESULT Command_DeleteItem(HNAVITEM hItem)
{
	Navigation *navigation;
	HRESULT hr = Plugin_GetNavigation(&navigation);
	if (SUCCEEDED(hr)) 
	{
		hr = navigation->DeleteItem(hItem);
		navigation->Release();
	}
	
	return hr;
}

HRESULT Command_DeleteAll()
{
	Navigation *navigation;
	HRESULT hr = Plugin_GetNavigation(&navigation);
	if (SUCCEEDED(hr)) 
	{
		hr = navigation->DeleteAll();
		navigation->Release();
	}
	
	return hr;
}

HRESULT Command_CreateService()
{
	Navigation *navigation;
	HRESULT hr = Plugin_GetNavigation(&navigation);
	if (FAILED(hr)) return hr;

	HNAVITEM hItem;
	hr = navigation->CreateUserService(&hItem);
	if (SUCCEEDED(hr))
	{
		ifc_omservice *service;
		hr= navigation->GetService(hItem, &service);
		if (SUCCEEDED(hr))
		{
			Command_EditService(service);
			service->Release();
		}
	}
	navigation->Release();
	return hr;
}

HRESULT Command_NewWindow(HNAVITEM hItem)
{
	Navigation *navigation;
	HRESULT hr = Plugin_GetNavigation(&navigation);
	if (SUCCEEDED(hr))
	{
		HWND hWindow;
		HRESULT hr = navigation->CreatePopup(hItem, &hWindow);
		if (SUCCEEDED(hr))
		{
			ShowWindow(hWindow, SW_SHOWNORMAL);
		}
		navigation->Release();
	}
	return hr;
}

HRESULT Command_OpenView(HNAVITEM hItem)
{
	Navigation *navigation;
	HRESULT hr = Plugin_GetNavigation(&navigation);
	if (SUCCEEDED(hr))
	{
		hr = navigation->SelectItem(hItem, NULL);
		navigation->Release();
	}

	return hr;
}

void Command_ImportFiles()
{	
	HWND hOwner = Plugin_GetDialogOwner();
	ImportService_FromFile(hOwner);
}

void Command_ImportUrl()
{
	HWND hOwner = Plugin_GetDialogOwner();
	ImportService_FromUrl(hOwner);
}

static void CALLBACK BrowserOptions_Callback(HWND hOptions, UINT type, ULONG_PTR user)
{
	HWND hLibrary = (HWND)user;
	switch(type)
	{
		case BOCALLBACK_INIT:
			{
				HWND hView = (HWND)SENDMLIPC(hLibrary, ML_IPC_GETCURRENTVIEW, 0);
				if (NULL != hView)
				{
					RECT viewRect, optionsRect;
					if (GetWindowRect(hView, &viewRect) && GetWindowRect(hOptions, &optionsRect))
					{
						INT x = viewRect.left + ((viewRect.right - viewRect.left) - (optionsRect.right - optionsRect.left))/2;
						INT y = viewRect.top + ((viewRect.bottom - viewRect.top) - (optionsRect.bottom - optionsRect.top))/2;
						SetWindowPos(hOptions, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
						SendMessage(hOptions, DM_REPOSITION, 0, 0L); 
					}
				}
			}
			break;
	}
}

HRESULT Command_ShowBrowserOptions()
{
	HWND hWinamp = Plugin_GetWinamp();
	if (NULL == hWinamp || NULL == OMBROWSERMNGR) 
		return E_UNEXPECTED;

	HRESULT hr = OMBROWSERMNGR->Initialize(NULL, hWinamp);
	if (SUCCEEDED(hr))
	{
		HWND hOwner = Plugin_GetDialogOwner();
		hr = OMBROWSERMNGR->ShowOptions(hOwner, BOSTYLE_NORMAL | BOSTYLE_SHOWDEBUG, 
										BrowserOptions_Callback, (ULONG_PTR)hOwner);
	}
	return hr;
}

BOOL CommandManager_Process(HNAVITEM hItem, ifc_omservice *service, UINT commandId)
{
	switch(commandId)
	{
		case ID_VIEW_OPEN:					Command_OpenView(hItem);	return TRUE;
		case ID_VIEW_NEWWINDOW:				Command_NewWindow(hItem);	return TRUE;
		case ID_SERVICE_NEW:				Command_CreateService();	return TRUE;
		case ID_SERVICE_DELETE:				Command_DeleteItem(hItem);	return TRUE;
		case ID_SERVICE_DELETEALL:			Command_DeleteAll();	return TRUE;
		case ID_SERVICE_EDIT:				Command_EditService(service);	return TRUE;
		case ID_SERVICE_LOCATE:				Command_LocateService(service);	return TRUE;
		case ID_SERVICE_EDITEXTERNAL:		Command_EditServiceExternal(service);	return TRUE;
		case ID_SERVICE_RELOAD:				Command_ReloadService(service);	return TRUE;
		case ID_SERVICE_RESETPERMISSIONS:	Command_ResetPermissions(service);	return TRUE;
		case ID_NAVIGATION_BACK:			Command_NavigateService(service, NAVIGATE_BACK, TRUE);	return TRUE;
		case ID_NAVIGATION_FORWARD:			Command_NavigateService(service, NAVIGATE_FORWARD, TRUE);	return TRUE;
		case ID_NAVIGATION_HOME:			Command_NavigateService(service, NAVIGATE_HOME, TRUE);	return TRUE;
		case ID_NAVIGATION_STOP:			Command_NavigateService(service, NAVIGATE_STOP, TRUE);	return TRUE;
		case ID_NAVIGATION_REFRESH:			Command_NavigateService(service, NAVIGATE_REFRESH, TRUE);	return TRUE;
		case ID_OMBROWSER_OPTIONS:			Command_ShowBrowserOptions();	return TRUE;
		case ID_SERVICE_IMPORT_FILE:		Command_ImportFiles();	return TRUE;
		case ID_SERVICE_IMPORT_URL:			Command_ImportUrl();	return TRUE;
	}
	return FALSE;
}