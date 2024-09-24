#ifndef NULLSOFT_AUTH_LOGINBOX_COMMON_HEADER
#define NULLSOFT_AUTH_LOGINBOX_COMMON_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(blah) (sizeof(blah)/sizeof(*blah))
#endif

#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

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

#define MSGRESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWLP_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }

#ifndef GetWindowStyle
#define GetWindowStyle(__hwnd) ((UINT)GetWindowLongPtr((__hwnd), GWL_STYLE))
#endif //GetWindowStyle

#ifndef GetWindowStyleEx
#define GetWindowStyleEx(__hwnd) ((UINT)GetWindowLongPtr((__hwnd), GWL_EXSTYLE))
#endif //GetWindowStyleEx


LPWSTR LoginBox_MallocString(size_t cchLen);
void LoginBox_FreeString(LPWSTR pszString);
void LoginBox_FreeStringSecure(LPWSTR pszString);

LPWSTR LoginBox_ReAllocString(LPWSTR pszString, size_t cchLen);
LPWSTR LoginBox_CopyString(LPCWSTR pszSource);
LPSTR LoginBox_MallocAnsiString(size_t cchLen);
LPSTR LoginBox_CopyAnsiString(LPCSTR pszSource);
void LoginBox_FreeAnsiString(LPSTR pszString);
void LoginBox_FreeAnsiStringSecure(LPSTR pszString);
size_t LoginBox_GetAllocSize(void *memory);
size_t LoginBox_GetStringMax(LPWSTR pszString);
size_t LoginBox_GetAnsiStringMax(LPSTR pszString);

HRESULT LoginBox_WideCharToMultiByte(UINT codePage, DWORD dwFlags, LPCWSTR lpWideCharStr, INT cchWideChar, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar, LPSTR *ppResult);
HRESULT LoginBox_MultiByteToWideChar(UINT codePage, DWORD dwFlags, LPCSTR lpMultiByteStr, INT cbMultiByte, LPWSTR *ppResult);

HRESULT LoginBox_GetWindowText(HWND hwnd, LPWSTR *ppszText, UINT *pcchText);

HRESULT LoginBox_GetConfigPath(LPWSTR pszConfig, BOOL fEnsureExist);
HRESULT LoginBox_EnsurePathExist(LPCWSTR pszDirectory);

BOOL LoginBox_PrintWindow(HWND hwnd, HDC hdc, UINT flags);

BOOL LoginBox_MessageBeep(UINT beepType);

HRESULT LoginBox_IsStringEqualEx(LCID locale, BOOL ignoreCase, LPCWSTR str1, LPCWSTR str2);

#define LoginBox_IsStrEq(str1, str2)\
	LoginBox_IsStringEqualEx(LOCALE_USER_DEFAULT, FALSE, str1, str2)

#define LoginBox_IsStrEqI(str1, str2)\
	LoginBox_IsStringEqualEx(LOCALE_USER_DEFAULT, TRUE, str1, str2)

#define LoginBox_IsStrEqInv(str1, str2)\
	LoginBox_IsStringEqualEx(CSTR_INVARIANT, FALSE, str1, str2)

#define LoginBox_IsStrEqInvI(str1, str2)\
	LoginBox_IsStringEqualEx(CSTR_INVARIANT, TRUE, str1, str2)

UINT LoginBox_GetCurrentTime();
HRESULT LoginBox_GetCurrentLang(LPSTR *ppLang);

HDWP LoginBox_LayoutButtonBar(HDWP hdwp, HWND hwnd, const INT *buttonList, UINT buttonCount, const RECT *prcClient, LONG buttonHeight, LONG buttonSpace, BOOL fRedraw, RECT *prcResult); 

BYTE LoginBox_GetSysFontQuality();
INT LoginBox_GetAveStrWidth(HDC hdc, INT cchLen);
INT LoginBox_GetAveCharWidth(HDC hdc);
BOOL LoginBox_GetWindowBaseUnits(HWND hwnd, INT *pBaseUnitX, INT *pBaseUnitY);
INT LoginBox_GetWindowTextHeight(HWND hwnd, INT paddingDlgUnit);
BOOL LoginBox_GetWindowTextSize(HWND hwnd, INT idealWidth, INT *pWidth, INT *pHeight);

BOOL LoginBox_OpenUrl(HWND hOwner, LPCWSTR pszUrl, BOOL forceExternal);


#endif //NULLSOFT_AUTH_LOGINBOX_COMMON_HEADER