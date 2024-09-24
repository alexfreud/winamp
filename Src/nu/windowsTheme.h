#ifndef NULLOSFT_WINAMP_WINDOWTHEME_HEADER
#define NULLOSFT_WINAMP_WINDOWTHEME_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define E_WINTHEME_NOTLOADED	MAKE_HRESULT(0, FACILITY_ITF, 0x0201) // library not loaded 
#define E_WINTHEME_LOADFAILED	MAKE_HRESULT(1, FACILITY_ITF, 0x0202) // library load failed
#define E_WINTHEME_BADFUNCTION	MAKE_HRESULT(1, FACILITY_ITF, 0x0203) // function was not loaded

//---------------------------------------------------------------------------
// NOTE: PartId's and StateId's used in the theme API are defined in the 
//       hdr file <tmschema.h> using the TM_PART and TM_STATE macros.  For
//       example, "TM_PART(BP, PUSHBUTTON)" defines the PartId "BP_PUSHBUTTON".

// UxTheme

typedef HANDLE UXTHEME;  

typedef enum UXTHEMESIZE
{
    TS_MIN,             // minimum size
    TS_TRUE,            // size without stretching
    TS_DRAW,            // size that theme mgr will use to draw part
};

#ifndef ETDT_ENABLE	
#define ETDT_DISABLE					0x00000001
#define ETDT_ENABLE						0x00000002
#define ETDT_USETABTEXTURE				0x00000004
#define ETDT_USEAEROWIZARDTABTEXTURE	0x00000008
#define ETDT_ENABLETAB					(ETDT_ENABLE | ETDT_USETABTEXTURE)
#define ETDT_ENABLEAEROWIZARDTAB		(ETDT_ENABLE | ETDT_USEAEROWIZARDTABTEXTURE)
#define ETDT_VALIDBITS					(ETDT_DISABLE | ETDT_ENABLE | ETDT_USETABTEXTURE | ETDT_USEAEROWIZARDTABTEXTURE)
#endif //ETDT_ENABLE

HRESULT STDAPICALLTYPE UxTheme_LoadLibrary(void);
HRESULT STDAPICALLTYPE UxTheme_GetLoadResult(void);
HRESULT STDAPICALLTYPE UxTheme_IsThemeActive(void);

UXTHEME STDAPICALLTYPE UxOpenThemeData(HWND hwnd, LPCWSTR pszClassList);
HRESULT STDAPICALLTYPE UxCloseThemeData(UXTHEME hTheme);
HRESULT STDAPICALLTYPE UxDrawThemeBackground(UXTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
											 const RECT *pRect, OPTIONAL const RECT *pClipRect);
HRESULT STDAPICALLTYPE UxDrawThemeParentBackground(HWND hwnd, HDC hdc, const RECT *prc);
HRESULT STDAPICALLTYPE UxDrawThemeParentBackgroundEx(HWND hwnd, HDC hdc, DWORD dwFlags, const RECT *prc);
HRESULT STDAPICALLTYPE UxDrawThemeText(UXTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText,
										int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect);
COLORREF STDAPICALLTYPE UxGetThemeSysColor(UXTHEME hTheme, int iColorID);
HRESULT STDAPICALLTYPE UxGetThemeColor(UXTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF *pColor);
HRESULT STDAPICALLTYPE UxGetThemeInt(UXTHEME hTheme, int iPartId, int iStateId, int iPropId, int *piVal);
HRESULT STDAPICALLTYPE UxGetThemeMetric(UXTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int *piVal);
HRESULT STDAPICALLTYPE UxGetThemePartSize(UXTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, UXTHEMESIZE eSize, SIZE *psz);
HRESULT STDAPICALLTYPE UxGetThemeBackgroundContentRect(UXTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect);
HRESULT STDAPICALLTYPE UxSetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);  
BOOL STDAPICALLTYPE UxIsAppThemed(void);
BOOL STDAPICALLTYPE UxIsThemeActive(void);
BOOL STDAPICALLTYPE UxIsThemeBackgroundPartiallyTransparent(UXTHEME hTheme, int iPartId, int iStateId);
HRESULT STDAPICALLTYPE UxEnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);


#endif //NULLOSFT_WINAMP_WINDOWTHEME_HEADER