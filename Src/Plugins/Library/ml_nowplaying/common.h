#ifndef NULLSOFT_NOWPLAYING_PLUGIN_COMMON_HEADER
#define NULLSOFT_NOWPLAYING_PLUGIN_COMMON_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

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

#ifdef __cplusplus
  #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) ::SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#else
 #define SENDMSG(__hwnd, __msgId, __wParam, __lParam) SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#endif // __cplusplus

#define SENDMLIPC(__hwndML, __ipcMsgId, __param) SENDMSG((__hwndML), WM_ML_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))
#define SENDWAIPC(__hwndWA, __ipcMsgId, __param) SENDMSG((__hwndWA), WM_WA_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))

#define MSGRESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWL_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }

#define SENDCMD(__hwnd, __ctrlId, __eventId, __hctrl) (SENDMSG((__hwnd), WM_COMMAND, MAKEWPARAM(__ctrlId, __eventId), (LPARAM)(__hctrl)))


LPWSTR Plugin_MallocString(size_t cchLen);
LPWSTR Plugin_ReAllocString(LPWSTR pszString, size_t cchLen);
void Plugin_FreeString(LPWSTR pszString);
LPWSTR Plugin_CopyString(LPCWSTR pszSource);

LPSTR Plugin_MallocAnsiString(size_t cchLen);
LPSTR Plugin_CopyAnsiString(LPCSTR pszSource);
void Plugin_FreeAnsiString(LPSTR pszString);

LPWSTR Plugin_DuplicateResString(LPCWSTR pszResource);
void Plugin_FreeResString(LPWSTR pszResource);
HRESULT Plugin_CopyResString(LPWSTR pszBuffer, INT cchBufferMax, LPCWSTR pszString);

LPSTR Plugin_WideCharToMultiByte(UINT codePage, DWORD dwFlags, LPCWSTR lpWideCharStr, INT cchWideChar, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
LPWSTR Plugin_MultiByteToWideChar(UINT codePage, DWORD dwFlags, LPCSTR lpMultiByteStr, INT cbMultiByte);

void Plugin_SafeRelease(IUnknown *pUnk);


#endif //NULLSOFT_NOWPLAYING_PLUGIN_COMMON_HEADER