#ifndef NULLOSFT_WINAMP_SETUPCOMMON_HEADER
#define NULLOSFT_WINAMP_SETUPCOMMON_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#ifdef __cplusplus
  #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) ::SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#else
 #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#endif // __cplusplus

#define SENDWAIPC(__ipcMsgId, __param) SENDMSG(hMainWindow, WM_WA_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))

#define MSGRESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWLP_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }
#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)


#endif // NULLOSFT_WINAMP_SETUPCOMMON_HEADER