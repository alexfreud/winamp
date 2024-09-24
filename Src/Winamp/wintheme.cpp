#include "main.h"
#include "./wintheme.h"


typedef HRESULT(WINAPI *UXTHEME_SETWINDOWTHEME)(HWND /*hwnd*/, LPCWSTR /*pszSubAppName*/, LPCWSTR /*pszSubIdList*/);  
typedef BOOL (WINAPI *UXTHEME_ISAPPTHEMED)(void);
typedef HRESULT (WINAPI *DWM_SETWINDOWATTRIBUTE)(HWND /*hwnd*/, DWORD /*dwAttribute*/, LPCVOID /*pvAttribute*/, DWORD /*cbAttribute*/);
typedef HRESULT (WINAPI *DWM_ISCOMPOSITIONENABLED)(BOOL* /*pfEnabled*/);


static DWM_SETWINDOWATTRIBUTE	fnSetWindowAttribute = NULL;
static DWM_ISCOMPOSITIONENABLED	fnIsCompositionEnabled = NULL;
static UXTHEME_SETWINDOWTHEME	fnSetWindowTheme = NULL;
static UXTHEME_ISAPPTHEMED		fnIsAppThemed = NULL;

static HMODULE hDwmModule = NULL;
static HMODULE hUxThemeModule = NULL;
static HRESULT loadDwmResult = E_LIBRARY_NOTLOADED;
static HRESULT loadUxThemeResult = E_LIBRARY_NOTLOADED;

HRESULT UxTheme_LoadLibrary(void)
{
	if (E_LIBRARY_NOTLOADED == loadUxThemeResult)
	{
		hUxThemeModule = LoadLibraryW(L"uxtheme.dll");
		if (!hUxThemeModule) loadUxThemeResult = E_LIBRARY_LOADFAILED;
		else
		{
			fnSetWindowTheme = (UXTHEME_SETWINDOWTHEME)GetProcAddress(hUxThemeModule, "SetWindowTheme");
			fnIsAppThemed = (UXTHEME_ISAPPTHEMED)GetProcAddress(hUxThemeModule, "IsAppThemed");
			loadUxThemeResult = S_OK;
		}
	}
	return loadUxThemeResult;
}

HRESULT Dwm_LoadLibrary(void)
{
	if (E_LIBRARY_NOTLOADED == loadDwmResult)
	{
		hDwmModule = LoadLibraryW(L"dwmapi.dll");
		if (!hDwmModule) loadDwmResult = E_LIBRARY_LOADFAILED;
		else
		{
			fnSetWindowAttribute = (DWM_SETWINDOWATTRIBUTE)GetProcAddress(hDwmModule, "DwmSetWindowAttribute");
			fnIsCompositionEnabled = (DWM_ISCOMPOSITIONENABLED)GetProcAddress(hDwmModule, "DwmIsCompositionEnabled");
			loadDwmResult = S_OK;
		}
	}
	return loadDwmResult;
}

HRESULT UxTheme_GetLoadResult(void)
{
	return loadUxThemeResult;
}

HRESULT Dwm_GetLoadResult()
{
	return loadDwmResult;
}


HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	if (!hUxThemeModule) return loadUxThemeResult;
	if (!fnSetWindowTheme) return E_LIBRARY_BADFUNCTION;
	return fnSetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
}
BOOL IsAppThemed(void)
{
	if (!hUxThemeModule) return FALSE;
	if (!fnIsAppThemed) return FALSE;
	return fnIsAppThemed();
}





HRESULT DwmSetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute)
{
	if (!hDwmModule) return loadDwmResult;
	if (!fnSetWindowAttribute) return E_LIBRARY_BADFUNCTION;
	return fnSetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute);
}

HRESULT DwmIsCompositionEnabled(BOOL *pfEnabled)
{
	if (!hDwmModule) return loadDwmResult;
	if (!fnIsCompositionEnabled) return E_LIBRARY_BADFUNCTION;
	return fnIsCompositionEnabled(pfEnabled);
}
