#ifndef NULLOSFT_MEDIALIBRARY_SKINNING_EXPORT_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNING_EXPORT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef LPVOID HMLIMGLST;
typedef INT (CALLBACK *MENUCUSTOMIZEPROC)(INT /*action*/, HMENU /*hMenu*/, HDC /*hdc*/, LPARAM /*param*/, ULONG_PTR /*user*/);

EXTERN_C _declspec(dllexport) BOOL MlSkinWindow(HWND hwndToSkin, UINT style);
EXTERN_C _declspec(dllexport) BOOL MlSkinWindowEx(HWND hwndToSkin, INT type, UINT style);
EXTERN_C _declspec(dllexport) BOOL MlUnskinWindow(HWND hwndToUnskin);
EXTERN_C _declspec(dllexport) BOOL MlTrackSkinnedPopupMenuEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, 
										HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam);
EXTERN_C _declspec(dllexport) BOOL MlIsSkinnedPopupEnabled(void);
EXTERN_C _declspec(dllexport) BOOL MlEnableSkinnedPopup(BOOL fEnable);
EXTERN_C _declspec(dllexport) HANDLE MlInitSkinnedPopupHook(HWND hwndOwner, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam);
EXTERN_C _declspec(dllexport) void MlRemoveSkinnedPopupHook(HANDLE hPopupHook);

EXTERN_C _declspec(dllexport) BOOL MlGetSkinColor(UINT uObject, UINT uPart, UINT uState, COLORREF *pColor);
EXTERN_C _declspec(dllexport) void MlResetSkinColor(void);

EXTERN_C _declspec(dllexport) BOOL MlRatingDraw(HDC hdc, INT maxValue, INT value, INT trackingVal, HMLIMGLST hmlil, INT index, RECT *prc, UINT fStyle);
EXTERN_C _declspec(dllexport) LONG MlRatingHitTest(POINT pt, INT maxValue, HMLIMGLST hmlil, RECT *prc, UINT fStyle);
EXTERN_C _declspec(dllexport) BOOL MlRatingCalcMinRect(INT maxValue, HMLIMGLST hmlil, RECT *prc);


#endif //NULLOSFT_MEDIALIBRARY_SKINNING_EXPORT_HEADER