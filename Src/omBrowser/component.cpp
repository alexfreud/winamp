#include "./main.h"
#include "./component.h"
#include "./browserFactory.h"
#include "./serviceFactory.h"
#include "./utilityFactory.h"
#include "./wasabiHelper.h"
#include "./winampHook.h"
#include "./skinHelper.h"
#include "./browserClass.h"
#include "./internetFeatures.h"
#include "./ieVersion.h"

#include <wininet.h>
#include <strsafe.h>

static OmBrowserFactory browserFactory;
static OmServiceFactory serviceFactory;
static OmUtilityFactory utilityFactory;

OmBrowserComponent::OmBrowserComponent()
	: wasabiHelper(NULL), winampHook(NULL), skinHelper(NULL), hookCookie(0), internetFeatures(NULL)
{
	InitializeCriticalSection(&lock);
}

OmBrowserComponent::~OmBrowserComponent()
{
	ReleaseServices();

	if (NULL != internetFeatures)
	{
		delete(internetFeatures);
		internetFeatures = NULL;

		InternetSetOption(NULL, INTERNET_OPTION_END_BROWSER_SESSION, NULL, 0);
	}

	DeleteCriticalSection(&lock);
}

size_t OmBrowserComponent::AddRef()
{
	return 1;
}

size_t OmBrowserComponent::Release()
{
	return 1;
}

int OmBrowserComponent::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_WinampHook))
		*object = static_cast<ifc_winamphook*>(this);
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

void OmBrowserComponent::RegisterServices(api_service *service)
{
	EnterCriticalSection(&lock);

	if (NULL == wasabiHelper)
		WasabiHelper::CreateInstance(service, &wasabiHelper);

	aTRACE_LINE("omBrowser Registered");
	browserFactory.Register(service);
	serviceFactory.Register(service);
	utilityFactory.Register(service);

	LeaveCriticalSection(&lock);
}

void OmBrowserComponent::ReleaseServices()
{
	EnterCriticalSection(&lock);

	if (NULL != wasabiHelper)
	{
		wasabiHelper->Release();
		wasabiHelper = NULL;
	}

	if (NULL != winampHook)
	{
		winampHook->Release();
		winampHook = NULL;
	}

	if (0 != hookCookie) 
	{
		UnregisterWinampHook(hookCookie);
		hookCookie = 0;
	}

	if (NULL != skinHelper)
	{
		skinHelper->Release();
		skinHelper = NULL;
	}

	LeaveCriticalSection(&lock);
}

int OmBrowserComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void OmBrowserComponent::DeregisterServices(api_service *service)
{
	browserFactory.Unregister(service);
	serviceFactory.Unregister(service);
	utilityFactory.Unregister(service);

	ReleaseServices();

	size_t index = unloadCallbacks.size();
	while(index--)
	{
		PLUGINUNLOADCALLBACK callback = unloadCallbacks[index];
		if (NULL != callback) callback();
	}
	unloadCallbacks.clear();
}

HRESULT OmBrowserComponent::InitializeComponent(HWND hwndWinamp)
{
	HRESULT hr(S_FALSE);

	EnterCriticalSection(&lock);

	if(NULL == winampHook)
	{
		HRESULT hookResult = WinampHook::CreateInstance(hwndWinamp, &winampHook);
		if (FAILED(hookResult) && SUCCEEDED(hr)) hr = hookResult;
	}

	if (SUCCEEDED(hr) && 0 == hookCookie)
	{
		hr = winampHook->RegisterCallback(this, &hookCookie);
	}

	if (NULL == skinHelper)
	{
		HRESULT skinResult = SkinHelper::CreateInstance(hwndWinamp, &skinHelper);
		if (FAILED(skinResult) && SUCCEEDED(hr)) hr = skinResult;
	}

	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT OmBrowserComponent::GetWasabiHelper( ifc_wasabihelper **wasabiOut )
{
	if ( NULL == wasabiOut ) return E_POINTER;

	EnterCriticalSection( &lock );

	*wasabiOut = wasabiHelper;
	if ( NULL != wasabiHelper )
		wasabiHelper->AddRef();

	LeaveCriticalSection( &lock );

	return ( NULL != *wasabiOut ) ? S_OK : E_NOINTERFACE;
}

HRESULT OmBrowserComponent::GetSkinHelper(ifc_skinhelper **skinOut)
{
	if (NULL == skinOut) return E_POINTER;

	EnterCriticalSection(&lock);

	*skinOut = skinHelper;
	if (NULL != skinHelper)
		skinHelper->AddRef();

	LeaveCriticalSection(&lock);

	return (NULL != *skinOut) ? S_OK : E_NOINTERFACE;
}

HRESULT OmBrowserComponent::RegisterWinampHook(ifc_winamphook *hook, UINT *cookieOut)
{
	if (NULL == cookieOut) return E_POINTER;
	*cookieOut = NULL;

	EnterCriticalSection(&lock);
	HRESULT hr = (NULL != winampHook) ? winampHook->RegisterCallback(hook, cookieOut) : E_FAIL;
	LeaveCriticalSection(&lock);

    return hr;
}

HRESULT OmBrowserComponent::UnregisterWinampHook(UINT cookie)
{
	EnterCriticalSection(&lock);
	HRESULT hr = (NULL != winampHook) ? winampHook->UnregisterCallback(cookie) : E_FAIL;
	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT OmBrowserComponent::GetWinampWnd(HWND *hwndWinamp)
{
	if (NULL == hwndWinamp) return E_POINTER;

	EnterCriticalSection(&lock);
	*hwndWinamp = (NULL != winampHook) ? winampHook->GetWinamp() : NULL;
	LeaveCriticalSection(&lock);

	HRESULT hr;
	if (NULL == *hwndWinamp) 
		hr = E_FAIL;
	else
		hr = S_OK;

	return hr;
}

HRESULT OmBrowserComponent::ResetFont(void)
{
	if (NULL != skinHelper)
		skinHelper->ResetFontCache();
	return S_OK;
}

HRESULT OmBrowserComponent::SkinChanged(const wchar_t *skinName)
{
	UpdateColors();
	return S_OK;
}

HRESULT OmBrowserComponent::SkinColorChange(const wchar_t *colorTheme)
{
	UpdateColors();
	return S_OK;
}

HRESULT OmBrowserComponent::RegisterUnloadCallback(PLUGINUNLOADCALLBACK callback)
{
	if(NULL == callback) return E_INVALIDARG;
	unloadCallbacks.push_back(callback);
	return S_OK;
}

HRESULT OmBrowserComponent::GetBrowserClass(LPCWSTR pszName, ifc_ombrowserclass **instance)
{
	if (NULL == instance) return E_POINTER;

	HRESULT hr(S_OK);
	ifc_ombrowserclass *browserClass = NULL;

	EnterCriticalSection(&lock);

	size_t index = browserClasses.size();
	while(index--)
	{
		browserClass = browserClasses[index];
		if (S_OK == browserClass->IsEqual(pszName))
		{
			browserClass->AddRef();
			break;
		}
	}

	if (((size_t)-1) == index)
	{
		hr = OmBrowserClass::CreateInstance(pszName, (OmBrowserClass**)&browserClass);
		if (SUCCEEDED(hr) && browserClass != NULL)
		{
			if (0 == browserClasses.size())
			{
				SetUserAgent();
				SetInternetFeautures();
			}

			browserClasses.push_back(browserClass);
		}
	}

	*instance = (SUCCEEDED(hr) && browserClass != NULL) ? browserClass : NULL;	

	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT OmBrowserComponent::UnregisterBrowserClass(LPCWSTR pszName)
{
	HRESULT hr(S_FALSE);

	EnterCriticalSection(&lock);

	size_t index = browserClasses.size();
	while(index--)
	{
		ifc_ombrowserclass *browserClass = browserClasses[index];
		if (S_OK == browserClass->IsEqual(pszName))
		{
			browserClasses.erase(browserClasses.begin() + index);
			hr = S_OK;
			break;
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

void OmBrowserComponent::UpdateColors()
{
	if (NULL != skinHelper)
		skinHelper->ResetColorCache();

	EnterCriticalSection(&lock);

	size_t index = browserClasses.size();
	while(index--)
	{
		browserClasses[index]->UpdateRegColors();
	}

	LeaveCriticalSection(&lock);

	InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0 );
}

typedef struct __INTERNETFEATUREREC
{
	INTERNETFEATURELIST entry;
	DWORD				flags;
	BOOL				enabled;
} INTERNETFEATUREREC;

void OmBrowserComponent::SetInternetFeautures()
{
	const INTERNETFEATUREREC szFeatures[] = 
	{  
		{ FEATURE_DISABLE_NAVIGATION_SOUNDS, SET_FEATURE_ON_PROCESS, TRUE },
		{ FEATURE_TABBED_BROWSING, SET_FEATURE_ON_PROCESS, TRUE },
		{ FEATURE_WINDOW_RESTRICTIONS, SET_FEATURE_ON_PROCESS, FALSE },
		{ FEATURE_SSLUX, SET_FEATURE_ON_PROCESS, TRUE },
		{ FEATURE_FORCE_ADDR_AND_STATUS, SET_FEATURE_ON_PROCESS, FALSE },
		{ FEATURE_BLOCK_INPUT_PROMPTS, SET_FEATURE_ON_PROCESS, FALSE },
		{ FEATURE_MIME_HANDLING, SET_FEATURE_ON_PROCESS, TRUE },
		{ FEATURE_LOCALMACHINE_LOCKDOWN, SET_FEATURE_ON_PROCESS, TRUE },
	};

	EnterCriticalSection(&lock);

	if (NULL == internetFeatures)
	{
		internetFeatures = new InternetFeatures();
		if (NULL == internetFeatures) 
		{
			LeaveCriticalSection(&lock);
			return;
		}
	}

	UINT modified = 0;
	HRESULT hr;

	for (INT i = 0; i < ARRAYSIZE(szFeatures); i++)
	{
		const INTERNETFEATUREREC *rec = &szFeatures[i];
		hr = internetFeatures->IsEnabled(rec->entry, rec->flags);
		if ((S_OK == hr && FALSE == rec->enabled) || (S_FALSE == hr && FALSE != rec->enabled))
		{
			if (SUCCEEDED(internetFeatures->SetEnabled(rec->entry, rec->flags, rec->enabled)))
				modified++;
		}
	}

	int majorVersion = 0;
	if (SUCCEEDED(MSIE_GetVersion(&majorVersion, NULL, NULL, NULL)))
	{
		unsigned long browserEmulation = 0;
		if (8 == majorVersion)
			browserEmulation = 8000;
		else if (9 == majorVersion)
			browserEmulation = 9000;
		else if (10 <= majorVersion)
			browserEmulation = 10000;
		else
			browserEmulation = 0;

		unsigned long valueAlreadySet = 0;
		hr = internetFeatures->GetDWORDFeature(L"FEATURE_BROWSER_EMULATION", TRUE, &valueAlreadySet);
		if (FAILED(hr) || valueAlreadySet != browserEmulation)
		{
			if (0 == browserEmulation)
			{
				if (0x80070002 /*ERROR_FILE_NOT_FOUND */ != hr)
					internetFeatures->DeleteFeature(L"FEATURE_BROWSER_EMULATION", TRUE);
			}
			else
			{
				if (SUCCEEDED(internetFeatures->SetDWORDFeature(L"FEATURE_BROWSER_EMULATION", TRUE, browserEmulation)))
					modified++;
			}
		}
	}

	LeaveCriticalSection(&lock);

	if (0 != modified)
		InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0 );
}

static HRESULT
Component_PrintWinampUA(char *buffer, size_t bufferMax)
{
	char *cursor = buffer;
	size_t remaining = bufferMax;

	HRESULT hr = StringCchPrintfExA(cursor, remaining, 
						&cursor, &remaining, 
						STRSAFE_NULL_ON_FAILURE, 
						"%S/%d.%d", 
						OMBROWSER_NAME, 
						OMBROWSER_VERSION_MAJOR, 
						OMBROWSER_VERSION_MINOR);

	if (SUCCEEDED(hr))
	{
		ifc_wasabihelper *wasabi = NULL;
		if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabi)) && wasabi != NULL)
		{
			api_application *app = NULL;
			if (SUCCEEDED(wasabi->GetApplicationApi(&app)) && app != NULL)
			{
				char *rollback = cursor;
				hr = StringCchPrintfExA(cursor, remaining, 
						&cursor, &remaining, 
						STRSAFE_NULL_ON_FAILURE, 
						" (%S)", 
						app->main_getVersionString());
				
				if (FAILED(hr))
					*rollback = '\0';

				app->Release();
			}
			wasabi->Release();
		}
	}

	if (FAILED(hr))
		buffer[0] = '\0';

	return hr;
}

BOOL OmBrowserComponent::SetUserAgent(void)
{
	unsigned long bufferSize = 0;

	HRESULT hr = UrlMkGetSessionOption(URLMON_OPTION_USERAGENT, 
								NULL, 
								0,
								&bufferSize,
								0);
	if(E_OUTOFMEMORY == hr)
		hr = S_OK;

	if (FAILED(hr))
		return FALSE;

	bufferSize += 512;

	char *buffer = (char *)calloc(bufferSize, sizeof(char));
	if (NULL == buffer)
		return FALSE;

	unsigned long bufferLength = 0;
	hr = UrlMkGetSessionOption(URLMON_OPTION_USERAGENT, 
								buffer, 
								bufferSize,
								&bufferLength,
								0);
	if (E_OUTOFMEMORY == hr
		&& bufferLength <= bufferSize)
	{
		hr = S_OK;
	}

	if (SUCCEEDED(hr))
	{
		if (bufferLength > 0 
			&& bufferLength < bufferSize)
		{
			buffer[bufferLength - 1] = ' ';
		}
		hr = Component_PrintWinampUA(buffer + bufferLength, 
									 bufferSize - bufferLength);
		if (FAILED(hr))
		{
			if (bufferLength > 0)
				buffer[bufferLength - 1] = '\0';
		}
		else
		{
			bufferLength = lstrlenA(buffer);

			hr = UrlMkSetSessionOption(URLMON_OPTION_USERAGENT, 
										buffer, 
										bufferLength,
										0);
		}
	}

	free(buffer);
	return SUCCEEDED(hr);
}

#define CBCLASS OmBrowserComponent
START_MULTIPATCH;
 START_PATCH(MPIID_WA5COMPONENT)
  M_CB(MPIID_WA5COMPONENT, ifc_wa5component, ADDREF, AddRef);
  M_CB(MPIID_WA5COMPONENT, ifc_wa5component, RELEASE, Release);
  M_CB(MPIID_WA5COMPONENT, ifc_wa5component, QUERYINTERFACE, QueryInterface);
  M_VCB(MPIID_WA5COMPONENT, ifc_wa5component, API_WA5COMPONENT_REGISTERSERVICES, RegisterServices);
  M_CB(MPIID_WA5COMPONENT, ifc_wa5component, 15, RegisterServicesSafeModeOk)
  M_VCB(MPIID_WA5COMPONENT, ifc_wa5component, API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices);
 NEXT_PATCH(MPIID_WINAMPHOOK)
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, ADDREF, AddRef);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, RELEASE, Release);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, API_RESETFONT, ResetFont);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, API_SKINCHANGED, SkinChanged);
  M_CB(MPIID_WINAMPHOOK, ifc_winamphook, API_SKINCOLORCHANGE, SkinColorChange);
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS