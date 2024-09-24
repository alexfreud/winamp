#ifndef NULLSOFT_WINAMP_OMBROWSER_OPTIONS_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_OPTIONS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./browserUiCommon.h"
#include "./obj_ombrowser.h"


HRESULT BrowserOptions_ShowDialog(obj_ombrowser *browserManager, HWND hOwner, UINT style, BROWSEROPTIONSCALLBACK callback, ULONG_PTR user);

typedef HWND (CALLBACK *OPTIONSPAGECREATOR)(HWND /*hParent*/, UINT /*style*/);
HRESULT BrowserOptions_RegisterPage(OPTIONSPAGECREATOR creatorFn);
HRESULT BrowserOptions_UnregisterPage(OPTIONSPAGECREATOR creatorFn);

#define BOM_FIRST		(WM_APP + 10)
#define BOM_GETBROWSER	(BOM_FIRST + 0) // wParam - not used; lParam = (LPARAM)(obj_ombrowser**)ppBrowser; Return TRUE on success;
#define BrowserOptions_GetBrowser(/*HWND*/ __hOptions, /*obj_ombrowser** */ __ppBrowser)\
	((BOOL)SENDMSG((__hOptions), BOM_GETBROWSER, 0, (LPARAM)(__ppBrowser)))

typedef struct __BOMCONFIGCHANGED
{
	const GUID *configUid;
	UINT valueId;
	ULONG_PTR value;
} BOMCONFIGCHANGED;

#define BOM_CONFIGCHANGED	(BOM_FIRST + 1) // wParam - not used; lParam = (LPARAM)(BOMCONFIGCHANGED*)configData;
#define BrowserOptions_ConfigChanged(/*HWND*/ __hOptions, /*BOM_CONFIGCHANGED* */ __configData)\
	(SENDMSG((__hOptions), BOM_CONFIGCHANGED, 0, (LPARAM)(__ppBrowser)))

// Internal Helpers
BOOL Options_SetCheckbox(HWND hwnd, UINT controlId, HRESULT checkedState);

#endif //NULLSOFT_WINAMP_OMBROWSER_OPTIONS_HEADER