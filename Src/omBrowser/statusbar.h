#ifndef NULLSOFT_WINAMP_OMBROWSER_STATUSBAR_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_STATUSBAR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define NWC_ONLINEMEDIASTATUSBAR		L"Nullsoft_omBrowserStatusbar"

BOOL Statusbar_RegisterClass(HINSTANCE hInstance);

#define SBS_UPDATELAYOUT		0x00000001
#define SBS_ACTIVE				0x00000002

// messages
#define SBM_FIRST			(WM_USER + 10)

#define SBM_UPDATESKIN			(SBM_FIRST + 0) //wParam = not used, lParam = (LPARAM)(BOOL)redraw.
#define Statusbar_UpdateSkin(/*HWND*/ __hStatusbar, /*BOOL*/ __fRedraw)\
	((BOOL)SENDMSG(__hStatusbar, SBM_UPDATESKIN, 0, (LPARAM)(__fRedraw)))

#define SBM_SETPARENTRECT		(SBM_FIRST + 1) //wParam = not used, lParam = (LPCRECT)parentRect.
#define Statusbar_SetParentRect(/*HWND*/ __hStatusbar, /*LPCRECT*/ __parentRect)\
	((BOOL)SENDMSG(__hStatusbar, SBM_SETPARENTRECT, 0, (LPARAM)(__parentRect)))

#define SBM_SETACTIVE			(SBM_FIRST + 2) //wParam = (BOOL)fActive, lParam = not used.
#define Statusbar_SetActive(/*HWND*/ __hStatusbar, /*BOOL*/ __fActive)\
	((BOOL)SENDMSG(__hStatusbar, SBM_SETACTIVE, (WPARAM)(__fActive), 0L))

#define SBM_UPDATE				(SBM_FIRST + 3) //wParam = 0, lParam = (LPCWSTR)pszText.
#define Statusbar_Update(/*HWND*/ __hStatusbar, /*LPCWSTR*/ __pszText)\
	((BOOL)SENDMSG(__hStatusbar, SBM_UPDATE, 0, (LPARAM)(__pszText)))

#define SBM_SETBROWSERHOST		(SBM_FIRST + 4) //wParam = 0, lParam = (LPARAM)(HWND)hwndBrowserHost.
#define Statusbar_SetBrowserHost(/*HWND*/ __hStatusbar, /*HWND*/ __hwndBrowserHost)\
	((BOOL)SENDMSG(__hStatusbar, SBM_SETBROWSERHOST, 0, (LPARAM)(__hwndBrowserHost)))

#define SBM_ENABLE				(SBM_FIRST + 5) //wParam - not used, lParam = (LPARAM)(BOOL)fEnable. Return previous value
#define Statusbar_Enable(/*HWND*/ __hStatusbar, /*BOOL*/ __fEnable)\
	((BOOL)SENDMSG(__hStatusbar, SBM_ENABLE, 0, (LPARAM)(__fEnable)))

// Notifications (WM_COMMAND)
#define SBN_ENABLECHANGED			1

#endif //NULLSOFT_WINAMP_OMBROWSER_STATUSBAR_HEADER