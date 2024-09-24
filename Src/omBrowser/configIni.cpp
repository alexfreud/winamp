#include "./main.h"
#include "./configIni.h"
#include "./ifc_wasabihelper.h"
#include "./ifc_omconfigcallback.h"
#include <api/application/api_application.h>

#include <shlwapi.h>
#include <strsafe.h>

#define BROWSER_SECTION				L"OmBrowser"
#define BROWSER_CLIENTID			L"clientId"
#define BROWSER_XPOS				L"x"
#define BROWSER_YPOS				L"y"

#define DEBUG_SECTION				L"Debug"
#define DEBUG_BROWSERPATH			L"browserPath"
#define DEBUG_FILTERCONTEXTMENU		L"filterContextMenu"
#define DEBUG_SHOWSCRIPTDEBUGGER	L"showScriptDebugger"
#define DEBUG_SHOWSCRIPTERROR		L"showScriptErrors"

#define TOOLBAR_SECTION				L"Toolbar"
#define TOOLBAR_BOTTOMDOCK			L"bottomDock"
#define TOOLBAR_AUTOHIDE			L"autoHide"
#define TOOLBAR_TABSTOP				L"tabStop"
#define TOOLBAR_FORCEADDRESSBAR		L"addressbarForce"
#define TOOLBAR_FANCYADDRESSBAR		L"addressbarFancy"

#define STATUSBAR_SECTION			L"Statusbar"
#define STATUSBAR_ENABLED			L"enabled"

#define BOOL2HRESULT(__result) ((FALSE != (__result)) ? S_OK : S_FALSE)

OmConfigIni::OmConfigIni(LPCWSTR pszPath)
	: ref(1), configPath(NULL), lastCookie(0),
	  pathValidated(FALSE)
{
	InitializeCriticalSection(&lock);
	configPath = Plugin_CopyString(pszPath);
}

OmConfigIni::~OmConfigIni()
{
	EnterCriticalSection(&lock);

	for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
	{
		ifc_omconfigcallback *callback = iter->second;
		if (NULL != callback) callback->Release();
	}

	LeaveCriticalSection(&lock);

	Plugin_FreeString(configPath);
	DeleteCriticalSection(&lock);
}

static HRESULT OmConfigIni_MakeFileName(LPWSTR pszBuffer, INT cchBufferMax, LPCWSTR pszName)
{
	if (NULL == pszBuffer) 
		return E_POINTER;

	if (NULL == pszName || L'\0' == *pszName || 
		FAILED(StringCchCopy(pszBuffer, cchBufferMax, pszName)))
	{
		return E_INVALIDARG;
	}

	PathRemoveBlanks(pszBuffer);
	INT cchBuffer = lstrlen(pszBuffer);

	if (0 == cchBuffer) 
		return E_INVALIDARG;

	if (FAILED(StringCchCopy(pszBuffer + cchBuffer, cchBufferMax - cchBuffer, L".ini")))
		return E_FAIL;

	return S_OK;
}

HRESULT OmConfigIni::CreateInstance(LPCWSTR pszName, OmConfigIni **instanceOut)
{
	if (NULL == instanceOut) return E_POINTER;
	*instanceOut = NULL;

	WCHAR szFile[MAX_PATH] = {0};
	HRESULT hr = OmConfigIni_MakeFileName(szFile, ARRAYSIZE(szFile), pszName);
	if (FAILED(hr)) return hr;
	
	ifc_wasabihelper *wasabi = NULL;
	hr = Plugin_GetWasabiHelper(&wasabi);
	if (SUCCEEDED(hr) && wasabi != NULL)
	{
		api_application *app = NULL;
		hr = wasabi->GetApplicationApi(&app);
		if (SUCCEEDED(hr) && app != NULL)
		{
			WCHAR szBuffer[1024] = {0};
			LPCWSTR userPath = app->path_getUserSettingsPath();
			if (NULL == userPath || L'\0' == *userPath ||
				NULL == PathCombine(szBuffer, userPath, L"Plugins\\omBrowser") ||
				FALSE == PathAppend(szBuffer, szFile))
			{
				hr = E_UNEXPECTED;
			}
			else
			{
				*instanceOut = new OmConfigIni(szBuffer);
				if (NULL == *instanceOut) hr = E_OUTOFMEMORY;
			}

			app->Release();
		}
		wasabi->Release();
	}
	return hr;
}

size_t OmConfigIni::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmConfigIni::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OmConfigIni::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmConfig))
		*object = static_cast<ifc_omconfig*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmBrowserConfig))
		*object = static_cast<ifc_ombrowserconfig*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmDebugConfig))
		*object = static_cast<ifc_omdebugconfig*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmToolbarConfig))
		*object = static_cast<ifc_omtoolbarconfig*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmStatusbarConfig))
		*object = static_cast<ifc_omstatusbarconfig*>(this);
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

HRESULT OmConfigIni::GetPath(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	return StringCchCopy(pszBuffer, cchBufferMax, configPath);
}

DWORD OmConfigIni::ReadStr(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize)
{
	return GetPrivateProfileStringW(lpSectionName, lpKeyName, lpDefault, lpReturnedString, nSize, configPath);
}

UINT OmConfigIni::ReadInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, INT nDefault)
{
	return GetPrivateProfileIntW(lpSectionName, lpKeyName, nDefault, configPath);
}

BOOL OmConfigIni::ReadBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bDefault)
{
	WCHAR szBuffer[32] = {0};
	INT cchLen = ReadStr(lpSectionName, lpKeyName, NULL, szBuffer, ARRAYSIZE(szBuffer));
	if (0 == cchLen) return bDefault;

	if (1 == cchLen)
	{
		switch(*szBuffer)
		{
			case L'0':
			case L'n':
			case L'f':
				return FALSE;
			case L'1':
			case L'y':
			case L't':
				return TRUE;
		}
	}
	else
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, L"yes", -1, szBuffer, cchLen) ||
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, L"true", -1, szBuffer, cchLen))
		{
			return TRUE;
		}

		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, L"no", -1, szBuffer, cchLen) ||
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, L"false", -1, szBuffer, cchLen))
		{
			return FALSE;
		}
	}

	INT v = 0;
	if (FALSE != StrToIntEx(szBuffer, STIF_SUPPORT_HEX,  &v))
		return (0 != v);

	return bDefault;

}

HRESULT OmConfigIni::WriteStr(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpString)
{
	if (NULL == configPath || L'\0' == *configPath) 
		return E_UNEXPECTED;

	if (FALSE == pathValidated)
	{
		WCHAR szDirectory[MAX_PATH*2] = {0};
		if (SUCCEEDED(StringCchCopy(szDirectory, ARRAYSIZE(szDirectory), configPath)))
		{
			PathRemoveFileSpec(szDirectory);
			Plugin_EnsurePathExist(szDirectory);
			pathValidated = TRUE;
		}
	}

	if (0 != WritePrivateProfileStringW(lpSectionName, lpKeyName, lpString, configPath))
		return S_OK;

	DWORD errorCode = GetLastError();
	return HRESULT_FROM_WIN32(errorCode);
}

HRESULT OmConfigIni::WriteInt(LPCWSTR lpSectionName, LPCWSTR lpKeyName, INT nValue)
{
	wchar_t szBuffer[32] = {0};
	HRESULT hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), L"%d", nValue);
	if (FAILED(hr)) return hr;

	return WriteStr(lpSectionName, lpKeyName, szBuffer);
}

HRESULT OmConfigIni::WriteBool(LPCWSTR lpSectionName, LPCWSTR lpKeyName, BOOL bValue)
{
	return WriteStr(lpSectionName, lpKeyName, (0 != bValue) ? L"yes" : L"no");
}

HRESULT OmConfigIni::GetClientId(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	INT cchLen = ReadStr(BROWSER_SECTION, BROWSER_CLIENTID, NULL, pszBuffer, cchBufferMax);
	if (0 == cchLen) return S_FALSE;

	INT cchPrefix = lstrlen(L"WA-");
	if (cchLen <= cchPrefix ||
		CSTR_EQUAL != CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszBuffer, cchPrefix, L"WA-", cchPrefix))
	{
		pszBuffer[0] = L'\0';
		return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT OmConfigIni::SetClientId(LPWSTR pszClientId)
{
	EnterCriticalSection(&lock);

	WCHAR szBuffer[128] = {0};
	HRESULT hr = GetClientId(szBuffer, ARRAYSIZE(szBuffer));
	if (FAILED(hr) || CSTR_EQUAL != CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszClientId, -1, szBuffer, -1))
	{
		hr = WriteStr(BROWSER_SECTION, BROWSER_CLIENTID, pszClientId);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmBrowserConfig, CFGID_BROWSER_CLIENTID, (ULONG_PTR)pszClientId);
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmConfigIni::GetMenuFilterEnabled(void)
{
	BOOL result = ReadBool(DEBUG_SECTION, DEBUG_FILTERCONTEXTMENU, TRUE);
	return BOOL2HRESULT(result);
}

HRESULT OmConfigIni::GetScriptErrorEnabled(void)
{
	BOOL result = ReadBool(DEBUG_SECTION, DEBUG_SHOWSCRIPTERROR, FALSE);
	return BOOL2HRESULT(result);
}

HRESULT OmConfigIni::GetScriptDebuggerEnabled(void)
{
	BOOL result = ReadBool(DEBUG_SECTION, DEBUG_SHOWSCRIPTDEBUGGER, FALSE);
	return BOOL2HRESULT(result);
}

HRESULT OmConfigIni::GetBrowserPath(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;

	INT cchLen = ReadStr(DEBUG_SECTION, DEBUG_BROWSERPATH, NULL, pszBuffer, cchBufferMax);
	if (0 == cchLen) return S_FALSE;
	
	if ((L'.' == pszBuffer[0] && L'\\' == pszBuffer[1]) ||
		(L'.' == pszBuffer[0] && L'.' == pszBuffer[1] && L'\\' == pszBuffer[2]))
	{
		WCHAR szTemp[2*MAX_PATH] = {0};
		StringCchCopy(szTemp, ARRAYSIZE(szTemp), configPath);
		PathRemoveFileSpec(szTemp);

		if (FALSE == PathAppend(szTemp, pszBuffer) || 
			FAILED(StringCchCopy(pszBuffer, cchBufferMax, szTemp)))
		{
			pszBuffer[0] = L'\0';
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT OmConfigIni::EnableMenuFilter(BOOL fEnable)
{
	EnterCriticalSection(&lock);

	HRESULT hr = GetMenuFilterEnabled();
	if (FAILED(hr) || ((S_OK == hr) != (FALSE != fEnable)))
	{
		hr = WriteBool(DEBUG_SECTION, DEBUG_FILTERCONTEXTMENU, fEnable);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmDebugConfig, CFGID_DEBUG_FILTERMENU, (ULONG_PTR)fEnable);
		}
	}

	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT OmConfigIni::EnableScriptError(BOOL fEnable)
{
	HRESULT hr;

	EnterCriticalSection(&lock);

	hr = GetScriptErrorEnabled();
	if (FAILED(hr) || ((S_OK == hr) != (FALSE != fEnable)))
	{
		hr = WriteBool(DEBUG_SECTION, DEBUG_SHOWSCRIPTERROR, fEnable);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmDebugConfig, CFGID_DEBUG_SCRIPTERROR, (ULONG_PTR)fEnable);
		}
	}

	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT OmConfigIni::EnableScriptDebugger(BOOL fEnable)
{
	EnterCriticalSection(&lock);

	HRESULT hr = GetScriptDebuggerEnabled();
	if (FAILED(hr) || ((S_OK == hr) != (FALSE != fEnable)))
	{
		hr = WriteBool(DEBUG_SECTION, DEBUG_SHOWSCRIPTDEBUGGER, fEnable);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmDebugConfig, CFGID_DEBUG_SCRIPTDEBUGGER, (ULONG_PTR)fEnable);
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmConfigIni::SetBrowserPath(LPCWSTR pszPath)
{
	EnterCriticalSection(&lock);

	WCHAR szBuffer[MAX_PATH * 2] = {0};
	HRESULT hr = GetBrowserPath(szBuffer, ARRAYSIZE(szBuffer));
	if (FAILED(hr) || 
		CSTR_EQUAL != CompareString(CSTR_INVARIANT, 0, szBuffer, -1, pszPath, -1))
	{
		hr = WriteString(DEBUG_SECTION, DEBUG_SHOWSCRIPTDEBUGGER, pszPath);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmDebugConfig, CFGID_DEBUG_BROWSERPATH, (ULONG_PTR)pszPath);
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmConfigIni::Toolbar_GetBottomDockEnabled(void)
{
	BOOL result = ReadBool(TOOLBAR_SECTION, TOOLBAR_BOTTOMDOCK, FALSE);
	return BOOL2HRESULT(result);
}

HRESULT OmConfigIni::Toolbar_GetAutoHideEnabled(void)
{
	BOOL result = ReadBool(TOOLBAR_SECTION, TOOLBAR_AUTOHIDE, FALSE);
	return BOOL2HRESULT(result);
}

HRESULT OmConfigIni::Toolbar_GetTabStopEnabled(void)
{
	BOOL result = ReadBool(TOOLBAR_SECTION, TOOLBAR_TABSTOP, FALSE);
	return BOOL2HRESULT(result);
}

HRESULT OmConfigIni::Toolbar_GetForceAddressbarEnabled(void)
{
	BOOL result = ReadBool(TOOLBAR_SECTION, TOOLBAR_FORCEADDRESSBAR, FALSE);
	return BOOL2HRESULT(result);
}

HRESULT OmConfigIni::Toolbar_GetFancyAddressbarEnabled(void)
{
	BOOL result = ReadBool(TOOLBAR_SECTION, TOOLBAR_FANCYADDRESSBAR, TRUE);
	return BOOL2HRESULT(result);
}

HRESULT OmConfigIni::Toolbar_EnableBottomDock(BOOL fEnable)
{
	EnterCriticalSection(&lock);

	HRESULT hr = Toolbar_GetBottomDockEnabled();
	if (FAILED(hr) || ((S_OK == hr) != (FALSE != fEnable)))
	{
		hr = WriteBool(TOOLBAR_SECTION, TOOLBAR_BOTTOMDOCK, fEnable);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmToolbarConfig, CFGID_TOOLBAR_BOTTOMDOCK, (ULONG_PTR)fEnable);
		}
	}

	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT OmConfigIni::Toolbar_EnableAutoHide(BOOL fEnable)
{
	EnterCriticalSection(&lock);

	HRESULT hr = Toolbar_GetAutoHideEnabled();
	if (FAILED(hr) || ((S_OK == hr) != (FALSE != fEnable)))
	{
		hr = WriteBool(TOOLBAR_SECTION, TOOLBAR_AUTOHIDE, fEnable);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmToolbarConfig, CFGID_TOOLBAR_AUTOHIDE, (ULONG_PTR)fEnable);
		}
	}

	LeaveCriticalSection(&lock);
	
	return hr;
}

HRESULT OmConfigIni::Toolbar_EnableTabStop(BOOL fEnable)
{
	EnterCriticalSection(&lock);

	HRESULT hr = Toolbar_GetTabStopEnabled();
	if (FAILED(hr) || ((S_OK == hr) != (FALSE != fEnable)))
	{
		hr = WriteBool(TOOLBAR_SECTION, TOOLBAR_TABSTOP, fEnable);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmToolbarConfig, CFGID_TOOLBAR_TABSTOP, (ULONG_PTR)fEnable);
		}
	}

	LeaveCriticalSection(&lock);
	
	return hr;
}

HRESULT OmConfigIni::Toolbar_EnableForceAddressbar(BOOL fEnable)
{
	EnterCriticalSection(&lock);

	HRESULT hr = Toolbar_GetForceAddressbarEnabled();
	if (FAILED(hr) || ((S_OK == hr) != (FALSE != fEnable)))
	{
		hr = WriteBool(TOOLBAR_SECTION, TOOLBAR_FORCEADDRESSBAR, fEnable);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmToolbarConfig, CFGID_TOOLBAR_FORCEADDRESS, (ULONG_PTR)fEnable);
		}
	}

	LeaveCriticalSection(&lock);
	
	return hr;
}

HRESULT OmConfigIni::Toolbar_EnableFancyAddressbar(BOOL fEnable)
{
	EnterCriticalSection(&lock);

	HRESULT hr = Toolbar_GetFancyAddressbarEnabled();
	if (FAILED(hr) || ((S_OK == hr) != (FALSE != fEnable)))
	{
		hr = WriteBool(TOOLBAR_SECTION, TOOLBAR_FANCYADDRESSBAR, fEnable);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmToolbarConfig, CFGID_TOOLBAR_FANCYADDRESS, (ULONG_PTR)fEnable);
		}
	}

	LeaveCriticalSection(&lock);
	
	return hr;
}

HRESULT OmConfigIni::Statusbar_GetEnabled(void)
{
	BOOL result = ReadBool(STATUSBAR_SECTION, STATUSBAR_ENABLED, FALSE);
	return BOOL2HRESULT(result);
}

HRESULT OmConfigIni::Statusbar_EnableStatusbar(BOOL fEnable)
{
	EnterCriticalSection(&lock);

	HRESULT hr = Statusbar_GetEnabled();
	if (FAILED(hr) || ((S_OK == hr) != (FALSE != fEnable)))
	{
		hr = WriteBool(STATUSBAR_SECTION, STATUSBAR_ENABLED, fEnable);
		if (SUCCEEDED(hr))
		{
			NotifyChange(&IFC_OmStatusbarConfig, CFGID_STATUSBAR_ENABLED, (ULONG_PTR)fEnable);
		}
	}

	LeaveCriticalSection(&lock);
	
	return hr;
}

HRESULT OmConfigIni::RegisterCallback(ifc_omconfigcallback *callback, UINT *cookie)
{
	if (NULL == cookie) return E_POINTER;
	*cookie = 0;

	if (NULL == callback) 
		return E_INVALIDARG;

	EnterCriticalSection(&lock);
	
	*cookie = ++lastCookie;

	callbackMap.insert({ *cookie, callback });
	callback->AddRef();

	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmConfigIni::UnregisterCallback(UINT cookie)
{
	if (0 == cookie) return E_INVALIDARG;

	ifc_omconfigcallback *callback = NULL;
	EnterCriticalSection(&lock);

	for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
	{
		if (cookie == iter->first)
		{
			callback = iter->second;
			callbackMap.erase(iter);
			break;
		}
	}

	LeaveCriticalSection(&lock);

	if (NULL != callback)
	{
		callback->Release();
		return S_OK;
	}

	return S_FALSE;
}

void OmConfigIni::NotifyChange(const GUID *configUid, UINT valueId, ULONG_PTR value)
{
	EnterCriticalSection(&lock);

	for(CallbackMap::iterator iter = callbackMap.begin(); iter != callbackMap.end(); iter++)
	{
		ifc_omconfigcallback *callback = iter->second;
		callback->ValueChanged(configUid, valueId, value);
	}

	LeaveCriticalSection(&lock);
}

UINT OmConfigIni::GetX(void)
{
	return ReadInt(BROWSER_SECTION, BROWSER_XPOS, -1);
}

UINT OmConfigIni::GetY(void)
{
	return ReadInt(BROWSER_SECTION, BROWSER_YPOS, -1);
}

HRESULT OmConfigIni::SetX(UINT x)
{
	return WriteInt(BROWSER_SECTION, BROWSER_XPOS, x);
}

HRESULT OmConfigIni::SetY(UINT y)
{
	return WriteInt(BROWSER_SECTION, BROWSER_YPOS, y);
}

#define CBCLASS OmConfigIni
START_MULTIPATCH
 START_PATCH(MPIID_OMCONFIG)
  M_CB(MPIID_OMCONFIG, ifc_omconfig, ADDREF, AddRef);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, RELEASE, Release);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, API_GETPATH, GetPath);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, API_READSTRING, ReadStr);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, API_READINT, ReadInt);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, API_READBOOL, ReadBool);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, API_WRITESTRING, WriteStr);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, API_WRITEINT, WriteInt);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, API_WRITEBOOL, WriteBool);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, API_REGISTERCALLBACK, RegisterCallback);
  M_CB(MPIID_OMCONFIG, ifc_omconfig, API_UNREGISTERCALLBACK, UnregisterCallback);
 NEXT_PATCH(MPIID_OMBROWSERCONFIG)
  M_CB(MPIID_OMBROWSERCONFIG, ifc_ombrowserconfig, ADDREF, AddRef);
  M_CB(MPIID_OMBROWSERCONFIG, ifc_ombrowserconfig, RELEASE, Release);
  M_CB(MPIID_OMBROWSERCONFIG, ifc_ombrowserconfig, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMBROWSERCONFIG, ifc_ombrowserconfig, API_GETCLIENTID, GetClientId);
  M_CB(MPIID_OMBROWSERCONFIG, ifc_ombrowserconfig, API_SETCLIENTID, SetClientId);
  M_CB(MPIID_OMBROWSERCONFIG, ifc_ombrowserconfig, API_GETX, GetX);
  M_CB(MPIID_OMBROWSERCONFIG, ifc_ombrowserconfig, API_SETX, SetX);
  M_CB(MPIID_OMBROWSERCONFIG, ifc_ombrowserconfig, API_GETY, GetY);
  M_CB(MPIID_OMBROWSERCONFIG, ifc_ombrowserconfig, API_SETY, SetY);
 NEXT_PATCH(MPIID_OMDEBUGCONFIG)
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, ADDREF, AddRef);
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, RELEASE, Release);
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, API_GETMENUFILTERENABLED, GetMenuFilterEnabled);
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, API_GETSCRIPTERRORENABLED, GetScriptErrorEnabled);
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, API_GETSCRIPTDEBUGGERENABLED, GetScriptDebuggerEnabled);
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, API_GETBROWSERPATH, GetBrowserPath);
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, API_ENABLEMENUFILTER, EnableMenuFilter);
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, API_ENABLESCRIPTERROR, EnableScriptError);
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, API_ENABLESCRIPTDEBUGGER, EnableScriptDebugger);
  M_CB(MPIID_OMDEBUGCONFIG, ifc_omdebugconfig, API_SETBROWSERPATH, SetBrowserPath);
 NEXT_PATCH(MPIID_OMTOOLBARCONFIG)
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, ADDREF, AddRef);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, RELEASE, Release);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, API_GETBOTTOMDOCKENABLED, Toolbar_GetBottomDockEnabled);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, API_ENABLEBOTTOMDOCK, Toolbar_EnableBottomDock);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, API_GETAUTOHIDEENABLED, Toolbar_GetAutoHideEnabled);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, API_ENABLEAUTOHIDE, Toolbar_EnableAutoHide);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, API_GETTABSTOPENABLED, Toolbar_GetTabStopEnabled);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, API_ENABLETABSTOP, Toolbar_EnableTabStop);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, API_GETFORCEADDRESSBARENABLED, Toolbar_GetForceAddressbarEnabled);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, API_ENABLEFORCEADDRESSBAR, Toolbar_EnableForceAddressbar);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, API_GETFANCYADDRESSBARENABLED, Toolbar_GetFancyAddressbarEnabled);
  M_CB(MPIID_OMTOOLBARCONFIG, ifc_omtoolbarconfig, API_ENABLEFANCYADDRESSBAR, Toolbar_EnableFancyAddressbar);
 NEXT_PATCH(MPIID_OMSTATUSBARCONFIG)
  M_CB(MPIID_OMSTATUSBARCONFIG, ifc_omstatusbarconfig, ADDREF, AddRef);
  M_CB(MPIID_OMSTATUSBARCONFIG, ifc_omstatusbarconfig, RELEASE, Release);
  M_CB(MPIID_OMSTATUSBARCONFIG, ifc_omstatusbarconfig, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSTATUSBARCONFIG, ifc_omstatusbarconfig, API_GETENABLED, Statusbar_GetEnabled);
  M_CB(MPIID_OMSTATUSBARCONFIG, ifc_omstatusbarconfig, API_ENABLESTATUSBAR, Statusbar_EnableStatusbar);
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS