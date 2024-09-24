#include "./windowsTheme.h"
#include <shlwapi.h>


typedef UXTHEME (WINAPI *UXOPENTHEMEDATA)(HWND hwnd, LPCWSTR /*pszClassList*/);
typedef HRESULT (WINAPI *UXCLOSETHEMEDATA)(UXTHEME /*hTheme*/);
typedef HRESULT (WINAPI *UXSETWINDOWTHEME)(HWND /*hwnd*/, LPCWSTR /*pszSubAppName*/, LPCWSTR /*pszSubIdList*/);
typedef HRESULT (WINAPI *UXDRAWTHEMEBACKGROUND)(UXTHEME /*hTheme*/, HDC /*hdc*/, INT /*iPartId*/, INT /*iStateId*/, 
												LPCRECT /*pRect*/, LPCRECT /*pClipRect*/);
typedef HRESULT (WINAPI *UXDRAWTHEMEPARENTBACKGROUND)(HWND /*hwnd*/, HDC /*hdc*/, const RECT * /*prc*/);
typedef HRESULT (WINAPI *UXDRAWTHEMEPARENTBACKGROUNDEX)(HWND /*hwnd*/, HDC /*hdc*/, DWORD /*dwFlags*/, const RECT * /*prc*/);
typedef HRESULT (WINAPI *UXDRAWTHEMETEXT)(UXTHEME /*hTheme*/, HDC /*hdc*/, INT /*iPartId*/, INT /*iStateId*/, 
												LPCWSTR /*pszText*/, INT /*iCharCount*/, DWORD /*dwTextFlags*/, 
												DWORD /*dwTextFlags2*/, LPCRECT /*pRect*/);
typedef COLORREF (WINAPI *UXGETTHEMESYSCOLOR)(UXTHEME /*hTheme*/, INT /*iColorID*/);

typedef HRESULT (WINAPI *UXGETTHEMECOLOR)(UXTHEME /*hTheme*/, int /*iPartId*/, int /*iStateId*/, int /*iPropId*/, LPCOLORREF /*pColor*/);
typedef HRESULT (WINAPI *UXGETTHEMEINT)(UXTHEME /*hTheme*/, int /*iPartId*/, int /*iStateId*/, int /*iPropId*/, LPINT /*piVal*/);
typedef HRESULT (WINAPI *UXGETTHEMEMETRIC)(UXTHEME /*hTheme*/, HDC /*hdc*/, int /*iPartId*/, int /*iStateId*/, int /*iPropId*/, LPINT /*piVal*/);
typedef HRESULT (WINAPI *UXGETTHEMEPARTSIZE)(UXTHEME /*hTheme*/, HDC /*hdc*/, int /*iPartId*/, int /*iStateId*/, LPCRECT /*prc*/, UXTHEMESIZE /*eSize*/, LPSIZE /*psz*/);
typedef HRESULT (WINAPI *UXGETTHEMEBACKGROUNDCONTENTRECT)(UXTHEME /*hTheme*/, HDC /*hdc*/, int /*iPartId*/, int /*iStateId*/, LPCRECT /*pBoundingRect*/, LPRECT /*pContentRect*/);
typedef HRESULT (WINAPI *UXENABLETHEMEDIALOGTEXTURE)(HWND /*hwnd*/, DWORD /*dwFlags*/);

typedef BOOL (WINAPI *UXISAPPTHEMED)(void);
typedef BOOL (WINAPI *UXISTHEMEACTIVE)(void);
typedef BOOL (WINAPI *UXISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)(UXTHEME /*hTheme*/, int /*iPartId*/, int /*iStateId*/);



static UXOPENTHEMEDATA uxOpenThemeData = NULL;
static UXCLOSETHEMEDATA uxCloseThemeData = NULL;
static UXSETWINDOWTHEME uxSetWindowTheme = NULL;
static UXISAPPTHEMED	 uxIsAppThemed = NULL;
static UXISTHEMEACTIVE	 uxIsThemeActive = NULL;
static UXDRAWTHEMEBACKGROUND uxDrawThemeBackground = NULL;
static UXDRAWTHEMEPARENTBACKGROUND uxDrawThemeParentBackground = NULL;
static UXDRAWTHEMEPARENTBACKGROUNDEX uxDrawThemeParentBackgroundEx = NULL;
static UXDRAWTHEMETEXT uxDrawThemeText = NULL;
static UXGETTHEMESYSCOLOR uxGetThemeSysColor = NULL;
static UXGETTHEMECOLOR uxGetThemeColor = NULL;
static UXGETTHEMEINT uxGetThemeInt = NULL;
static UXGETTHEMEMETRIC uxGetThemeMetric = NULL;
static UXGETTHEMEPARTSIZE uxGetThemePartSize = NULL;
static UXGETTHEMEBACKGROUNDCONTENTRECT uxGetThemeBackgroundContentRect = NULL;
static UXISTHEMEBACKGROUNDPARTIALLYTRANSPARENT uxIsThemeBackgroundPartiallyTransparent = NULL;
static UXENABLETHEMEDIALOGTEXTURE uxEnableThemeDialogTexture = NULL;

static HMODULE uxModule = NULL;
static HRESULT uxLoadResult = E_WINTHEME_NOTLOADED;
static BOOL fSysCheckOk = -1;
static BOOL fComCtlCheckOk = -1;


#define UXTHEME_LOADCHECK(__uxFunction){\
	if (NULL == uxModule && FAILED(UxTheme_LoadLibrary())) return uxLoadResult;\
	if (NULL == (__uxFunction)) return E_WINTHEME_BADFUNCTION;}

#define UXTHEME_LOADCHECK_RESULT(__uxFunction, __failResult){\
	if ((NULL == uxModule && FAILED(UxTheme_LoadLibrary())) || NULL == (__uxFunction))\
		return (__failResult);}

HRESULT STDAPICALLTYPE UxTheme_LoadLibrary(void)
{
	if (E_WINTHEME_NOTLOADED == uxLoadResult)
	{
		uxModule = LoadLibraryW(L"uxtheme.dll");
		if (NULL == uxModule)
		{
			uxLoadResult = E_WINTHEME_LOADFAILED;
		}
		else
		{
			uxSetWindowTheme = (UXSETWINDOWTHEME)GetProcAddress(uxModule, "SetWindowTheme");
			uxIsAppThemed = (UXISAPPTHEMED)GetProcAddress(uxModule, "IsAppThemed");
			uxIsThemeActive = (UXISTHEMEACTIVE)GetProcAddress(uxModule, "IsThemeActive");
			uxOpenThemeData = (UXOPENTHEMEDATA)GetProcAddress(uxModule, "OpenThemeData");
			uxCloseThemeData = (UXCLOSETHEMEDATA)GetProcAddress(uxModule, "CloseThemeData");
			uxDrawThemeBackground = (UXDRAWTHEMEBACKGROUND)GetProcAddress(uxModule, "DrawThemeBackground");
			uxDrawThemeParentBackground = (UXDRAWTHEMEPARENTBACKGROUND)GetProcAddress(uxModule, "DrawThemeParentBackground");
			uxDrawThemeParentBackgroundEx = (UXDRAWTHEMEPARENTBACKGROUNDEX)GetProcAddress(uxModule, "DrawThemeParentBackgroundEx");
			uxDrawThemeText = (UXDRAWTHEMETEXT)GetProcAddress(uxModule, "DrawThemeText");
			uxGetThemeSysColor = (UXGETTHEMESYSCOLOR)GetProcAddress(uxModule, "GetThemeSysColor");
			uxGetThemeColor = (UXGETTHEMECOLOR)GetProcAddress(uxModule, "GetThemeColor");
			uxGetThemeInt = (UXGETTHEMEINT)GetProcAddress(uxModule, "GetThemeInt");
			uxGetThemeMetric = (UXGETTHEMEMETRIC)GetProcAddress(uxModule, "GetThemeMetric");
			uxGetThemePartSize = (UXGETTHEMEPARTSIZE)GetProcAddress(uxModule, "GetThemePartSize");
			uxGetThemeBackgroundContentRect = (UXGETTHEMEBACKGROUNDCONTENTRECT)GetProcAddress(uxModule, "GetThemeBackgroundContentRect");
			uxIsThemeBackgroundPartiallyTransparent = (UXISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)GetProcAddress(uxModule, "IsThemeBackgroundPartiallyTransparent");
			uxEnableThemeDialogTexture = (UXENABLETHEMEDIALOGTEXTURE)GetProcAddress(uxModule, "EnableThemeDialogTexture");

			uxLoadResult = S_OK;
		}
	}

	if (-1 == fSysCheckOk)
	{
		OSVERSIONINFOW vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (FALSE != GetVersionEx(&vi) && 
			(vi.dwMajorVersion > 5 || (vi.dwMajorVersion == 5 && vi.dwMinorVersion > 0)))
		{
			fSysCheckOk = TRUE;
		}
		else
		{
			fSysCheckOk = FALSE;
		}
	}

	if (-1 == fComCtlCheckOk)
	{
		fComCtlCheckOk = FALSE;

		HMODULE hComCtl = LoadLibraryW(L"comctl32.dll");
		if (NULL != hComCtl)
		{
			HRESULT (CALLBACK *waDllGetVersion)(DLLVERSIONINFO*) = (HRESULT (CALLBACK *)(DLLVERSIONINFO*))GetProcAddress(hComCtl, "DllGetVersion");
			if (NULL != waDllGetVersion)
			{
				DLLVERSIONINFO dllVer;
				dllVer.cbSize = sizeof(DLLVERSIONINFO);
				if (S_OK == waDllGetVersion(&dllVer) && dllVer.dwMajorVersion >= 6) 
				{
					fComCtlCheckOk = TRUE;
				}
				FreeLibrary(hComCtl);
			}
		}
	}


	return uxLoadResult;
}

HRESULT STDAPICALLTYPE UxTheme_IsThemeActive(void)
{
	if (NULL == uxIsAppThemed)
	{
		if(FAILED(UxTheme_LoadLibrary()))
			return uxLoadResult;
		if (NULL == uxIsAppThemed) return E_WINTHEME_BADFUNCTION;
	}

	if (NULL == uxIsThemeActive)
	{
		if(FAILED(UxTheme_LoadLibrary()))
			return uxLoadResult;
		if (NULL == uxIsThemeActive) return E_WINTHEME_BADFUNCTION;
	}

	if (TRUE != fSysCheckOk || TRUE != fComCtlCheckOk ||
		FALSE == uxIsAppThemed() || FALSE == uxIsThemeActive())
	{
		return S_FALSE;
	}

	return S_OK;


}

HRESULT STDAPICALLTYPE UxTheme_GetLoadResult(void)
{
	return uxLoadResult;
}

UXTHEME STDAPICALLTYPE UxOpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	UXTHEME_LOADCHECK_RESULT(uxOpenThemeData, NULL);
	return uxOpenThemeData(hwnd, pszClassList);
}

HRESULT STDAPICALLTYPE UxCloseThemeData(UXTHEME hTheme)
{
	UXTHEME_LOADCHECK(uxCloseThemeData);
	return uxCloseThemeData(hTheme);
}

HRESULT STDAPICALLTYPE UxSetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	UXTHEME_LOADCHECK(uxSetWindowTheme);
	return uxSetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
}

BOOL STDAPICALLTYPE UxIsAppThemed(void)
{
	UXTHEME_LOADCHECK_RESULT(uxIsAppThemed, FALSE);
	return uxIsAppThemed();
}

BOOL STDAPICALLTYPE UxIsThemeActive(void)
{
	UXTHEME_LOADCHECK_RESULT(UxIsThemeActive, FALSE);
	return UxIsThemeActive();
}

HRESULT STDAPICALLTYPE UxDrawThemeBackground(UXTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
											 const RECT *pRect, OPTIONAL const RECT *pClipRect)
{
	UXTHEME_LOADCHECK(uxDrawThemeBackground);
	return uxDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

HRESULT STDAPICALLTYPE UxDrawThemeParentBackground(HWND hwnd, HDC hdc, const RECT *prc)
{
	UXTHEME_LOADCHECK(uxDrawThemeParentBackground);
	return uxDrawThemeParentBackground(hwnd, hdc, prc);
}

HRESULT STDAPICALLTYPE UxDrawThemeParentBackgroundEx(HWND hwnd, HDC hdc, DWORD dwFlags, const RECT *prc)
{
	UXTHEME_LOADCHECK(uxDrawThemeParentBackgroundEx);
	return uxDrawThemeParentBackgroundEx(hwnd, hdc, dwFlags, prc);
}

HRESULT STDAPICALLTYPE UxDrawThemeText(UXTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText,
										int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect)
{
	UXTHEME_LOADCHECK(uxDrawThemeText);
	return uxDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
}

COLORREF STDAPICALLTYPE UxGetThemeSysColor(UXTHEME hTheme, int iColorID)
{
	UXTHEME_LOADCHECK_RESULT(uxGetThemeSysColor, RGB(255, 0, 255));
	return uxGetThemeSysColor(hTheme, iColorID);
}

HRESULT STDAPICALLTYPE UxGetThemeColor(UXTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF *pColor)
{
	UXTHEME_LOADCHECK(uxGetThemeColor);
	return uxGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
}
HRESULT STDAPICALLTYPE UxGetThemeInt(UXTHEME hTheme, int iPartId, int iStateId, int iPropId, int *piVal)
{
	UXTHEME_LOADCHECK(uxGetThemeInt);
	return uxGetThemeInt(hTheme, iPartId, iStateId, iPropId, piVal);
}
HRESULT STDAPICALLTYPE UxGetThemeMetric(UXTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int *piVal)
{
	UXTHEME_LOADCHECK(uxGetThemeMetric);
	return uxGetThemeMetric(hTheme, hdc, iPartId, iStateId, iPropId, piVal);
}
HRESULT STDAPICALLTYPE UxGetThemePartSize(UXTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, UXTHEMESIZE eSize, SIZE *psz)
{
	UXTHEME_LOADCHECK(uxGetThemePartSize);
	return uxGetThemePartSize(hTheme, hdc, iPartId, iStateId, prc, eSize, psz);
}

HRESULT STDAPICALLTYPE UxGetThemeBackgroundContentRect(UXTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect)
{
	UXTHEME_LOADCHECK(uxGetThemeBackgroundContentRect);
	return uxGetThemeBackgroundContentRect(hTheme, hdc, iPartId, iStateId, pBoundingRect, pContentRect);
}

BOOL STDAPICALLTYPE UxIsThemeBackgroundPartiallyTransparent(UXTHEME hTheme, int iPartId, int iStateId)
{
	UXTHEME_LOADCHECK_RESULT(uxIsThemeBackgroundPartiallyTransparent, FALSE);
	return uxIsThemeBackgroundPartiallyTransparent(hTheme, iPartId, iStateId);
}

HRESULT STDAPICALLTYPE UxEnableThemeDialogTexture(HWND hwnd, DWORD dwFlags)
{
	UXTHEME_LOADCHECK(uxEnableThemeDialogTexture);
	return uxEnableThemeDialogTexture(hwnd, dwFlags);
}
