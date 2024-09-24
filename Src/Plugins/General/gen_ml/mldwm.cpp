#include "./mldwm.h"

typedef HRESULT (WINAPI *DWMSETWINDOWATTRIBUTE)(HWND /*hwnd*/, DWORD /*dwAttribute*/, LPCVOID /*pvAttribute*/, DWORD /*cbAttribute*/);
typedef HRESULT (WINAPI *DWMISCOMPOSITIONENABLED)(BOOL* /*pfEnabled*/);

static HMODULE hDwmModule = NULL;
static HRESULT loadResult = E_MLDWM_NOTLOADED;

static DWMSETWINDOWATTRIBUTE fnSetWindowAttribute = NULL;
static DWMISCOMPOSITIONENABLED fnIsCompositionEnabled = NULL;

HRESULT MlDwm_IsLoaded()
{
	return loadResult;
}

HRESULT MlDwm_LoadLibrary(void)
{
	if (E_MLDWM_NOTLOADED == loadResult)
	{
		hDwmModule = LoadLibraryW(L"dwmapi.dll");
		if (!hDwmModule) loadResult = E_MLDWM_LOADFAILED;
		else
		{
			fnSetWindowAttribute = (DWMSETWINDOWATTRIBUTE)GetProcAddress(hDwmModule, "DwmSetWindowAttribute");
			fnIsCompositionEnabled = (DWMISCOMPOSITIONENABLED)GetProcAddress(hDwmModule, "DwmIsCompositionEnabled");
			loadResult = S_OK;
		}
	}
	return loadResult;
}


HRESULT MlDwm_SetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute)
{
	if (!hDwmModule) return loadResult;
	if (!fnSetWindowAttribute) return E_MLDWM_BADFUNCTION;
	return fnSetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute);
}

HRESULT MlDwm_IsCompositionEnabled(BOOL *pfEnabled)
{
	if (!hDwmModule) return loadResult;
	if (!fnIsCompositionEnabled) return E_MLDWM_BADFUNCTION;
	return fnIsCompositionEnabled(pfEnabled);
}
