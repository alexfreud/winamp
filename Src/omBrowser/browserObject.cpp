#include "main.h"
#include "./browserObject.h"
#include "./ifc_menucustomizer.h"
#include "./ifc_wasabihelper.h"
#include "./ifc_omconfig.h"
#include "./browserView.h"
#include "./browserPopup.h"
#include "./options.h"
#include "./ifc_ombrowserconfig.h"
#include "./ifc_ombrowserregistry.h"
#include "./ifc_ombrowserevent.h"
#include "./ifc_omtoolbarconfig.h"
#include "./ifc_omstatusbarconfig.h"
#include "./ifc_ombrowserclass.h"
#include "./ifc_omservice.h"
#include "./ieversion.h"
#include "./browserWndEnum.h"
#include "./browserWndRecord.h"
#include <strsafe.h>

OmBrowserObject::OmBrowserObject() : ref(1), flags(0), browserClass(NULL)
{
	InitializeCriticalSection(&lock);
}

OmBrowserObject::~OmBrowserObject()
{
	if (NULL != browserClass)
		browserClass->Release();

	DeleteCriticalSection(&lock);
}	

HRESULT OmBrowserObject::CreateInstance(OmBrowserObject **instance)
{
	if (NULL == instance) return E_POINTER;

	*instance = new OmBrowserObject();
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t OmBrowserObject::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmBrowserObject::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

int OmBrowserObject::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, OBJ_OmBrowser))
		*object = static_cast<obj_ombrowser*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmBrowserWindowManager))
		*object = static_cast<ifc_ombrowserwndmngr*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmBrowserEventManager))
		*object = static_cast<ifc_ombrowsereventmngr*>(this);
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

HRESULT OmBrowserObject::Initialize(LPCWSTR pszName, HWND hwndWinamp)
{		
	if ( browserClass != NULL )
		return S_FALSE;

	HRESULT hr = Plugin_Initialize(hwndWinamp);
	if (SUCCEEDED(hr))
	{
		if (NULL == pszName || L'\0' == *pszName)
			pszName = OMBROWSER_NAME;

		hr = Plugin_GetBrowserClass(pszName, &browserClass);
	}
	return hr;
}

HRESULT OmBrowserObject::Finish(void)
{
	HRESULT hr = S_OK;

	EnterCriticalSection(&lock);

	flags |= flagFinishing;

	size_t index = windowList.size();
	while(index--)
	{
		OmBrowserWndRecord *r = windowList[index];
		if (FALSE == DestroyWindow(r->GetHwnd()))
		{
			hr = E_FAIL;
		}
		else
		{
			windowList.erase(windowList.begin() + index);
			r->Release();
		}
	}	

	while(!eventList.empty())
	{
		ifc_ombrowserevent *e = eventList.back();
		if (NULL == e) break;
		eventList.pop_back();
		e->Release();
	}

	if (NULL != browserClass)
	{
		browserClass->Release();
		browserClass = NULL;
	}
	
	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmBrowserObject::RegisterWinampHook(ifc_winamphook *hook, UINT *cookieOut)
{
	return Plugin_RegisterWinampHook(hook, cookieOut);
}

HRESULT OmBrowserObject::UnregisterWinampHook(UINT cookie)
{
	return Plugin_UnregisterWinampHook(cookie);
}

HRESULT OmBrowserObject::GetConfig(const GUID *configIfc, void **configOut)
{
	if (NULL == configOut) return E_POINTER;
	if (NULL == browserClass) return E_UNEXPECTED;

	ifc_omconfig *config = NULL;
	HRESULT hr = browserClass->GetConfig(&config);	

	if (SUCCEEDED(hr) && config != NULL)
	{
		if (NULL == configIfc || IsEqualIID(*configIfc, GUID_NULL))
			*configOut = config;
		else
		{
			hr = config->QueryInterface(*configIfc, configOut);
			config->Release();
		}
	}
	return hr;
}

HRESULT OmBrowserObject::GetSessionId(LPWSTR pszBuffer, INT cchBufferMax)
{	
	if (NULL == pszBuffer) return E_POINTER;
	*pszBuffer = L'\0';

	ifc_wasabihelper *wasabi = NULL;
	HRESULT hr = Plugin_GetWasabiHelper(&wasabi);
	if (SUCCEEDED(hr) && wasabi != NULL)
	{
		api_application *app = NULL;
		hr = wasabi->GetApplicationApi(&app);
		if (SUCCEEDED(hr))
		{
			UUID uid;
			if (API_APPLICATION_SUCCESS == app->GetSessionID(&uid))
			{
				hr = Plugin_FormatUuidString(uid, pszBuffer, cchBufferMax);
			}
			else
			{
				hr = E_FAIL;
			}
			app->Release();
		}
		wasabi->Release();
	}

	if (FAILED(hr))
		hr = StringCchCopy(pszBuffer, cchBufferMax, L"0123456789ABCDEF");

	return hr;
}

HRESULT OmBrowserObject::GetClientId(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	*pszBuffer = L'\0';

	ifc_ombrowserconfig *browserConfig;
	if (FAILED(GetConfig(&IFC_OmBrowserConfig, (void**)&browserConfig)))
		return E_NOINTERFACE;

	HRESULT hr = browserConfig->GetClientId(pszBuffer, cchBufferMax);
	if (S_OK != hr)
	{
		UUID uid;
		LPWSTR cursor = NULL;
		size_t remaining = 0;
		StringCchCopyEx(pszBuffer, cchBufferMax, L"WA-", &cursor, &remaining, 0);

		if (RPC_S_OK != UuidCreate(&uid) ||
			FAILED(Plugin_FormatUuidString(uid, cursor, remaining)) || 
			FAILED(browserConfig->SetClientId(pszBuffer)))
		{
			*pszBuffer = L'\0';
			hr = E_FAIL;
		}
	}

	browserConfig->Release();
	return hr;
}

HRESULT OmBrowserObject::GetRegistry(ifc_ombrowserregistry **registryOut)
{
	if (NULL == registryOut) return E_POINTER;
	if (NULL == browserClass) return E_UNEXPECTED;
	return browserClass->GetRegistry(registryOut);
}

HRESULT OmBrowserObject::CreateView(ifc_omservice *service, HWND hParent, LPCWSTR forceUrl, UINT viewStyle, HWND *hView)
{
	if (NULL == hView) return E_POINTER;
	*hView = NULL;

	if (NULL == hParent) return E_INVALIDARG;
	*hView = BrowserView_Create(this, service, hParent, forceUrl, viewStyle);
	if (NULL == *hView) return E_FAIL;

	BrowserView_UpdateSkin(*hView, FALSE);
	return S_OK;
}

HRESULT OmBrowserObject::CreatePopup(ifc_omservice *service, INT x, INT y, INT cx, INT cy, HWND hOwner, LPCWSTR forceUrl, UINT viewStyle, HWND *hWindow)
{
	if (NULL == hWindow) return E_POINTER;
	*hWindow = NULL;

	if (NULL == hOwner && FAILED(Plugin_GetWinampWnd(&hOwner)))
		return E_INVALIDARG;

	HWND hPopup = BrowserPopup_Create(this, service, viewStyle, x, y, cx, cy, hOwner, NULL, 0L);
	if (NULL == hPopup) return E_FAIL;

	*hWindow = hPopup;

	BrowserPopup_UpdateSkin(hPopup, FALSE);
	if (NULL != forceUrl)
	{
		BrowserPopup_Navigate(hPopup, forceUrl, 0L);
	}
	else
	{
		BrowserPopup_Navigate(hPopup, NAVIGATE_HOME, 0L);
	}
	return S_OK;
}

HRESULT OmBrowserObject::IsFinishing(void)
{
	return (0 != (flagFinishing & flags)) ? S_OK : S_FALSE;
}

HRESULT OmBrowserObject::GetClass(ifc_ombrowserclass **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = browserClass;
	if (NULL == *instance) return E_UNEXPECTED;
	browserClass->AddRef();

	return S_OK;
}

HRESULT OmBrowserObject::GetVersion(int *major, int *minor)
{
	if (NULL != major) *major = OMBROWSER_VERSION_MAJOR;
	if (NULL != minor) *minor = OMBROWSER_VERSION_MINOR;
	return S_OK;
}

HRESULT OmBrowserObject::GetIEVersion(int *major, int *minor, int *build, int *subbuild)
{
	return MSIE_GetVersion(major, minor, build, subbuild);
}

HRESULT OmBrowserObject::ShowOptions(HWND hOwner, UINT style, BROWSEROPTIONSCALLBACK callback, ULONG_PTR user)
{	
	return BrowserOptions_ShowDialog(this, hOwner, style, callback, user);
}

HRESULT OmBrowserObject::RegisterWindow(HWND hwnd, const GUID *windowType)
{
	if (NULL == hwnd) return E_INVALIDARG;

	if (S_FALSE != IsFinishing()) 
		return E_UNEXPECTED;

	EnterCriticalSection(&lock);

	size_t index = windowList.size();
	while(index--)
	{
		if (windowList[index]->GetHwnd() == hwnd)
		{
			LeaveCriticalSection(&lock);
			return S_FALSE;
		}
	}

	OmBrowserWndRecord *r = NULL;
	HRESULT hr = OmBrowserWndRecord::CreateInstance(hwnd, windowType, &r);
	if (SUCCEEDED(hr) && r != NULL)
	{
		windowList.push_back(r);
	}

	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT OmBrowserObject::UnregisterWindow(HWND hwnd)
{
	if (NULL == hwnd) return E_INVALIDARG;

	if (S_FALSE != IsFinishing()) 
		return E_PENDING;

	EnterCriticalSection(&lock);
	size_t index = windowList.size();
	while(index--)
	{
		OmBrowserWndRecord *r = windowList[index];

		if (r->GetHwnd() == hwnd)
		{
			windowList.erase(windowList.begin() + index);
			r->Release();

			LeaveCriticalSection(&lock);
			return S_OK;
		}
	}

	LeaveCriticalSection(&lock);
	return S_FALSE;
}

HRESULT OmBrowserObject::Enumerate(const GUID *windowType, UINT *serviceIdFilter, ifc_ombrowserwndenum **enumerator)
{
	EnterCriticalSection(&lock);

	HRESULT hr = OmBrowserWndEnumerator::CreateInstance(windowType, serviceIdFilter, windowList.size() ? &windowList.at(0) : nullptr,
														windowList.size(), (OmBrowserWndEnumerator**)enumerator);

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmBrowserObject::RegisterEventHandler(ifc_ombrowserevent *eventHandler)
{
	if (NULL == eventHandler)
		return E_INVALIDARG;

	if (S_FALSE != IsFinishing()) 
		return E_PENDING;

	EnterCriticalSection(&lock);

	size_t index = eventList.size();
	while (index--)
	{
		if (eventList[index] == eventHandler)
		{
			LeaveCriticalSection(&lock);
			return E_FAIL;
		}
	}

	eventList.push_back(eventHandler);
	eventHandler->AddRef();

	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmBrowserObject::UnregisterEventHandler(ifc_ombrowserevent *eventHandler)
{
	if (NULL == eventHandler)
		return E_INVALIDARG;

	if (S_FALSE != IsFinishing()) 
		return E_PENDING;

	EnterCriticalSection(&lock);

	size_t index = eventList.size();
	while (index--)
	{
		if (eventList[index] == eventHandler)
		{
			eventList.erase(eventList.begin() + index);
			eventHandler->Release();
			LeaveCriticalSection(&lock);
			return S_OK;
		}
	}

	LeaveCriticalSection(&lock);
	return S_FALSE;
}

HRESULT OmBrowserObject::Signal_WindowCreate(HWND hwnd, const GUID *windowType)
{
	EnterCriticalSection(&lock);

	size_t index = eventList.size();
	while (index--)
	{
		eventList[index]->WindowCreate(hwnd, windowType);
	}

	LeaveCriticalSection(&lock);
	return S_OK;
}

HRESULT OmBrowserObject::Signal_WindowClose(HWND hwnd, const GUID *windowType)
{
	EnterCriticalSection(&lock);

	size_t index = eventList.size();
	while (index--)
	{
		eventList[index]->WindowClose(hwnd, windowType);
	}

	LeaveCriticalSection(&lock);
	return S_OK;
}

#define CBCLASS OmBrowserObject
START_MULTIPATCH;
 START_PATCH(MPIID_OMBROWSER)
  M_CB(MPIID_OMBROWSER, obj_ombrowser, ADDREF, AddRef);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, RELEASE, Release);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_INITIALIZE, Initialize);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_FINISH, Finish);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_REGISTERWINAMPHOOK, RegisterWinampHook);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_UNREGISTERWINAMPHOOK, UnregisterWinampHook);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_GETCONFIG, GetConfig);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_GETSESSIONID, GetSessionId);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_GETCLIENTID, GetClientId);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_GETREGISTRY, GetRegistry);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_CREATEVIEW, CreateView);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_CREATEPOPUP, CreatePopup);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_ISFINISHING, IsFinishing);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_GETCLASS, GetClass);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_GETVERSION, GetVersion);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_GETIEVERSION, GetIEVersion);
  M_CB(MPIID_OMBROWSER, obj_ombrowser, API_SHOWOPTIONS, ShowOptions);
 NEXT_PATCH(MPIID_OMBROWSERWNDMNGR)
  M_CB(MPIID_OMBROWSERWNDMNGR, ifc_ombrowserwndmngr, ADDREF, AddRef);
  M_CB(MPIID_OMBROWSERWNDMNGR, ifc_ombrowserwndmngr, RELEASE, Release);
  M_CB(MPIID_OMBROWSERWNDMNGR, ifc_ombrowserwndmngr, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMBROWSERWNDMNGR, ifc_ombrowserwndmngr, API_REGISTERWINDOW, RegisterWindow);
  M_CB(MPIID_OMBROWSERWNDMNGR, ifc_ombrowserwndmngr, API_UNREGISTERWINDOW, UnregisterWindow);
  M_CB(MPIID_OMBROWSERWNDMNGR, ifc_ombrowserwndmngr, API_ENUMERATE, Enumerate);
 NEXT_PATCH(MPIID_OMBROWSEREVENTMNGR)
  M_CB(MPIID_OMBROWSEREVENTMNGR, ifc_ombrowsereventmngr, ADDREF, AddRef);
  M_CB(MPIID_OMBROWSEREVENTMNGR, ifc_ombrowsereventmngr, RELEASE, Release);
  M_CB(MPIID_OMBROWSEREVENTMNGR, ifc_ombrowsereventmngr, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMBROWSEREVENTMNGR, ifc_ombrowsereventmngr, API_REGISTERHANDLER, RegisterEventHandler);
  M_CB(MPIID_OMBROWSEREVENTMNGR, ifc_ombrowsereventmngr, API_UNREGISTERHANDLER, UnregisterEventHandler);
  M_CB(MPIID_OMBROWSEREVENTMNGR, ifc_ombrowsereventmngr, API_SIGNAL_WINDOWCREATE, Signal_WindowCreate);
  M_CB(MPIID_OMBROWSEREVENTMNGR, ifc_ombrowsereventmngr, API_SIGNAL_WINDOWCLOSE, Signal_WindowClose);
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS