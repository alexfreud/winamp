#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_COMMON_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_COMMON_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

#ifdef __cplusplus
  #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) ::SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#else
 #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#endif // __cplusplus

#define SENDMLIPC(__hwndML, __ipcMsgId, __param) SENDMSG((__hwndML), WM_ML_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))

#define SENDWAIPC(__hwndWA, __ipcMsgId, __param) SENDMSG((__hwndWA), WM_WA_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))

#define SENDCMD(__hwnd, __ctrlId, __eventId, __hctrl) (SENDMSG((__hwnd), WM_COMMAND, MAKEWPARAM(__ctrlId, __eventId), (LPARAM)(__hctrl)))

#define DIALOG_RESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWLP_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }

#ifndef GetWindowStyle
  #define GetWindowStyle(__hwnd) ((UINT)GetWindowLongPtr((__hwnd), GWL_STYLE))
#endif //GetWindowStyle

#ifndef SetWindowStyle
  #define SetWindowStyle(__hwnd, __style) (SetWindowLongPtr((__hwnd), GWL_STYLE, (__style)))
#endif //SetWindowStyle

#ifndef GetWindowStyleEx
  #define GetWindowStyleEx(__hwnd) ((UINT)GetWindowLongPtr((__hwnd), GWL_EXSTYLE))
#endif // GetWindowStyleEx

#ifndef SetWindowStyleEx
  #define SetWindowStyleEx(__hwnd, __style) (SetWindowLongPtr((__hwnd), GWL_EXSTYLE, (__style)))
#endif //SetWindowStyle

#ifndef RECTWIDTH
  #define RECTWIDTH(__r) ((__r).right - (__r).left)
#endif

#ifndef RECTHEIGHT
  #define RECTHEIGHT(__r) ((__r).bottom - (__r).top)
#endif

#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifndef ARRAYSIZE
  #define ARRAYSIZE(_a) (sizeof(_a)/sizeof((_a)[0]))
#endif

#ifndef ABS
  #define ABS(x) (((x) > 0) ? (x) : (-x))
#endif

#ifndef MIN
  #define MIN(v1, v2) (((v1) < (v2)) ? (v1) : (v2))
#endif

#ifndef MAX
  #define MAX(v1, v2) (((v1) > (v2)) ? (v1) : (v2))
#endif


#endif //_NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_COMMON_HEADER
