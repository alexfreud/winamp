#include "./skinExport.h"
#include "./skinning.h"
#include "./colors.h"
#include "./ml_rating.h"

extern HMLIMGLST hmlilRating;

EXTERN_C _declspec(dllexport) BOOL MlSkinWindow(HWND hwndToSkin, UINT style)
{
	return SkinWindow(hwndToSkin, style);
}
EXTERN_C _declspec(dllexport) BOOL MlSkinWindowEx(HWND hwndToSkin, INT type, UINT style)
{
	return SkinWindowEx(hwndToSkin, type, style);
}
EXTERN_C _declspec(dllexport) BOOL MlUnskinWindow(HWND hwndToUnskin)
{
	return UnskinWindow(hwndToUnskin);
}
EXTERN_C _declspec(dllexport) BOOL MlTrackSkinnedPopupMenuEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm,
										HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam)
{
	return TrackSkinnedPopupMenuEx(hmenu, fuFlags, x, y, hwnd, lptpm, hmlil, width, skinStyle, customProc, customParam);
}

EXTERN_C _declspec(dllexport) BOOL MlIsSkinnedPopupEnabled(void)
{
	return IsSkinnedPopupEnabled(FALSE);
}

EXTERN_C _declspec(dllexport) BOOL MlEnableSkinnedPopup(BOOL fEnable)
{
	return EnableSkinnedPopup(fEnable);
}

EXTERN_C _declspec(dllexport) BOOL MlGetSkinColor(UINT uObject, UINT uPart, UINT uState, COLORREF *pColor)
{
	return SUCCEEDED(MLGetSkinColor(uObject, uPart, uState, pColor));
}

EXTERN_C _declspec(dllexport) void MlResetSkinColor(void)
{
	ResetColors(FALSE);
}

EXTERN_C _declspec(dllexport) HANDLE MlInitSkinnedPopupHook(HWND hwndOwner, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam)
{
	return InitSkinnedPopupHook(hwndOwner, hmlil, width, skinStyle, customProc, customParam);
}

EXTERN_C _declspec(dllexport) void MlRemoveSkinnedPopupHook(HANDLE hPopupHook)
{
	RemoveSkinnedPopupHook(hPopupHook);
}

EXTERN_C _declspec(dllexport) BOOL MlRatingDraw(HDC hdc, INT maxValue, INT value, INT trackingVal, HMLIMGLST hmlil, INT index, RECT *prc, UINT fStyle)
{
	if (NULL == hmlil) hmlil = hmlilRating;
	return MLRatingI_Draw(hdc, maxValue, value, trackingVal, hmlil, index, prc, fStyle);
}

EXTERN_C _declspec(dllexport) LONG MlRatingHitTest(POINT pt, INT maxValue, HMLIMGLST hmlil, RECT *prc, UINT fStyle)
{
	if (NULL == hmlil) hmlil = hmlilRating;
	return MLRatingI_HitTest(pt, maxValue, hmlil, prc, fStyle);
}

EXTERN_C _declspec(dllexport) BOOL MlRatingCalcMinRect(INT maxValue, HMLIMGLST hmlil, RECT *prc)
{
	if (NULL == hmlil) hmlil = hmlilRating;
	return  MLRatingI_CalcMinRect(maxValue, hmlil, prc);
}