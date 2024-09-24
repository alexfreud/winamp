#ifndef NULLSOFT_WINAMP_OMBROWSER_WA5SERVICE_MAIN_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_WA5SERVICE_MAIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define OMBROWSER_VERSION_MAJOR			1
#define OMBROWSER_VERSION_MINOR			5
#define OMBROWSER_NAME					L"omBrowser"

#include "./common.h"

/* string managment */
LPWSTR Plugin_MallocString(size_t cchLen);
LPWSTR Plugin_ReAllocString(LPWSTR pszString, size_t cchLen);
void Plugin_FreeString(LPWSTR pszString);
LPWSTR Plugin_CopyString(LPCWSTR pszSource);
LPSTR Plugin_MallocAnsiString(size_t cchLen);
LPSTR Plugin_CopyAnsiString(LPCSTR pszSource);
void Plugin_FreeAnsiString(LPSTR pszString);
LPWSTR Plugin_DuplicateResString(LPCWSTR pszResource);
void Plugin_FreeResString(LPWSTR pszResource);
HRESULT Plugin_CopyResString(LPWSTR pszBuffer, INT cchBufferMax, LPCWSTR pszString);

LPSTR Plugin_WideCharToMultiByte(UINT codePage, DWORD dwFlags, LPCWSTR lpWideCharStr, INT cchWideChar, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
LPWSTR Plugin_MultiByteToWideChar(UINT codePage, DWORD dwFlags, LPCSTR lpMultiByteStr, INT cbMultiByte);


HRESULT Plugin_FormatUuidString(UUID &uid, LPWSTR pszBuffer, size_t cchBufferMax);


HINSTANCE Plugin_GetInstance(void);
HINSTANCE Plugin_GetLangInstance(void);

HRESULT Plugin_Initialize(HWND hwndWinamp);

class ifc_wasabihelper;
HRESULT Plugin_GetWasabiHelper(ifc_wasabihelper **wasabiHelper);

class ifc_skinhelper;
HRESULT Plugin_GetSkinHelper(ifc_skinhelper **skinHelper);

class ifc_skinnedbrowser;
HRESULT Plugin_GetBrowserSkin(ifc_skinnedbrowser **skinnedBrowser);

HRESULT Plugin_GetWinampWnd(HWND *hwndWinamp);

class ifc_winamphook;
HRESULT Plugin_RegisterWinampHook(ifc_winamphook *hook, UINT *cookieOut);
HRESULT Plugin_UnregisterWinampHook(UINT cookie);

const wchar_t* Plugin_LoadString(UINT id, wchar_t *buffer, int bufferMax);
const char* Plugin_LoadStringAnsi(UINT id, char *buffer, int bufferMax);
HWND Plugin_CreateDialogParam(const wchar_t *templateName, HWND parent, DLGPROC proc, LPARAM param);
INT_PTR Plugin_DialogBoxParam(const wchar_t *templateName, HWND parent, DLGPROC proc, LPARAM param);
HMENU Plugin_LoadMenu(const wchar_t *menuName);
void *Plugin_LoadResource(const wchar_t *resourceType, const wchar_t *resourceName, unsigned long *size);
HACCEL Plugin_LoadAccelerators(const wchar_t *tableName);

class ifc_omimageloader;
HRESULT Plugin_QueryImageLoader(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, ifc_omimageloader **imageLoader);

size_t Plugin_TlsAlloc(void);
BOOL Plugin_TlsFree(size_t index);
void *Plugin_TlsGetValue(size_t index);
BOOL Plugin_TlsSetValue(size_t index, void* value);

typedef void (CALLBACK *PLUGINUNLOADCALLBACK)(void);
void Plugin_RegisterUnloadCallback(PLUGINUNLOADCALLBACK callback);

class ifc_ombrowserclass;
HRESULT Plugin_GetBrowserClass(LPCWSTR pszName, ifc_ombrowserclass **instance);
HRESULT Plugin_UnregisterBrowserClass(LPCWSTR pszName); // internal call

HRESULT Plugin_EnsurePathExist(LPCWSTR pszDirectory);

#define RESPATH_TARGETIE	0x0001		// IE safe path
#define RESPATH_COMPACT		0x0002		// compact path relative to winamp location if possible
HRESULT Plugin_MakeResourcePath(LPWSTR pszBuffer, UINT cchBufferMax, HINSTANCE hInstance, LPCWSTR pszType, LPCWSTR pszName, UINT uFlags);

class ifc_omservicehost;
HRESULT Plugin_ResolveRelativePath(LPCWSTR pszPath, ifc_omservicehost *host, LPWSTR pszBuffer, UINT cchBufferMax);

BOOL Plugin_IsDirectMouseWheelMessage(const UINT uMsg);

#endif //NULLSOFT_WINAMP_OMBROWSER_WA5SERVICE_MAIN_HEADER