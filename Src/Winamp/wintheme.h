#ifndef NULLOSFT_WINAMP_WINTHEME_HEADER
#define NULLOSFT_WINAMP_WINTHEME_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define E_LIBRARY_NOTLOADED		MAKE_HRESULT(0, FACILITY_ITF, 0x0201) // library not loaded (you just need to call MlDwm_LoadLibrary)
#define E_LIBRARY_LOADFAILED		MAKE_HRESULT(1, FACILITY_ITF, 0x0202) // library load failed (probably not vista?)
#define E_LIBRARY_BADFUNCTION	MAKE_HRESULT(1, FACILITY_ITF, 0x0203) // function was not loaded

// XP UxTheme
HRESULT UxTheme_LoadLibrary(void);
HRESULT UxTheme_GetLoadResult(void);

HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);  
BOOL IsAppThemed(void);

// Vista DWM

HRESULT Dwm_LoadLibrary(void);
HRESULT Dwm_GetLoadResult(void);

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED			0x031E
#define WM_DWMNCRENDERINGCHANGED			0x031F
#define WM_DWMCOLORIZATIONCOLORCHANGED	0x0320
#define WM_DWMWINDOWMAXIMIZEDCHANGE		0x0321
#endif // !WM_DWMCOMPOSITIONCHANGED

typedef enum _DWMWINDOWATTRIBUTE {
    DWMWA_NCRENDERING_ENABLED = 1,
    DWMWA_NCRENDERING_POLICY,
    DWMWA_TRANSITIONS_FORCEDISABLED,
    DWMWA_ALLOW_NCPAINT,
    DWMWA_CAPTION_BUTTON_BOUNDS,
    DWMWA_NONCLIENT_RTL_LAYOUT,
    DWMWA_FORCE_ICONIC_REPRESENTATION,
    DWMWA_FLIP3D_POLICY,
    DWMWA_EXTENDED_FRAME_BOUNDS,
	DWMWA_HAS_ICONIC_BITMAP,            // [set] Indicates an available bitmap when there is no better thumbnail representation.
    DWMWA_DISALLOW_PEEK,                // [set] Don't invoke Peek on the window.
    DWMWA_EXCLUDED_FROM_PEEK,
    DWMWA_LAST
} DWMWINDOWATTRIBUTE;

typedef enum _DWMNCRENDERINGPOLICY {
    DWMNCRP_USEWINDOWSTYLE,
    DWMNCRP_DISABLED,
    DWMNCRP_ENABLED,
    DWMNCRP_LAST
} DWMNCRENDERINGPOLICY;

HRESULT DwmSetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
HRESULT DwmIsCompositionEnabled(BOOL *pfEnabled);

#endif //NULLOSFT_WINAMP_WINTHEME_HEADER