#ifndef NULLSOFT_WINAMP_OMBROWSER_VIEW_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_VIEW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./browserUiCommon.h"

#define NWC_OMBROWSERVIEW		L"Nullsoft_omBrowserView"

// {13A6B4C7-1881-4a55-B375-26DF2A830825}
static const GUID WTID_BrowserView = 
{ 0x13a6b4c7, 0x1881, 0x4a55, { 0xb3, 0x75, 0x26, 0xdf, 0x2a, 0x83, 0x8, 0x25 } };


class obj_ombrowser;
HWND BrowserView_Create(obj_ombrowser *browserManager, ifc_omservice *service, HWND hParent, LPCWSTR redirectUrl, UINT style);

// browser common messages
#define BrowserView_GetToolbar(/*HWND*/ __hwndView)\
	BrowserControl_GetToolbar(__hwndView)

#define BrowserView_GetStatusbar(/*HWND*/ __hwndView)\
	BrowserControl_GetStatusbar(__hwndView)

#define BrowserView_GetHost(/*HWND*/ __hwndView)\
	BrowserControl_GetHost(__hwndView)

#define BrowserView_UpdateSkin(/*HWND*/ __hwndView, /*BOOL*/ __fRedraw)\
	BrowserControl_UpdateSkin(__hwndView, __fRedraw)

#define BrowserView_GetService(/*HWND*/ __hwndView, /*ifc_omservice** */ __serviceOut)\
	BrowserControl_GetService(__hwndView, __serviceOut)

#define BrowserView_Navigate(/*HWND*/ __hwndView, /*LPCWSTR*/ __navigateUrl, /*BOOL*/ __scheduleBlocked)\
	BrowserControl_Navigate(__hwndView, __navigateUrl, __scheduleBlocked)

#define BrowserView_NavigateHome(/*HWND*/ __hwndView, /*BOOL*/ __scheduleBlocked)\
	BrowserView_Navigate((__hwndView), NAVIGATE_HOME, (__scheduleBlocked))

#define BrowserView_WriteDocument(/*HWND*/ __hwndView, /*BSTR*/ __documentData, /*BOOL*/ __scheduleBlocked)\
	BrowserControl_WriteDocument(__hwndView, __documentData, __scheduleBlocked)

#define BrowserView_ShowOperation(/*HWND*/ __hwndView, /*const OPERATIONINFO* */ __pOperationInfo)\
	BrowserControl_ShowOperation(__hwndView, __pOperationInfo)

#endif // NULLSOFT_WINAMP_OMBROWSER_VIEW_HEADER