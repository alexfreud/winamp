#include "main.h"
#include "./updateService.h"
#include "./api.h"
#include "./externalCOM.h"

#include "../omBrowser/obj_ombrowser.h"
#include "../omBrowser/ifc_omservice.h"
#include "../omBrowser/browserPopup.h"
#include "../omBrowser/ifc_omdebugconfig.h"

HRESULT ServiceSubclass_Attach(HWND hwnd, UpdateService *service);

UpdateService::UpdateService(obj_ombrowser *browserObject, LPWSTR pszUrl)
: ref(1), url(pszUrl), browserManager(browserObject)
{
	if (NULL != browserManager)
		browserManager->AddRef();
}

UpdateService::~UpdateService()
{
	Finish();
}

HRESULT UpdateService::CreateInstance(LPCSTR pszUrl, UpdateService **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = NULL;

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(OBJ_OmBrowser);
	if (NULL == sf) return E_FAIL;
		
	obj_ombrowser *browserManager = reinterpret_cast<obj_ombrowser*>(sf->getInterface());
	sf->Release();

	if (NULL == browserManager)
		return E_NOINTERFACE;

	HRESULT hr = browserManager->Initialize(NULL, hMainWindow);
	if (SUCCEEDED(hr))
	{
		LPWSTR url = NULL;
		if (NULL != pszUrl)
		{
			UINT cchUrlMax = MultiByteToWideChar(CP_UTF8, 0, pszUrl, -1, NULL, 0);
			if (0 != cchUrlMax)
			{	
				cchUrlMax++;
				url = (LPWSTR)calloc(cchUrlMax, sizeof(WCHAR));
				if (NULL == url) hr = E_OUTOFMEMORY;
				else
				{
					if (0 == MultiByteToWideChar(CP_UTF8, 0, pszUrl, -1, url, cchUrlMax))
						hr = E_FAIL;
				}
			}
		}
		
		if (SUCCEEDED(hr))
		{
			*instance = new UpdateService(browserManager, url);
			if (NULL == *instance) 
			{
				if (NULL != url) free(url);
				hr = E_OUTOFMEMORY;
			}
		}
	}

	browserManager->Release();
	return hr;
}

size_t UpdateService::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t UpdateService::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int UpdateService::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmService))
		*object = static_cast<ifc_omservice*>(this);
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

unsigned int UpdateService::GetId()
{
	return 505;
}

HRESULT UpdateService::GetName(wchar_t *pszBuffer, int cchBufferMax)
{
	getStringW(IDS_WINAMP_UPDATE, pszBuffer, cchBufferMax);
	return S_OK;
}

HRESULT UpdateService::GetUrl(wchar_t *pszBuffer, int cchBufferMax)
{

	return StringCchCopyExW(pszBuffer, cchBufferMax, url, NULL, NULL, STRSAFE_IGNORE_NULLS );
}

HRESULT UpdateService::GetIcon(wchar_t *pszBuffer, int cchBufferMax)
{
	return E_NOTIMPL;
}

HRESULT UpdateService::GetExternal(IDispatch **ppDispatch)
{
	if (NULL == ppDispatch) 
		return E_POINTER;

	ExternalCOM *external;
	HRESULT hr = JSAPI1_GetExternal(&external);
	if (FAILED(hr)) external = NULL;
	
	*ppDispatch = (IDispatch*)external;
	return S_OK;
}

HRESULT UpdateService::Show()
{
	if (NULL == browserManager)
		return E_UNEXPECTED;

	LONG cx = 300;
	LONG cy = 200;

	RECT centerRect;
	HWND hCenter = (NULL != g_dialog_box_parent) ? g_dialog_box_parent : hMainWindow;

	if (NULL == hCenter || 
		0 == GetWindowRect(hCenter, &centerRect) ||
		((centerRect.right - centerRect.left) < cx) || 
		((centerRect.bottom - centerRect.top) < cy))
	{
		HMONITOR hMonitor = MonitorFromWindow(hCenter, MONITOR_DEFAULTTONEAREST);
		MONITORINFO info;
		info.cbSize = sizeof(info);
		
		if (NULL != hMonitor && 0 != GetMonitorInfo(hMonitor, &info))
			CopyRect(&centerRect, &info.rcWork);
		else
			SetRectEmpty(&centerRect);

	}

	LONG x = centerRect.left + (centerRect.right - centerRect.left - cx)/2;
	LONG y = centerRect.top + (centerRect.bottom - centerRect.top - cy)/2;
	if (x < centerRect.left) x = centerRect.left;
	if (y < centerRect.top) y = centerRect.top;
	
	UINT popupFlags = NBCS_NOTOOLBAR | NBCS_NOSTATUSBAR | NBCS_DIALOGMODE | NBCS_BLOCKPOPUP;

	ifc_omdebugconfig *debugConfig;
	if (SUCCEEDED(browserManager->GetConfig(&IFC_OmDebugConfig, (void**)&debugConfig)))
	{
		if (S_OK == debugConfig->GetMenuFilterEnabled())
			popupFlags |= NBCS_DISABLECONTEXTMENU;
		debugConfig->Release();
	}
	
	HWND hPopup;
	HRESULT hr =  browserManager->CreatePopup(this, x, y, cx, cy, hMainWindow, NULL, popupFlags, &hPopup);
	if (SUCCEEDED(hr))
	{		
		if (FAILED(ServiceSubclass_Attach(hPopup, this)))
		{
			ShowWindowAsync(hPopup, SW_SHOWNORMAL);
		}
	}

	return hr;
}

HRESULT UpdateService::Finish()
{
	if (NULL != browserManager)
	{
		browserManager->Finish();
		browserManager->Release();
		browserManager = NULL;
	}

	if (NULL != url) 
	{
		free(url);
		url = NULL;
	}

	return S_OK;
}

#define CBCLASS UpdateService
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETID, GetId)
CB(API_GETNAME, GetName)
CB(API_GETURL, GetUrl)
CB(API_GETICON, GetIcon)
CB(API_GETEXTERNAL, GetExternal)
END_DISPATCH;
#undef CBCLASS

