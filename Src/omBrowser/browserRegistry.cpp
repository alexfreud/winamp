#include "main.h"
#include "./browserRegistry.h"
#include "./obj_ombrowser.h"
#include "./ifc_wasabihelper.h"
#include "./ifc_skinhelper.h"
#include "./ifc_skinnedbrowser.h"
#include "./ifc_ombrowserclass.h"
#include "./ifc_omconfig.h"
#include "./ifc_omdebugconfig.h"

#include "../Plugins/General/gen_ml/colors.h"

#include <shlwapi.h>
#include <strsafe.h>

#define YES L"yes"
#define NO	L"no"

#define REGVAL_DWORD(__regKey, __valueName, __data) { DWORD __iVal = (__data); RegSetValueExW((__regKey), (__valueName), NULL, REG_DWORD, (LPCBYTE)&__iVal, sizeof(DWORD)); }
#define REGVAL_STR(__regKey, __valueName, __data, __cbData) RegSetValueExW((__regKey), (__valueName), NULL, REG_SZ, (LPCBYTE)(__data), (__cbData))

#define REGVAL_ONE(__regKey, __valueName) REGVAL_DWORD(__regKey, __valueName, 1)
#define REGVAL_ZERO(__regKey, __valueName) REGVAL_DWORD(__regKey, __valueName, 0)
#define REGVAL_YES(__regKey, __valueName) REGVAL_STR(__regKey, __valueName, YES, sizeof(YES))
#define REGVAL_NO(__regKey, __valueName) REGVAL_STR(__regKey, __valueName, NO, sizeof(NO))
#define REGVAL_YESNO(__regKey, __valueName, __condition) ((0 != (__condition)) ? REGVAL_YES(__regKey, __valueName) : REGVAL_NO(__regKey, __valueName))
#define REGVAL_RGB(__regKey, __valueName, __rgb)\
					{ WCHAR szRGB[64] = {0}; COLORREF c = (__rgb);\
						StringCchPrintfW(szRGB, ARRAYSIZE(szRGB), L"%d,%d,%d", GetRValue(c), GetGValue(c), GetBValue(c));\
						INT cbLen = lstrlenW(szRGB) * sizeof(WCHAR);\
						REGVAL_STR(__regKey, __valueName, szRGB, cbLen);}


OmBrowserRegistry::OmBrowserRegistry(LPCWSTR pszName) 
	: ref(1), name(NULL), path(NULL)
{
	name = Plugin_CopyString(pszName);
}

OmBrowserRegistry::~OmBrowserRegistry()
{
	Plugin_FreeString(name);
	Plugin_FreeString(path);
}

HRESULT OmBrowserRegistry::CreateInstance(LPCWSTR pszName, OmBrowserRegistry **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	if (NULL == pszName || L'\0' == *pszName) 
		pszName = OMBROWSER_NAME;

	*instance = new OmBrowserRegistry(pszName);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t OmBrowserRegistry::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmBrowserRegistry::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OmBrowserRegistry::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmBrowserRegistry))
		*object = static_cast<ifc_ombrowserregistry*>(this);
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

HRESULT	OmBrowserRegistry::Write()
{
	HKEY hKey = NULL;
    HKEY hKey2 = NULL;
 
	LONG result;
	DWORD disposition;
	
	if (FAILED(CreateRoot(&hKey, NULL)))
		return E_FAIL;
			
	result = RegCreateKeyExW(hKey, L"Main", NULL, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &hKey2, &disposition);
	if (ERROR_SUCCESS == result)
	{
		REGVAL_ONE(hKey2, L"AllowWindowReuse");
		
		
		HRESULT hr(S_FALSE);
		
		
		ifc_ombrowserclass *browserClass;
		if (SUCCEEDED(Plugin_GetBrowserClass(name, &browserClass)))
		{
			ifc_omconfig *config;
			if (SUCCEEDED(browserClass->GetConfig(&config)))
			{
				ifc_omdebugconfig *debugConfig;
				if (SUCCEEDED(config->QueryInterface(IFC_OmDebugConfig, (void**)&debugConfig)))
				{
					hr = debugConfig->GetScriptDebuggerEnabled();
					debugConfig->Release();
				}
				config->Release();
			}
			browserClass->Release();
		}
						
		REGVAL_YESNO(hKey2, L"Disable Script Debugger", (S_FALSE == hr));
		
		
		REGVAL_NO(hKey2, L"Check_Associations");
		REGVAL_ZERO(hKey2, L"EnableSearchPane");
		REGVAL_NO(hKey2, L"Friendly http errors");
		REGVAL_NO(hKey2, L"FullScreen");
		REGVAL_NO(hKey2, L"Save_Session_History_On_Exit");
		REGVAL_NO(hKey2, L"Use_DlgBox_Colors");
		REGVAL_YES(hKey2, L"ShowedCheckBrowser");
		REGVAL_STR(hKey2, L"Start Page", L"", sizeof(L""));
		REGVAL_STR(hKey2, L"Default_Page_URL", L"about:blank", sizeof(L"about:blank"));
		REGVAL_STR(hKey2, L"Default_Search_URL", L"about:blank", sizeof(L"about:blank"));
		REGVAL_NO(hKey2, L"Enable Browser Extensions");
		REGVAL_NO(hKey2, L"Error Dlg Details Pane Open");
		REGVAL_NO(hKey2, L"Error Dlg Displayed On Every Error");
		REGVAL_ONE(hKey2, L"NoUpdateCheck");
		REGVAL_NO(hKey2, L"Save_Session_History_On_Exit");
		REGVAL_ONE(hKey2, L"NoJITSetup");
		REGVAL_ONE(hKey2, L"NoWebJITSetup");
		REGVAL_ONE(hKey2, L"RunOnceComplete");
		REGVAL_ONE(hKey2, L"RunOnceHasShown");
		REGVAL_ONE(hKey2, L"SearchMigrated");
		
		RegCloseKey(hKey2);
	}

	result = RegCreateKeyExW(hKey, L"MenuExt", NULL, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &hKey2, &disposition);
	if (ERROR_SUCCESS == result)
	{
		RegCloseKey(hKey2);
	}

	result = RegCreateKeyExW(hKey, L"New Windows", NULL, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &hKey2, &disposition);
	if (ERROR_SUCCESS == result)
	{
		REGVAL_ZERO(hKey2, L"PlaySound");
		REGVAL_ONE(hKey2, L"PopupMgr");
		RegCloseKey(hKey2);
	}

	result = RegCreateKeyExW(hKey, L"SearchScopes", NULL, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &hKey2, &disposition);
	if (ERROR_SUCCESS == result)
	{
		RegCloseKey(hKey2);
	}

	result = RegCreateKeyExW(hKey, L"SearchUrl", NULL, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &hKey2, &disposition);
	if (ERROR_SUCCESS == result)
	{
		RegCloseKey(hKey2);
	}

	result = RegCreateKeyExW(hKey, L"URLSearchHooks", NULL, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &hKey2, &disposition);
	if (ERROR_SUCCESS == result)
	{
		RegCloseKey(hKey2);
	}

	HRESULT hr = WriteColors(hKey);
	
	RegCloseKey(hKey);
	return hr;
}

HRESULT OmBrowserRegistry::Delete()
{	
	HRESULT hr;
	WCHAR szBuffer[512] = {0};
	hr = GetPath(szBuffer, ARRAYSIZE(szBuffer));
	if (FAILED(hr)) return hr;

	LONG result = SHDeleteKey(HKEY_CURRENT_USER, szBuffer);
	hr = (ERROR_SUCCESS != result) ? E_FAIL : S_OK;
	return hr;
}

HRESULT OmBrowserRegistry::UpdateColors()
{
	HKEY hKey;
	
	HRESULT hr = CreateRoot(&hKey, NULL);
    if (FAILED(hr)) return hr;
	
	hr = WriteColors(hKey);

	RegCloseKey(hKey);
	return hr;
}

HRESULT OmBrowserRegistry::GetPath(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) 
		return E_POINTER;

	if (NULL == path)
	{		
		HRESULT hr = CreatePath(pszBuffer, cchBufferMax);
		if (FAILED(hr)) return hr;
	
		path = Plugin_CopyString(pszBuffer);
	}

	return StringCchPrintfW(pszBuffer, cchBufferMax, path);
}

HRESULT OmBrowserRegistry::CreatePath(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	if (NULL == name || L'\0' == *name) return E_INVALIDARG;
	
	HRESULT hr;
	WCHAR szApp[128] = {0}, szSessionId[128] = {0};

	ifc_wasabihelper *wasabi;
	hr = Plugin_GetWasabiHelper(&wasabi);
	if (SUCCEEDED(hr))
	{
		api_application *app;
		hr = wasabi->GetApplicationApi(&app);
		if (SUCCEEDED(hr))
		{
			if (FAILED(StringCchCopyEx(szApp, ARRAYSIZE(szApp), app->main_getAppName(), NULL, NULL, STRSAFE_IGNORE_NULLS)))
				szApp[0] = L'\0';
			
			UUID uid;
			if (API_APPLICATION_SUCCESS == app->GetSessionID(&uid))
			{
				RPC_WSTR pszUid;
				if (RPC_S_OK == UuidToString(&uid, &pszUid))
				{
					if(FAILED(StringCchCopy(szSessionId, ARRAYSIZE(szSessionId), (LPCWSTR)pszUid)))
						szSessionId[0] = L'\0';
					RpcStringFree(&pszUid);
				}
			}

			app->Release();
		}
		wasabi->Release();
	}

	if (L'\0' == szApp[0])
		hr = StringCchCopy(szApp, ARRAYSIZE(szApp), L"Winamp");

	if (L'\0' == szSessionId[0])
		hr = StringCchPrintf(szSessionId, ARRAYSIZE(szSessionId), L"%u", GetCurrentProcessId());
	
	if (SUCCEEDED(hr))
		hr = StringCchPrintfW(pszBuffer, cchBufferMax, L"Software\\%s\\%s\\%s", szApp, name, szSessionId);

	if (FAILED(hr))
		*pszBuffer = L'\0';

	return hr;
}
HRESULT OmBrowserRegistry::CreateRoot(HKEY *hKey, DWORD *pDisposition)
{
	if (NULL == path)
	{
		WCHAR szBuffer[2048] = {0};
		HRESULT hr = CreatePath(szBuffer, ARRAYSIZE(szBuffer));
		if (FAILED(hr)) return hr;
	
		path = Plugin_CopyString(szBuffer);
		if (NULL == path) return E_OUTOFMEMORY;
	}

	if (NULL == path || L'\0' == *path)
		return E_UNEXPECTED;

	DWORD disposition;
	LONG result = RegCreateKeyExW(HKEY_CURRENT_USER, path, NULL, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, hKey, &disposition);
	if (ERROR_SUCCESS != result)
	{
		return E_FAIL;
	}

	if (NULL != pDisposition)
		*pDisposition = disposition;

	return S_OK;
}

HRESULT OmBrowserRegistry::WriteColors(HKEY hKey)
{
	return S_OK;

	ifc_skinnedbrowser *skinnedBrowser;
	HRESULT hr = Plugin_GetBrowserSkin(&skinnedBrowser);
	if (FAILED(hr)) return E_FAIL;
			
	LONG result;
	DWORD disposition;
	HKEY hKey2 = NULL;
	
	result = RegCreateKeyExW(hKey, L"Settings", NULL, NULL, REG_OPTION_VOLATILE, KEY_SET_VALUE, NULL, &hKey2, &disposition);
	if (ERROR_SUCCESS == result)
	{		
		REGVAL_RGB(hKey2, L"Text Color", skinnedBrowser->GetTextColor());
		REGVAL_RGB(hKey2, L"Background Color", skinnedBrowser->GetBackColor());
		REGVAL_RGB(hKey2, L"Anchor Color", skinnedBrowser->GetLinkColor());
		REGVAL_RGB(hKey2, L"Anchor Color Hover", skinnedBrowser->GetHoveredLinkColor());
		REGVAL_YES(hKey2, L"Use Anchor Hover Color");
		REGVAL_RGB(hKey2, L"Anchor Color Visited", skinnedBrowser->GetVisitedLinkColor());

		RegCloseKey(hKey2);
	}
		
	skinnedBrowser->Release();
	return hr;
}

#define CBCLASS OmBrowserRegistry
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETPATH, GetPath)
CB(API_WRITE, Write)
CB(API_DELETE, Delete)
CB(API_UPDATECOLORS, UpdateColors)
END_DISPATCH;
#undef CBCLASS