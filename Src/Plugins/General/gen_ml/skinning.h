#ifndef NULLOSFT_MEDIALIBRARY_SKINNING_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNING_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef LPVOID HMLIMGLST;
typedef INT (CALLBACK *MENUCUSTOMIZEPROC)(INT /*action*/, HMENU /*hMenu*/, HDC /*hdc*/, LPARAM /*param*/, ULONG_PTR /*user*/);

BOOL SkinWindow(HWND hwndToSkin, UINT style);
BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);
BOOL UnskinWindow(HWND hwndToUnskin);
BOOL TrackSkinnedPopupMenuEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, 
				HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam);

BOOL IsSkinnedPopupEnabled(BOOL fIgnoreCache);
BOOL EnableSkinnedPopup(BOOL fEnable);

// you can call this from WM_CONTEXTMENU
HANDLE InitSkinnedPopupHook(HWND hwndOwner, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam);
void RemoveSkinnedPopupHook(HANDLE hPopupHook);

#endif //NULLOSFT_MEDIALIBRARY_SKINNING_HEADER