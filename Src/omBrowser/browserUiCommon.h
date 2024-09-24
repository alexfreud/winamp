#ifndef NULLSOFT_WINAMP_OMBROWSER_UI_COMMON_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_UI_COMMON_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ifc_omservice;


// styles
#define NBCS_DISABLEHOSTCSS		0x00000001	// no coloring
#define NBCS_POPUPOWNER			0x00000002	// every created popup owner by window
#define NBCS_DISABLEFULLSCREEN	0x00000004	// disable fullscreen mode (popup only)
#define NBCS_NOTOOLBAR			0x00000100	// force no toolbar
#define NBCS_NOSTATUSBAR		0x00000200	// force no statusbar
#define NBCS_NOSERVICECOMMANDS	0x00000400	// force disable service commands
#define NBCS_DISABLECONTEXTMENU	0x00001000	// disable context menu
#define NBCS_DIALOGMODE			0x00002000	// enable dialog mode
#define NBCS_BLOCKPOPUP			0x00004000	// block any popup creation



// messages
#define NBCM_FIRST				(WM_USER + 20)
#define NBVM_FIRST				(NBCM_FIRST + 100)
#define NBPM_FIRST				(NBCM_FIRST + 200)

#define NBCM_GETTOOLBAR		(NBCM_FIRST + 1)	// wParam - not used; lParam - not used; Return: toolbar hwnd.
#define BrowserControl_GetToolbar(/*HWND*/ __hwndControl)\
	((HWND)SENDMSG((__hwndControl), NBCM_GETTOOLBAR, 0, 0L))

#define NBCM_GETSTATUSBAR		(NBCM_FIRST + 2)	// wParam - not used; lParam - not used; Return: statusbar hwnd.
#define BrowserControl_GetStatusbar(/*HWND*/ __hwndControl)\
	((HWND)SENDMSG((__hwndControl), NBCM_GETSTATUSBAR, 0, 0L))

#define NBCM_UPDATESKIN		(NBCM_FIRST + 3) // wParam - not used; lParam - (LPARAM)(BOOL)fRedraw; Return: not used.
#define BrowserControl_UpdateSkin(/*HWND*/ __hwndControl, /*BOOL*/ __fRedraw)\
	(SENDMSG((__hwndControl), NBCM_UPDATESKIN, 0, (LPARAM)(__fRedraw)))

#define NBCM_GETSERVICE		(NBCM_FIRST + 4)	// wParam - not used; lParam = (LPARAM)(ifc_omservice**)serviceOut; Return: TRUE on success.
#define BrowserControl_GetService(/*HWND*/ __hwndControl, /*ifc_omservice** */ __serviceOut)\
	((BOOL)SENDMSG((__hwndControl), NBCM_GETSERVICE, 0, (LPARAM)(__serviceOut)))

#define NAVIGATE_BLANK				MAKEINTRESOURCEW(0)
#define NAVIGATE_HOME				MAKEINTRESOURCEW(1)
#define NAVIGATE_BACK				MAKEINTRESOURCEW(2)
#define NAVIGATE_FORWARD			MAKEINTRESOURCEW(3)
#define NAVIGATE_REFRESH			MAKEINTRESOURCEW(4)
#define NAVIGATE_STOP				MAKEINTRESOURCEW(5)
#define NAVIGATE_REFRESH_COMPLETELY	MAKEINTRESOURCEW(6)

#define NBCM_NAVIGATE			(NBCM_FIRST + 5)	// wParam = (WPARAM)(BOOL)fScheduleIfBlocked; lParam = (LPARAM)(LPWSTR)navigateUrl; Return: TRUE on success.
#define BrowserControl_Navigate(/*HWND*/ __hwndControl, /*LPCWSTR*/ __navigateUrl, /*BOOL*/ __scheduleBlocked)\
	((BOOL)SENDMSG((__hwndControl), NBCM_NAVIGATE, (WPARAM)__scheduleBlocked, (LPARAM)(__navigateUrl)))


#define NBCM_WRITEDOCUMENT	(NBCM_FIRST + 6)	// wParam = (WPARAM)(BOOL)fScheduleIfBlocked; lParam = (LPARAM)(BSTR)documentData; Return: TRUE on success.
#define BrowserControl_WriteDocument(/*HWND*/ __hwndControl, /*BSTR*/ __documentData, /*BOOL*/ __scheduleBlocked)\
	((BOOL)SENDMSG((__hwndControl), NBCM_WRITEDOCUMENT, (WPARAM)__scheduleBlocked, (LPARAM)(__documentData)))

#define NBCM_GETHOST		(NBCM_FIRST + 7)	// wParam - not used; lParam - not used; Return: host hwnd.
#define BrowserControl_GetHost(/*HWND*/ __hwndControl)\
	((HWND)SENDMSG((__hwndControl), NBCM_GETHOST, 0, 0L))

#define NBCM_GETBROWSEROBJECT		(NBCM_FIRST + 8)	// wParam - not used; lParam = (LPARAM)(obj_ombrowser**)browserOut; Return: TRUE on success.
#define BrowserControl_GetBrowserObject(/*HWND*/ __hwndControl, /*obj_ombrowser** */ __browserOut)\
	((BOOL)SENDMSG((__hwndControl), NBCM_GETBROWSEROBJECT, 0, (LPARAM)(__browserOut)))

// operation flags
#define NBCOF_HIDEWIDGET	0x00000001
#define NBCOF_SHOWWIDGET	0x00000002


// operation valid fields mask
#define NBCOM_FLAGS		0x00000001
#define NBCOM_TITLE		0x00000002
#define NBCOM_TEXT		0x00000004

typedef struct __OPERATIONINFO
{
	UINT cbSize;	//sizeof(OPERATIONINFO)
	UINT mask;		// 
	UINT flags;		// show flags;
	LPCWSTR title;	// operation title
	LPCWSTR text;	// operation text
} OPERATIONINFO;

#define NBCM_SHOWOPERATION		(NBCM_FIRST + 9)	// wParam - not sued, lParam - (LPARAM)(OPERATIONINFO*)__pOperationInfo.
#define BrowserControl_ShowOperation(/*HWND*/ __hwndControl, /*const OPERATIONINFO* */ __pOperationInfo)\
	((BOOL)SENDMSG((__hwndControl), NBCM_SHOWOPERATION, 0, (LPARAM)(__pOperationInfo)))

#define NBCM_NAVSTOREDURL		(NBCM_FIRST + 10) // wParam - not used, lParam - not used
#define BrowserControl_NavigateStoredUrl(/*HWND*/ __hwndControl)\
	((BOOL)SNDMSG((__hwndControl), NBCM_NAVSTOREDURL, 0, 0L))

#define NBCM_BLOCK				(NBCM_FIRST + 11) //wParam - not used, lParam = (LPARAM)(BOOL)fBlock;
#define BrowserControl_Block(/*HWND*/ __hwndControl, /*BOOL*/ __fBlock)\
	(SNDMSG((__hwndControl), NBCM_BLOCK, 0, (LPARAM)(__fBlock)))

#define NBCM_SETEXTSTYLE		(NBCM_FIRST + 12) //wParam = (WPARAM)(UINT)extMask, lParam = (LPARAM)(UINT)extStyle; Return UINT with prevoius extended style
#define BrowserControl_SetExtendedStyle(/*HWND*/ __hwndControl, /*UINT*/ __extMask, /*UINT*/__extStyle)\
	((UINT)SNDMSG((__hwndControl), NBCM_SETEXTSTYLE, (WPARAM)(__extMask), (LPARAM)(__extStyle)))

#define NBCM_GETEXTSTYLE		(NBCM_FIRST + 13) //wParam - not used, lParam - not used; Return UINT extended style.
#define BrowserControl_GetExtendedStyle(/*HWND*/ __hwndControl)\
	((UINT)SNDMSG((__hwndControl), NBCM_GETTEXTSTYLE, 0, 0L))

#endif // NULLSOFT_WINAMP_OMBROWSER_UI_COMMON_HEADER