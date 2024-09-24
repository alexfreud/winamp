#ifndef NULLSOFT_WINAMP_OMBROWSER_COMMON_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_COMMON_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _WIN32_IE_IE70
#define _WIN32_IE_IE70	0x0700
#endif //_WIN32_IE_IE70

#include <wtypes.h>
#include "../nu/trace.h"

#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

#ifndef ARRAYSIZE
#define ARRAYSIZE(blah) (sizeof(blah)/sizeof(*blah))
#endif

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#define __WTEXT(quote) L##quote    
#define WTEXT(quote) __WTEXT(quote)

#ifdef __cplusplus
  #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) ::SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#else
 #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#endif // __cplusplus

#define SENDMLIPC(__hwndML, __ipcMsgId, __param) SENDMSG((__hwndML), WM_ML_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))
#define SENDWAIPC(__hwndWA, __ipcMsgId, __param) SENDMSG((__hwndWA), WM_WA_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))

#define MSGRESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWLP_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }

#define SENDCMD(__hwnd, __ctrlId, __eventId, __hctrl) (SENDMSG((__hwnd), WM_COMMAND, MAKEWPARAM(__ctrlId, __eventId), (LPARAM)(__hctrl)))

#ifndef GetWindowStyle
#define GetWindowStyle(__hwnd) ((UINT)GetWindowLongPtr((__hwnd), GWL_STYLE))
#endif //GetWindowStyle

#ifndef GetWindowStyleEx
#define GetWindowStyleEx(__hwnd) ((UINT)GetWindowLongPtr((__hwnd), GWL_EXSTYLE))
#endif // GetWindowStyleEx

#endif //NULLSOFT_WINAMP_OMBROWSER_COMMON_HEADER