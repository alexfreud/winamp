#ifndef NULLSOFT_WINAMP_OMBROWSER_CURTAIN_CONTROL_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_CURTAIN_CONTROL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define NWC_ONLINEMEDIACURTAIN		L"Nullsoft_omBrowserCurtain"

BOOL Curtain_RegisterClass(HINSTANCE hInstance);

#define CWM_FIRST			(WM_USER + 20)  

#define CWM_SETOPERATIONTEXT		(CWM_FIRST + 0) //wParam = not used, lParam = (LPARAM)(LPCWSTR)pszOperationText.
#define Curtain_SetOperationText(/*HWND*/ __hCurtain, /*LPCWSTR*/__operationText)\
	((BOOL)SENDMSG(__hCurtain, CWM_SETOPERATIONTEXT, 0, (LPARAM)(__operationText)))

#define CWM_UPDATESKIN			(CWM_FIRST + 1) //wParam = not used, lParam = (LPARAM)(BOOL)fRedraw.
#define Curtain_UpdateSkin(/*HWND*/ __hCurtain, /*BOOL*/ __fRedraw)\
	(SENDMSG(__hCurtain, CWM_UPDATESKIN, 0, (LPARAM)(__fRedraw)))

#endif //NULLSOFT_WINAMP_OMBROWSER_CURTAIN_CONTROL_HEADER