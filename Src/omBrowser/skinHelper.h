#ifndef NULLSOFT_WINAMP_SKIN_HELPER_HEADER
#define NULLSOFT_WINAMP_SKIN_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../Plugins/General/gen_ml/ml_ipc_0313.h"
#include "../Plugins/General/gen_ml/colors.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/wa_dlg.h"

#include "./ifc_skinhelper.h"
#include "./ifc_skinnedmenu.h"
#include "./ifc_skinnedrating.h"
#include "./ifc_skinnedbrowser.h"

#include <bfc/multipatch.h>

#define MPIID_SKINHELPER		10
#define MPIID_SKINNEDMENU	20
#define MPIID_SKINNEDRATING	30
#define MPIID_SKINNEDBROWSER	40

class SkinHelper : public MultiPatch<MPIID_SKINHELPER, ifc_skinhelper>,
					public MultiPatch<MPIID_SKINNEDMENU, ifc_skinnedmenu>,
					public MultiPatch<MPIID_SKINNEDRATING, ifc_skinnedrating>,
					public MultiPatch<MPIID_SKINNEDBROWSER, ifc_skinnedbrowser>
{

protected:
	SkinHelper(HWND hwndWa);
	~SkinHelper();

public:
	static HRESULT CreateInstance(HWND hwndWinamp, SkinHelper **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_skinhelper */
	HRESULT GetColor(UINT colorIndex, COLORREF *pColor);
    HRESULT GetColorEx(UINT uObject, UINT uPart, UINT uState, COLORREF *pColor);
	HRESULT GetBrush(UINT colorIndex, HBRUSH *pBrush);
	HFONT GetFont(void);
	HRESULT SkinWindow(HWND hwnd, const GUID *windowGuid, UINT flagsEx, FFCALLBACK callbackFF);
	HRESULT SkinControl(HWND hwnd, UINT type, UINT style);
	HRESULT UnskinWindow(HWND hwnd);
	
	/* ifc_skinnedmenu */
	BOOL TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, ifc_menucustomizer *customizer);
	HRESULT IsPopupEnabled(void);
	HANDLE InitPopupHook(HWND hwnd, ifc_menucustomizer *customizer);
	HRESULT RemovePopupHook(HANDLE popupHook);

    /* ifc_skinnedrating */
	HRESULT RatingDraw(HDC hdc, INT maxValue, INT value, INT trackingVal, RECT *prc, UINT fStyle);
	HRESULT RatingHitTest(POINT pt, INT maxValue, RECT *prc, UINT fStyle, LONG *result);
	HRESULT RatingCalcMinRect(INT maxValue, RECT *prc);
	HRESULT RatingCreateMenuCustomizer(HMENU hMenu, ifc_menucustomizer **customizer);

	 /* ifc_skinnedbrowser */
	HRESULT Browser_GetHostCss(wchar_t **ppchHostCss);
	COLORREF Browser_GetBackColor(void);
	COLORREF Browser_GetTextColor(void);
	COLORREF Browser_GetLinkColor(void);
	COLORREF Browser_GetActiveLinkColor(void);
	COLORREF Browser_GetVisitedLinkColor(void);
	COLORREF Browser_GetHoveredLinkColor(void);

public:
	HFONT CreateSkinFont(void);
	void ResetColorCache(void);
	void ResetFontCache(void);

protected:
	HRESULT LoadMediaLibraryModule(void);
	HRESULT InitializeColorData(void);
	HRESULT MlTrackSkinnedPopupMenuEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam, INT *resultOut);
	HRESULT MlInitSkinnedPopupHook(HWND hwnd, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam, HANDLE *hookOut);

protected:
	RECVS_MULTIPATCH;

protected:
	typedef BOOL (__cdecl *MLSKINWINDOWEX)(HWND /*hwnd*/, INT /*type*/, UINT /*style*/);
	typedef BOOL (__cdecl *MLUNSKINWINDOW)(HWND /*hwnd*/);
	typedef BOOL (__cdecl *MLGETSKINCOLOR)(UINT /*uObject*/, UINT /*uPart*/, UINT /*uState*/, COLORREF* /*pColor*/);
	typedef void (__cdecl *MLRESETSKINCOLOR)(void);
	typedef BOOL (__cdecl *MLTRACKSKINNEDPOPUPMENUEX)(HMENU /*hmenu*/, UINT /*fuFlags*/, INT /*x*/, INT /*y*/, HWND /*hwnd*/,
												LPTPMPARAMS /*lptpm*/, HMLIMGLST /*hmlil*/, INT /*width*/, UINT /*skinStyle*/,
												MENUCUSTOMIZEPROC /*customProc*/, ULONG_PTR /*customParam*/);
	typedef BOOL (__cdecl *MLISSKINNEDPOPUPENABLED)(void);
	typedef HANDLE (__cdecl *MLINITSKINNEDPOPUPHOOK)(HWND /*hwnd*/, HMLIMGLST /*hmlil*/, INT /*width*/, UINT /*skinStyle*/,
												MENUCUSTOMIZEPROC /*customProc*/, ULONG_PTR /*customParam*/);
	typedef void (__cdecl *MLREMOVESKINNEDPOPUPHOOK)(HANDLE /*hPopupHook*/);

	typedef BOOL (__cdecl *MLRATINGDRAW)(HDC /*hdc*/, INT /*maxValue*/, INT /*value*/, INT /*trackingVal*/, HMLIMGLST /*hmlil*/, INT /*index*/, RECT* /*prc*/, UINT /*fStyle*/);
	typedef LONG (__cdecl *MLRATINGHITTEST)(POINT /*pt*/, INT /*maxValue*/, HMLIMGLST /*hmlil*/, RECT* /*prc*/, UINT /*fStyle*/);
	typedef BOOL (__cdecl *MLRATINGCALCMINRECT)(INT /*maxValue*/, HMLIMGLST /*hmlil*/, RECT* /*prc*/);
	

protected:
	ULONG	ref;
	HWND	hwndWinamp;
	HMODULE mlModule;
	HRESULT mlLoadResult;
	HFONT	playlistFont;
	HBRUSH	szBrushes[WADLG_NUM_COLORS];
	BOOL	initializeColors;
	size_t	cchHostCss;

	MLSKINWINDOWEX mlSkinWindowEx;
	MLUNSKINWINDOW mlUnskinWindow;
	MLTRACKSKINNEDPOPUPMENUEX mlTrackSkinnedPopupMenuEx;
	MLISSKINNEDPOPUPENABLED mlIsSkinnedPopupEnabled;
	MLINITSKINNEDPOPUPHOOK mlInitSkinnedPopupHook;
	MLREMOVESKINNEDPOPUPHOOK mlRemoveSkinnedPopupHook;
	MLGETSKINCOLOR mlGetSkinColor;
	MLRESETSKINCOLOR mlResetSkinColor;
	MLRATINGDRAW	 mlRatingDraw;
	MLRATINGHITTEST mlRatingHitTest;
	MLRATINGCALCMINRECT mlRatingCalcMinRect;

	WCHAR	szHostCss[396];
};


#endif // NULLSOFT_WINAMP_SKIN_HELPER_HEADER