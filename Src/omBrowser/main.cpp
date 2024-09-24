#include "./main.h"
#include "./component.h"
#include "../nu/htmlcontainer2.h"
#include "./ifc_wasabihelper.h"
#include "./ifc_skinhelper.h"
#include "./ifc_skinnedbrowser.h"
#include "./pngLoader.h"
#include "./ifc_omservicehost.h"

#include <shlwapi.h>
#include <strsafe.h>

static HINSTANCE pluginInstance = NULL;
static OmBrowserComponent component;

LPWSTR Plugin_MallocString(size_t cchLen)
{
	return (LPWSTR)calloc(cchLen, sizeof(WCHAR));
}

void Plugin_FreeString(LPWSTR pszString)
{
	if (NULL != pszString)
	{
		free(pszString);
	}
}

LPWSTR Plugin_ReAllocString(LPWSTR pszString, size_t cchLen)
{
	return (LPWSTR)realloc(pszString, sizeof(WCHAR) * cchLen);
}

LPWSTR Plugin_CopyString(LPCWSTR pszSource)
{
	if (NULL == pszSource)
		return NULL;

	INT cchSource = lstrlenW(pszSource) + 1;

	LPWSTR copy = Plugin_MallocString(cchSource);
	if (NULL != copy)
	{
		CopyMemory(copy, pszSource, sizeof(WCHAR) * cchSource);
	}
	return copy;
}

LPSTR Plugin_MallocAnsiString(size_t cchLen)
{
	return (LPSTR)calloc(cchLen, sizeof(CHAR));
}

LPSTR Plugin_CopyAnsiString(LPCSTR pszSource)
{
	if (NULL == pszSource)
		return NULL;

	INT cchSource = lstrlenA(pszSource) + 1;

	LPSTR copy = Plugin_MallocAnsiString(cchSource);
	if (NULL != copy)
	{
		CopyMemory(copy, pszSource, sizeof(CHAR) * cchSource);
	}
	return copy;
}

void Plugin_FreeAnsiString(LPSTR pszString)
{
	Plugin_FreeString((LPWSTR)pszString);
}

LPWSTR Plugin_DuplicateResString(LPCWSTR pszResource)
{
	return (IS_INTRESOURCE(pszResource)) ? 
			(LPWSTR)pszResource : 
			Plugin_CopyString(pszResource);
}

void Plugin_FreeResString(LPWSTR pszResource)
{
	if (!IS_INTRESOURCE(pszResource))
		Plugin_FreeString(pszResource);
}

HRESULT Plugin_CopyResString(LPWSTR pszBuffer, INT cchBufferMax, LPCWSTR pszString)
{
	if (NULL == pszBuffer)
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	if (NULL == pszString)
	{
		pszBuffer[0] = L'\0';
	}
	else if (IS_INTRESOURCE(pszString))
	{
		Plugin_LoadString((INT)(INT_PTR)pszString, pszBuffer, cchBufferMax);
	}
	else
	{
		hr = StringCchCopy(pszBuffer, cchBufferMax, pszString);
	}
	return hr;
}

LPSTR Plugin_WideCharToMultiByte(UINT codePage, DWORD dwFlags, LPCWSTR lpWideCharStr, INT cchWideChar, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
	INT cchBuffer = WideCharToMultiByte(codePage, dwFlags, lpWideCharStr, cchWideChar, NULL, 0, lpDefaultChar, lpUsedDefaultChar);
	if (0 == cchBuffer) return NULL;
	
	LPSTR buffer = Plugin_MallocAnsiString(cchBuffer);
	if (NULL == buffer) return NULL; 
		
	if (0 == WideCharToMultiByte(codePage, dwFlags, lpWideCharStr, cchWideChar, buffer, cchBuffer, lpDefaultChar, lpUsedDefaultChar))
	{
		Plugin_FreeAnsiString(buffer);
		return NULL;
	}
	return buffer;
}

LPWSTR Plugin_MultiByteToWideChar(UINT codePage, DWORD dwFlags, LPCSTR lpMultiByteStr, INT cbMultiByte)
{
	if (NULL == lpMultiByteStr) return NULL;
	INT cchBuffer = MultiByteToWideChar(codePage, dwFlags, lpMultiByteStr, cbMultiByte, NULL, 0);
	if (NULL == cchBuffer) return NULL;

	if (cbMultiByte > 0) cchBuffer++;

	LPWSTR buffer = Plugin_MallocString(cchBuffer);
	if (NULL == buffer) return NULL;

	if (0 == MultiByteToWideChar(codePage, dwFlags, lpMultiByteStr, cbMultiByte, buffer, cchBuffer))
	{
		Plugin_FreeString(buffer);
		return NULL;
	}

	if (cbMultiByte > 0)
	{
		buffer[cchBuffer - 1] = L'\0';
	}
	return buffer;
}

HRESULT Plugin_Initialize(HWND hwndWinamp)
{
	return component.InitializeComponent(hwndWinamp);
}

HRESULT Plugin_GetWasabiHelper(ifc_wasabihelper **wasabiHelper)
{
	return component.GetWasabiHelper(wasabiHelper);
}

HRESULT Plugin_GetSkinHelper(ifc_skinhelper **skinHelper)
{
	return component.GetSkinHelper(skinHelper);
}

HRESULT Plugin_GetBrowserSkin(ifc_skinnedbrowser **skinnedBrowser)
{
	ifc_skinhelper *skin = NULL;
	HRESULT hr = Plugin_GetSkinHelper(&skin);
	if (FAILED(hr) || skin == NULL) return hr;

	hr = skin->QueryInterface(IFC_SkinnedBrowser, (void**)skinnedBrowser);
	skin->Release();
	return hr;
}

HRESULT Plugin_GetWinampWnd(HWND *hwndWinamp)
{
	return component.GetWinampWnd(hwndWinamp);
}


HRESULT Plugin_RegisterWinampHook(ifc_winamphook *hook, UINT *cookieOut)
{
	return component.RegisterWinampHook(hook, cookieOut);
}

HRESULT Plugin_UnregisterWinampHook(UINT cookie)
{
	return component.UnregisterWinampHook(cookie);
}


HINSTANCE Plugin_GetInstance()
{
	return pluginInstance;
}

HINSTANCE Plugin_GetLangInstance()
{
	ifc_wasabihelper *wasabi = NULL;
	if (FAILED(component.GetWasabiHelper(&wasabi)) || wasabi == NULL) return NULL;

	HINSTANCE langModule = NULL;
	if (FAILED(wasabi->GetLanguageModule(&langModule)))
		langModule = NULL;

	wasabi->Release();
	return langModule;
}

HRESULT Plugin_FormatUuidString(UUID &uid, LPWSTR pszBuffer, size_t cchBufferMax)
{
	if (NULL == pszBuffer || cchBufferMax < 1) 
		return E_INVALIDARG;

	pszBuffer[0] = L'\0';

	RPC_WSTR pszUid;
	if (RPC_S_OK != UuidToString(&uid, &pszUid))
	{
		DWORD errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}

	UINT cchLen = lstrlen((LPCWSTR)pszUid);
	UINT s = 0, d = 0; 

	for (; s < cchLen && d < cchBufferMax; s++)
	{
		if (pszUid[s] >= L'a' && pszUid[s] <= L'f')
		{
			pszBuffer[d++] = (0xDF & pszUid[s]);
		}
		else if (pszUid[s] >= L'0' && pszUid[s] <= L'9')
		{
			pszBuffer[d++] = pszUid[s];
		}
	}

	if (d > cchBufferMax) 
		d = 0;

	pszBuffer[d] = L'\0';
	RpcStringFree(&pszUid);

	return (L'\0' != pszBuffer[0]) ? S_OK : E_FAIL;
}

static HINSTANCE Plugin_GetLangHelper(api_language **langManager)
{
	ifc_wasabihelper *wasabi = NULL;
	if (FAILED(component.GetWasabiHelper(&wasabi)) || wasabi == NULL)
	{
		*langManager = NULL;
		return NULL;
	}

	if (FAILED(wasabi->GetLanguageManager(langManager)))
		*langManager = NULL;

	HINSTANCE langModule = NULL;
	if (FAILED(wasabi->GetLanguageModule(&langModule)))
		langModule = NULL;

	wasabi->Release();

	return langModule;
}

const wchar_t* Plugin_LoadString(UINT id, wchar_t *buffer, int bufferMax)
{
	const wchar_t* r = 0;
	api_language *lang = NULL;
	HINSTANCE langModule = Plugin_GetLangHelper(&lang);
	if (NULL != lang)
	{
		r = lang->GetStringW(langModule, Plugin_GetInstance(), id, buffer, bufferMax);
		lang->Release();
	}
	return r;
}

const char* Plugin_LoadStringAnsi(UINT id, char *buffer, int bufferMax)
{
	const char* r = 0;
	api_language *lang = NULL;
	HINSTANCE langModule = Plugin_GetLangHelper(&lang);
	if (NULL != lang)
	{
		r = lang->GetString(langModule, Plugin_GetInstance(), id, buffer, bufferMax);
		lang->Release();
	}
	return r;
}

HWND Plugin_CreateDialogParam(const wchar_t *templateName, HWND parent, DLGPROC proc, LPARAM param)
{
	HWND r = 0;
	api_language *lang = NULL;
	HINSTANCE langModule = Plugin_GetLangHelper(&lang);
	if (NULL != lang)
	{
		r = lang->CreateLDialogParamW(langModule, Plugin_GetInstance(), 	(INT)(INT_PTR)templateName, parent, proc, param);
		lang->Release();
	}
	return r;
}

INT_PTR Plugin_DialogBoxParam(const wchar_t *templateName, HWND parent, DLGPROC proc, LPARAM param)
{	
	INT_PTR r = -1;
	api_language *lang = NULL;
	HINSTANCE langModule = Plugin_GetLangHelper(&lang);
	if (NULL != lang)
	{
		r = lang->LDialogBoxParamW(langModule, Plugin_GetInstance(), (INT)(INT_PTR)templateName, parent, proc, param);
		lang->Release();
	}
	return r;
}

HMENU Plugin_LoadMenu(const wchar_t *menuName)
{
	HMENU r = NULL;
	api_language *lang = NULL;
	HINSTANCE langModule = Plugin_GetLangHelper(&lang);
	if (NULL != lang)
	{
		r = lang->LoadLMenuW(langModule, Plugin_GetInstance(), (INT)(INT_PTR)menuName);
		lang->Release();
	}
	return r;
}

void *Plugin_LoadResource(const wchar_t *resourceType, const wchar_t *resourceName, unsigned long *size)
{
	void *r = NULL;
	api_language *lang = NULL;
	HINSTANCE langModule = Plugin_GetLangHelper(&lang);
	if (NULL != lang)
	{
		r = lang->LoadResourceFromFileW(langModule, Plugin_GetInstance(), resourceType, resourceName, size);
		lang->Release();
	}
	return r;
}

HACCEL Plugin_LoadAccelerators(const wchar_t *tableName)
{
	HACCEL r = NULL;
	api_language *lang = NULL;
	HINSTANCE langModule = Plugin_GetLangHelper(&lang);
	if (NULL != lang)
	{
		r = lang->LoadAcceleratorsW(langModule, Plugin_GetInstance(), tableName); 
		lang->Release();
	}
	return r;
}

HRESULT Plugin_QueryImageLoader(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, ifc_omimageloader **imageLoader)
{
	if (NULL == imageLoader) return E_POINTER;
	*imageLoader = NULL;

	if (NULL == pszName) return E_INVALIDARG;

	return PngLoader::CreateInstance(hInstance, pszName, fPremultiply, imageLoader);
}

size_t Plugin_TlsAlloc(void)
{
	size_t index = TLS_OUT_OF_INDEXES;
	ifc_wasabihelper *wasabi = NULL;
	if (SUCCEEDED(component.GetWasabiHelper(&wasabi)) && wasabi != NULL)
	{
		api_application *app = NULL;
		if (SUCCEEDED(wasabi->GetApplicationApi(&app)) && app != NULL)
		{
			index = app->AllocateThreadStorage();
			app->Release();
		}
	}
	return index;
}

BOOL Plugin_TlsFree(size_t index)
{
	return FALSE;
}

void *Plugin_TlsGetValue(size_t index)
{
	void *result = NULL;
	ifc_wasabihelper *wasabi = NULL;
	if (SUCCEEDED(component.GetWasabiHelper(&wasabi)) && wasabi != NULL)
	{
		api_application *app = NULL;
		if (SUCCEEDED(wasabi->GetApplicationApi(&app)) && app != NULL)
		{
			result = app->GetThreadStorage(index);
			app->Release();
		}
	}
	return result;
}

BOOL Plugin_TlsSetValue(size_t index, void* value)
{
	BOOL result = FALSE;
	ifc_wasabihelper *wasabi = NULL;
	if (SUCCEEDED(component.GetWasabiHelper(&wasabi)) && wasabi != NULL)
	{
		api_application *app = NULL;
		if (SUCCEEDED(wasabi->GetApplicationApi(&app)) && app != NULL)
		{
			app->SetThreadStorage(index, value);
			app->Release();
			result = TRUE;
		}
	}
	return result;
}

void Plugin_RegisterUnloadCallback(PLUGINUNLOADCALLBACK callback)
{
	component.RegisterUnloadCallback(callback);
}

HRESULT Plugin_GetBrowserClass(LPCWSTR pszName, ifc_ombrowserclass **instance)
{
	return component.GetBrowserClass(pszName, instance);
}

HRESULT Plugin_UnregisterBrowserClass(LPCWSTR pszName)
{
	return component.UnregisterBrowserClass(pszName);
}

HRESULT Plugin_EnsurePathExist(LPCWSTR pszDirectory)
{
	DWORD ec = ERROR_SUCCESS;
	UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

	if (0 == CreateDirectory(pszDirectory, NULL))
	{
		ec = GetLastError();
		if (ERROR_PATH_NOT_FOUND == ec)
		{
			LPCWSTR pszBlock = pszDirectory;
			WCHAR szBuffer[MAX_PATH] = {0};

			LPCTSTR pszCursor = PathFindNextComponent(pszBlock);
			ec = (pszCursor == pszBlock || S_OK != StringCchCopyN(szBuffer, ARRAYSIZE(szBuffer), pszBlock, (pszCursor - pszBlock))) ?
					ERROR_INVALID_NAME : ERROR_SUCCESS;

			pszBlock = pszCursor;

			while (ERROR_SUCCESS == ec && NULL != (pszCursor = PathFindNextComponent(pszBlock)))
			{
				if (pszCursor == pszBlock || S_OK != StringCchCatN(szBuffer, ARRAYSIZE(szBuffer), pszBlock, (pszCursor - pszBlock)))
					ec = ERROR_INVALID_NAME;

				if (ERROR_SUCCESS == ec && !CreateDirectory(szBuffer, NULL))
				{
					ec = GetLastError();
					if (ERROR_ALREADY_EXISTS == ec) ec = ERROR_SUCCESS;
				}
				pszBlock = pszCursor;
			}
		}

		if (ERROR_ALREADY_EXISTS == ec) 
			ec = ERROR_SUCCESS;
	}

	SetErrorMode(errorMode);
	SetLastError(ec);
	return HRESULT_FROM_WIN32(ec);
}

HRESULT Plugin_MakeResourcePath(LPWSTR pszBuffer, UINT cchBufferMax, HINSTANCE hInstance, LPCWSTR pszType, LPCWSTR pszName, UINT uFlags)
{
	if (NULL == pszBuffer) return E_INVALIDARG;
	
	if (NULL == hInstance || NULL == pszName) 
	{
		pszBuffer[0] = L'\0';
		return E_INVALIDARG;
	}

	LPWSTR cursor = pszBuffer;
	size_t remaining = cchBufferMax;

	HRESULT hr = StringCchCopyEx(cursor, remaining, L"res://", &cursor, &remaining, 0);
	if (SUCCEEDED(hr))
	{
		DWORD cchPath = GetModuleFileName(hInstance, cursor, (DWORD)remaining);
		if (0 == cchPath)
		{
			DWORD errorCode = GetLastError();
			hr = HRESULT_FROM_WIN32(errorCode);
		}
		else
		{
			if (0 != (RESPATH_COMPACT & uFlags))
			{
				ifc_wasabihelper *wasabi = NULL;
				if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabi)) && wasabi != NULL)
				{
					api_application *application = NULL;
					if (SUCCEEDED(wasabi->GetApplicationApi(&application)) && application != NULL)
					{
						WCHAR szPlugin[MAX_PATH] = {0};
						HINSTANCE hWinamp = application->main_gethInstance();
						if (NULL != hWinamp && 0 != GetModuleFileName(hWinamp, szPlugin, ARRAYSIZE(szPlugin)))
						{
							PathRemoveFileSpec(szPlugin);
							INT cchCommon = PathCommonPrefix(cursor, szPlugin, NULL);
							// prevents messing things up if on same drive and that's all that matches
							if (cchCommon > 3)
							{
								cchCommon++;
								cchPath -= cchCommon;
								MoveMemory(cursor, cursor + cchCommon, cchPath * sizeof(WCHAR));
								cursor[cchPath] = L'\0';
							}
						}
						application->Release();
					}
					wasabi->Release();
				}
			}

			remaining -= cchPath;
			cursor += cchPath;
		}

		LPCWSTR pszTemplate = NULL;
		if (SUCCEEDED(hr) && NULL != pszType)
		{
			if (IS_INTRESOURCE(pszType))
			{
				pszTemplate = (0 != (RESPATH_TARGETIE & uFlags)) ? L"/%d" : L"/#%d";
				hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, 0, pszTemplate, pszType);
			}
			else if (L'\0' != *pszType)
			{
				hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, 0, L"/%s", pszType);
			}
		}

		if (SUCCEEDED(hr))
		{			
			if (IS_INTRESOURCE(pszName))
				pszTemplate = (0 != (RESPATH_TARGETIE & uFlags)) ? L"/%d" : L"/#%d";
			else
				pszTemplate = L"/%s";
			
			hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, 0, pszTemplate, pszName);
		}
	}

	if (FAILED(hr))
		pszBuffer[0] = L'\0';
	
	return hr;
}

HRESULT Plugin_ResolveRelativePath(LPCWSTR pszPath, ifc_omservicehost *host, LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	HRESULT hr = S_OK;

	if (NULL != pszPath && L'\0' == *pszPath) 
		pszPath = NULL;

	if (NULL != pszPath && FALSE == PathIsRelative(pszPath))
	{
		hr = StringCchCopy(pszBuffer, cchBufferMax, pszPath);
		return (FAILED(hr)) ? hr : S_FALSE;
	}

	WCHAR szBase[MAX_PATH] = {0};
	if (NULL == host || FAILED(host->GetBasePath(NULL, szBase, ARRAYSIZE(szBase))))
		szBase[0]= L'\0';

	if (L'\0' == szBase[0] || PathIsRelative(szBase))
	{
		ifc_wasabihelper *wasabi = NULL;
		if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabi)) && wasabi != NULL)
		{
			api_application *application = NULL;
			if (SUCCEEDED(wasabi->GetApplicationApi(&application)) && application != NULL)
			{
				LPCWSTR pszUser = application->path_getUserSettingsPath();
				if (NULL != pszUser && L'\0' != *pszUser)
				{
					if (L'\0' != szBase[0]) 
						hr = StringCchCopy(pszBuffer, cchBufferMax, szBase);
					
					if (SUCCEEDED(hr))
					{
						hr = StringCchCopy(szBase, ARRAYSIZE(szBase), pszUser);
						if (SUCCEEDED(hr) && L'\0' != *pszBuffer && FALSE == PathAppend(szBase, pszBuffer))
							hr = E_OUTOFMEMORY;
					}
				}
				application->Release();
			}
			wasabi->Release();
		}
	}

	if (SUCCEEDED(hr) && NULL != pszPath)
	{
		if (L'\0' == szBase[0])
			hr = StringCchCopy(szBase, ARRAYSIZE(szBase), pszPath);
		else if (FALSE == PathAppend(szBase, pszPath))
			hr = E_OUTOFMEMORY;
	}

	if (SUCCEEDED(hr) && 0 == PathCanonicalize(pszBuffer, szBase))
		hr = E_OUTOFMEMORY;

	return hr;
}

BOOL Plugin_IsDirectMouseWheelMessage(const UINT uMsg)
{
	static UINT WINAMP_WM_DIRECT_MOUSE_WHEEL = WM_NULL;

	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
	{
		WINAMP_WM_DIRECT_MOUSE_WHEEL = RegisterWindowMessageW(L"WINAMP_WM_DIRECT_MOUSE_WHEEL");
		if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
			return FALSE;
	}

	return (WINAMP_WM_DIRECT_MOUSE_WHEEL == uMsg);
}

extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  uReason, void *reserved)
{
    switch(uReason) 
	{
		case DLL_PROCESS_ATTACH:
			pluginInstance = (HINSTANCE)hModule;
			HTMLContainer2_Initialize();
			break;
		case DLL_PROCESS_DETACH:
			HTMLContainer2_Uninitialize();
			break;
    }
    return TRUE;
}