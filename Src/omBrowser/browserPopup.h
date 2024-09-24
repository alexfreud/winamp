#ifndef NULLSOFT_WINAMP_OMBROWSER_POPUP_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_POPUP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./browserUiCommon.h"

#define NWC_OMBROWSERPOPUP		L"Nullsoft_omBrowserPopup"

// {6EFB8AF7-C54F-43fb-B1DD-DF2EE7E67703}
static const GUID WTID_BrowserPopup = 
{ 0x6efb8af7, 0xc54f, 0x43fb, { 0xb1, 0xdd, 0xdf, 0x2e, 0xe7, 0xe6, 0x77, 0x3 } };

// {00000010-0000-00FF-8000-C5E2FB8CD50B} // guid will be incremented for each instance
static const GUID SkinClass_BrowserPopup = 
{ 0x00000010, 0x0000, 0x00FF, { 0x80, 0x00, 0xc5, 0xe2, 0xfb, 0x8c, 0xd5, 0xb } };


typedef void (CALLBACK *DISPATCHAPC)(IDispatch *pDisp, ULONG_PTR /*param*/);

HWND BrowserPopup_Create(obj_ombrowser *browserManager, ifc_omservice *service, UINT fStyle, INT x, INT y, INT cx, INT cy, HWND hOwner, DISPATCHAPC callback, ULONG_PTR param);

// browser common messages
#define BrowserPopup_GetToolbar(/*HWND*/ __hwndPopup)\
	BrowserControl_GetToolbar(__hwndPopup)
	
#define BrowserPopup_GetStatusbar(/*HWND*/ __hwndPopup)\
	BrowserControl_GetStatusbar(__hwndPopup)

#define BrowserPopup_GetHost(/*HWND*/ __hwndPopup)\
	BrowserControl_GetHost(__hwndPopup)

#define BrowserPopup_UpdateSkin(/*HWND*/ __hwndPopup, /*BOOL*/ __fRedraw)\
	BrowserControl_UpdateSkin(__hwndPopup, __fRedraw)
	
#define BrowserPopup_GetService(/*HWND*/ __hwndPopup, /*ifc_omservice** */ __serviceOut)\
	BrowserControl_GetService(__hwndPopup, __serviceOut)

#define BrowserPopup_Navigate(/*HWND*/ __hwndPopup, /*LPCWSTR*/ __navigateUrl, /*BOOL*/ __scheduleBlocked)\
	BrowserControl_Navigate(__hwndPopup, __navigateUrl, __scheduleBlocked)

#define BrowserPopup_NavigateHome(/*HWND*/ __hwndPopup, /*BOOL*/ __scheduleBlocked)\
	BrowserPopup_Navigate((__hwndPopup), NAVIGATE_HOME, (__scheduleBlocked))

#define BrowserPopup_WriteDocument(/*HWND*/ __hwndPopup, /*BSTR*/ __documentData, /*BOOL*/ __scheduleBlocked)\
	BrowserControl_WriteDocument(__hwndPopup, __documentData, __scheduleBlocked)

#define BrowserPopup_ShowOperation(/*HWND*/ __hwndView, /*const OPERATIONINFO* */ __pOperationInfo)\
	BrowserControl_ShowOperation(__hwndView, __pOperationInfo)

// browser popup messages
#define NBPM_PARENTCHANGED		(NBPM_FIRST + 1) // wParam = not used, lParam = not used

#define NBPM_SKINREFRESHING		(NBPM_FIRST + 2) // wParam = not used, lParam = not used. Return 0 to allow, non zero to prevent.
#define BrowserPopup_SkinRefreshing(/*HWND*/ __hwndPopup)\
	SENDMSG((__hwndPopup), NBPM_SKINREFRESHING, 0, 0L)

#define NBPM_SKINREFRESHED		(NBPM_FIRST + 3) // wParam = not used, lParam = not used.
#define BrowserPopup_SkinRefreshed(/*HWND*/ __hwndPopup)\
	SENDMSG((__hwndPopup), NBPM_SKINREFRESHED, 0, 0L)

#define NBPM_SETFRAMEPOS			(NBPM_FIRST + 4) // wParam = not used, lParam = (LPARAM)(WINDOWPOS*)pwp;
#define BrowserPopup_SetFramePos(/*HWND*/ __hwndPopup, /*HWND*/__hwndInsertAfter, /*INT*/ __x, /*INT*/ __y, /*INT*/__cx, /*INT*/ __cy, /*UINT*/ __flags)\
	{ WINDOWPOS wp; wp.hwndInsertAfter = (__hwndInsertAfter); wp.x = (__x); wp.y = (__y); wp.cx = (__cx); wp.cy = (__cy); wp.flags = (__flags);\
		SENDMSG((__hwndPopup), NBPM_SETFRAMEPOS, 0, (LPARAM)(&wp));}

#define NBPM_ACTIVATEFRAME		(NBPM_FIRST + 5) // wParam = not used, lParam = not used.
#define BrowserPopup_ActivateFrame(/*HWND*/ __hwndPopup)\
	SENDMSG((__hwndPopup), NBPM_ACTIVATEFRAME, 0, 0L)

#define NBPM_REFRESHTITLE			(NBPM_FIRST + 6) // wParam - not used, lParam - not used, return TRUE on success
#define BrowserPopup_RefreshTitle(/*HWND*/ __hwndPopup)\
	((BOOL)SENDMSG((__hwndPopup), NBPM_REFRESHTITLE, 0, 0L))

#endif //NULLSOFT_WINAMP_OMBROWSER_POPUP_HEADER