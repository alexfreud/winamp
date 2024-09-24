#include "main.h"

#ifndef WA_DLG_IMPLEMENT
#define WA_DLG_IMPLEMENT
#endif // WA_DLG_IMPLEMENT
#include "../winamp/wa_dlg.h"
#undef WA_DLG_IMPLEMENT
#include "./skinHelper.h"
#include "./ratingMenuCustomizer.h"
#include "./ifc_menucustomizer.h"
#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>

#define ENSURE_FUNCTION_LOADED(__function) {\
	HRESULT hr;\
	if (NULL == (__function)) 	{\
		hr = LoadMediaLibraryModule();\
		if (SUCCEEDED(hr) && NULL == (__function)) hr = E_UNEXPECTED;\
		if (FAILED(hr)) return hr; }}

#define BOOL2HRESULT(__result) ((FALSE != (__result)) ? S_OK : E_FAIL)

SkinHelper::SkinHelper(HWND hwndWa)
	: ref(1), hwndWinamp(hwndWa), mlModule(NULL), mlLoadResult(S_FALSE),
	  playlistFont(NULL), initializeColors(TRUE), cchHostCss(0),
	  mlSkinWindowEx(NULL), mlUnskinWindow(NULL), mlTrackSkinnedPopupMenuEx(NULL),
	  mlIsSkinnedPopupEnabled(NULL), mlInitSkinnedPopupHook(NULL),
	  mlRemoveSkinnedPopupHook(NULL), mlGetSkinColor(NULL), mlResetSkinColor(NULL),
	  mlRatingDraw(NULL), mlRatingHitTest(NULL), mlRatingCalcMinRect(NULL)
{
	ZeroMemory(szBrushes, sizeof(HBRUSH) * ARRAYSIZE(szBrushes));
	ZeroMemory(szHostCss, sizeof(szHostCss));
}

SkinHelper::~SkinHelper()
{
	if (NULL != playlistFont)
		DeleteObject(playlistFont);

	for (int i = 0; i < ARRAYSIZE(szBrushes); i++)
	{
		if (NULL != szBrushes[i]) DeleteObject(szBrushes[i]);
	}

	if (NULL != mlModule)
		FreeLibrary(mlModule);
}

HRESULT SkinHelper::CreateInstance(HWND hwndWinamp, SkinHelper **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = NULL;
	
	if (NULL == hwndWinamp || FALSE == IsWindow(hwndWinamp))
		return E_INVALIDARG;

	*instance = new SkinHelper(hwndWinamp);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}
	
size_t SkinHelper::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t SkinHelper::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int SkinHelper::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_SkinHelper))
		*object = static_cast<ifc_skinhelper*>(this);
	else if (IsEqualIID(interface_guid, IFC_SkinnedMenu))
		*object = static_cast<ifc_skinnedmenu*>(this);
	else if (IsEqualIID(interface_guid, IFC_SkinnedRating))
		*object = static_cast<ifc_skinnedrating*>(this);
	else if (IsEqualIID(interface_guid, IFC_SkinnedBrowser))
		*object = static_cast<ifc_skinnedbrowser*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT SkinHelper::GetColor(UINT colorIndex, COLORREF *pColor)
{
	if (NULL == pColor) return E_POINTER;
	
	if (colorIndex >= WADLG_NUM_COLORS)
		return E_INVALIDARG;
	
	if (FALSE != initializeColors)
	{
		HRESULT hr = InitializeColorData();
		if (FAILED(hr)) return hr;
	}
	
	*pColor = WADlg_getColor(colorIndex);
	return S_OK;
}

HRESULT SkinHelper::GetColorEx(UINT uObject, UINT uPart, UINT uState, COLORREF *pColor)
{
	if (NULL == pColor) return E_POINTER;
	
	ENSURE_FUNCTION_LOADED(mlGetSkinColor);
	BOOL result = mlGetSkinColor(uObject, uPart, uState, pColor);
	return BOOL2HRESULT(result);
}

HRESULT SkinHelper::GetBrush(UINT colorIndex, HBRUSH *pBrush)
{
	if (NULL == pBrush) return E_POINTER;

	if (FALSE != initializeColors) 
		InitializeColorData();

	if (colorIndex < WADLG_NUM_COLORS)
	{
		*pBrush = szBrushes[colorIndex];
		if (NULL == *pBrush)
		{
			COLORREF color = WADlg_getColor(colorIndex);
			*pBrush = CreateSolidBrush(color);
			szBrushes[colorIndex] = *pBrush;
		}
		return S_OK;
	}
	return E_INVALIDARG;
}

HFONT SkinHelper::GetFont()
{
	if (NULL == playlistFont)
		playlistFont = CreateSkinFont();

	return playlistFont;
}

HRESULT SkinHelper::SkinWindow(HWND hwnd, const GUID *windowGuid, UINT flagsEx, FFCALLBACK callbackFF)
{
	SKINWINDOWPARAM swp;
	swp.cbSize = sizeof(SKINWINDOWPARAM);
	swp.hwndToSkin = hwnd;
	swp.windowGuid = *windowGuid;
	swp.flagsEx = flagsEx;
	swp.callbackFF = callbackFF;

	BOOL result = (BOOL)SENDWAIPC(hwndWinamp, IPC_SKINWINDOW, (WPARAM)&swp);
	return BOOL2HRESULT(result);
}

HRESULT SkinHelper::SkinControl(HWND hwnd, UINT type, UINT style)
{
	ENSURE_FUNCTION_LOADED(mlSkinWindowEx);
	BOOL result = mlSkinWindowEx(hwnd, type, style);
	return BOOL2HRESULT(result);
}

HRESULT SkinHelper::UnskinWindow(HWND hwnd)
{
	ENSURE_FUNCTION_LOADED(mlUnskinWindow);
	BOOL result = mlUnskinWindow(hwnd);
	return BOOL2HRESULT(result);
}

static INT CALLBACK SkinHelper_CustomMenuProc(INT action, HMENU hMenu, HDC hdc, LPARAM param, ULONG_PTR user)
{
	ifc_menucustomizer *customizer = (ifc_menucustomizer*)user;
	if (NULL != customizer)
	{
		return customizer->CustomDraw(hMenu, action, hdc, param);
	}
	return FALSE;
}

BOOL SkinHelper::TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, ifc_menucustomizer *customizer)
{
	if (ForceNoSkinnedMenu != customizer && 	S_OK == IsPopupEnabled())
	{			
		BOOL result;
		HMLIMGLST hmlil = NULL;
		INT width = 0;
		UINT skinStyle = SMS_USESKINFONT;
		MENUCUSTOMIZEPROC customProc = NULL;
		ULONG_PTR customParam = 0;

		if (NULL != customizer)
		{
			hmlil = customizer->GetImageList();
			skinStyle = customizer->GetSkinStyle();
			if (FAILED(customizer->GetWidth(&width))) 
				width = 0;
			
			customProc = SkinHelper_CustomMenuProc;
			customParam = (ULONG_PTR)customizer;
		}

		HRESULT hr = MlTrackSkinnedPopupMenuEx(hMenu, fuFlags, x, y, hwnd, lptpm, 
			hmlil, width, skinStyle, customProc, customParam, &result);
		
		if (SUCCEEDED(hr))
			return result;
	}
	
	return TrackPopupMenuEx(hMenu, fuFlags, x, y, hwnd, lptpm);
}

HRESULT SkinHelper::IsPopupEnabled()
{
	ENSURE_FUNCTION_LOADED(mlIsSkinnedPopupEnabled);
	return (FALSE != mlIsSkinnedPopupEnabled()) ? S_OK : S_FALSE;
}

HANDLE SkinHelper::InitPopupHook(HWND hwnd, ifc_menucustomizer *customizer)
{
	if (ForceNoSkinnedMenu != customizer && S_OK == IsPopupEnabled())
	{			
		HMLIMGLST hmlil = NULL;
		INT width = 0;
		UINT skinStyle = SMS_USESKINFONT;
		MENUCUSTOMIZEPROC customProc = NULL;
		ULONG_PTR customParam = 0;

		if (NULL != customizer)
		{
			hmlil = customizer->GetImageList();
			skinStyle = customizer->GetSkinStyle();
			if (FAILED(customizer->GetWidth(&width))) 
				width = 0;
			
			customProc = SkinHelper_CustomMenuProc;
			customParam = (ULONG_PTR)customizer;
		}

		HANDLE hook;
		HRESULT hr = MlInitSkinnedPopupHook(hwnd, hmlil, width, skinStyle, customProc, customParam, &hook);
		if (FAILED(hr))
			hook = NULL;
		return hook;
	}
	
	return NULL;
}

HRESULT SkinHelper::RemovePopupHook(HANDLE hPopupHook)
{
	ENSURE_FUNCTION_LOADED(mlRemoveSkinnedPopupHook);
	mlRemoveSkinnedPopupHook(hPopupHook);
	return S_OK;
}

HRESULT SkinHelper::MlTrackSkinnedPopupMenuEx(HMENU hmenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam, INT *resultOut)
{
	if (NULL != resultOut)
		*resultOut = 0;

	ENSURE_FUNCTION_LOADED(mlTrackSkinnedPopupMenuEx);
	INT result = mlTrackSkinnedPopupMenuEx(hmenu, fuFlags, x, y, hwnd, lptpm, hmlil, width, skinStyle, customProc, customParam);

	if (NULL != resultOut)
		*resultOut = result;

	return S_OK;
}

HRESULT SkinHelper::MlInitSkinnedPopupHook(HWND hwnd, HMLIMGLST hmlil, INT width, UINT skinStyle, MENUCUSTOMIZEPROC customProc, ULONG_PTR customParam, HANDLE *hookOut)
{
	if (NULL == hookOut) return E_POINTER;
	*hookOut = NULL;
	ENSURE_FUNCTION_LOADED(mlInitSkinnedPopupHook);

	*hookOut = mlInitSkinnedPopupHook(hwnd, hmlil, width, skinStyle, customProc, customParam);
	if (NULL == *hookOut) 
		return E_FAIL;

	return S_OK;
}


HRESULT SkinHelper::RatingDraw(HDC hdc, INT maxValue, INT value, INT trackingVal, RECT *prc, UINT fStyle)
{
	ENSURE_FUNCTION_LOADED(mlRatingDraw);
	BOOL result = mlRatingDraw(hdc, maxValue, value, trackingVal, NULL, 0, prc, fStyle);
	return BOOL2HRESULT(result);
}

HRESULT SkinHelper::RatingHitTest(POINT pt, INT maxValue, RECT *prc, UINT fStyle, LONG *result)
{
	if (NULL == result) return E_POINTER;
	ENSURE_FUNCTION_LOADED(mlRatingHitTest);
	*result = mlRatingHitTest(pt, maxValue, NULL, prc, fStyle);
	return S_OK;
}

HRESULT SkinHelper::RatingCalcMinRect(INT maxValue, RECT *prc)
{
	ENSURE_FUNCTION_LOADED(mlRatingCalcMinRect);
	BOOL result = mlRatingCalcMinRect(maxValue, NULL, prc);
	return BOOL2HRESULT(result);
}

HRESULT SkinHelper::RatingCreateMenuCustomizer(HMENU hMenu, ifc_menucustomizer **customizer)
{
	return RatingMenuCustomizer::CreateInstance(hMenu, this, (RatingMenuCustomizer**)customizer);
}

HRESULT SkinHelper::LoadMediaLibraryModule()
{
	if (S_FALSE != mlLoadResult)
		return mlLoadResult;

	WCHAR szPath[MAX_PATH*2] = {0};
	PathCombine(szPath, (LPCWSTR)SENDWAIPC(hwndWinamp, IPC_GETPLUGINDIRECTORYW, 0), L"gen_ml.dll");

	mlModule = LoadLibrary(szPath);
	if (NULL == mlModule)
		mlLoadResult = HRESULT_FROM_WIN32(GetLastError());

	if (SUCCEEDED(mlLoadResult))
	{
		mlSkinWindowEx = (MLSKINWINDOWEX)GetProcAddress(mlModule, "MlSkinWindowEx");
		mlUnskinWindow = (MLUNSKINWINDOW)GetProcAddress(mlModule, "MlUnskinWindow");
		mlTrackSkinnedPopupMenuEx = (MLTRACKSKINNEDPOPUPMENUEX)GetProcAddress(mlModule, "MlTrackSkinnedPopupMenuEx");
		mlIsSkinnedPopupEnabled = (MLISSKINNEDPOPUPENABLED)GetProcAddress(mlModule, "MlIsSkinnedPopupEnabled");
		mlGetSkinColor = (MLGETSKINCOLOR)GetProcAddress(mlModule, "MlGetSkinColor");
		mlResetSkinColor = (MLRESETSKINCOLOR)GetProcAddress(mlModule, "MlResetSkinColor");
		mlInitSkinnedPopupHook = (MLINITSKINNEDPOPUPHOOK)GetProcAddress(mlModule, "MlInitSkinnedPopupHook");
		mlRemoveSkinnedPopupHook = (MLREMOVESKINNEDPOPUPHOOK)GetProcAddress(mlModule, "MlRemoveSkinnedPopupHook");
		mlRatingDraw =  (MLRATINGDRAW)GetProcAddress(mlModule, "MlRatingDraw");
		mlRatingHitTest =  (MLRATINGHITTEST)GetProcAddress(mlModule, "MlRatingHitTest");
		mlRatingCalcMinRect = (MLRATINGCALCMINRECT)GetProcAddress(mlModule, "MlRatingCalcMinRect");
	}
	return mlLoadResult;
}


HRESULT SkinHelper::InitializeColorData()
{
	if (NULL == hwndWinamp || !IsWindow(hwndWinamp))
		return E_UNEXPECTED;

	for (int i = 0; i < ARRAYSIZE(szBrushes); i++)
	{
		if (NULL != szBrushes[i])
		{
			DeleteObject(szBrushes[i]);
			szBrushes[i] = NULL;
		}
	}

	cchHostCss = 0;

	WADlg_init(hwndWinamp);
	initializeColors = FALSE;

	return S_OK;
}

HFONT SkinHelper::CreateSkinFont()
{
	if (NULL == hwndWinamp || !IsWindow(hwndWinamp))
		return NULL;

	LOGFONT lf = 
	{
		0, /* lfHeight */
		0, /* lfWidth */
		0, /* lfEscapement */
		0, /* lfOrientation */
		FW_NORMAL, /* lfWeight */
		FALSE, /* lfItalic */
		FALSE, /* lfUnderline */
		FALSE, /* lfStrikeOut */
		DEFAULT_CHARSET, /* lfCharSet */
		OUT_DEFAULT_PRECIS, /* lfOutPrecision */
		CLIP_DEFAULT_PRECIS, /* lfClipPrecision */
		DEFAULT_QUALITY, /* lfQuality */
		DEFAULT_PITCH | FF_DONTCARE, /* lfPitchAndFamily */
		TEXT(""), /* lfFaceName */
	};

						
	lf.lfHeight = -(INT)SENDWAIPC(hwndWinamp, IPC_GET_GENSKINBITMAP, 3);
	lf.lfCharSet = (BYTE)SENDWAIPC(hwndWinamp, IPC_GET_GENSKINBITMAP, 2);
					
	LPCSTR faceNameAnsi = (LPCSTR)SENDWAIPC(hwndWinamp,IPC_GET_GENSKINBITMAP, 1);
	if (NULL != faceNameAnsi/* && '\0' != faceNameAnsi*/)
	{
		INT count = MultiByteToWideChar(CP_ACP, 0, faceNameAnsi, -1, lf.lfFaceName, ARRAYSIZE(lf.lfFaceName));
		if (count > 0) count--;
		lf.lfFaceName[count] = L'\0';
	}
	return CreateFontIndirect(&lf);	
}

void SkinHelper::ResetColorCache()
{
	initializeColors = TRUE;
	if (NULL != mlResetSkinColor)
		mlResetSkinColor();
}

void SkinHelper::ResetFontCache()
{
	if (NULL != playlistFont)
	{
		DeleteObject(playlistFont);
		playlistFont = NULL;
	}
}


#define RGBTOHTML(__rgbColor) (((__rgbColor)>>16) & 0xFF | ((__rgbColor)&0xFF00) | (((__rgbColor)<<16)&0xFF0000))

HRESULT SkinHelper::Browser_GetHostCss(wchar_t **ppchHostCss)
{
	if (NULL == ppchHostCss) return E_POINTER;
	*ppchHostCss = NULL;

	if (FALSE != initializeColors)
	{
		HRESULT hr = InitializeColorData();
		if (FAILED(hr)) return hr;
	}

	if (0 == cchHostCss)
	{
		size_t remaining;
		COLORREF szColors[] = 
		{
			Browser_GetBackColor(),
			Browser_GetTextColor(),
			(COLORREF)WADlg_getColor(WADLG_LISTHEADER_BGCOLOR),
			(COLORREF)WADlg_getColor(WADLG_SCROLLBAR_BGCOLOR),
			(COLORREF)WADlg_getColor(WADLG_LISTHEADER_FRAME_TOPCOLOR),
			(COLORREF)WADlg_getColor(WADLG_LISTHEADER_BGCOLOR),
			(COLORREF)WADlg_getColor(WADLG_LISTHEADER_FRAME_BOTTOMCOLOR),
			(COLORREF)WADlg_getColor(WADLG_LISTHEADER_BGCOLOR),
			(COLORREF)WADlg_getColor(WADLG_BUTTONFG),
			Browser_GetLinkColor(),
			Browser_GetActiveLinkColor(),
			Browser_GetVisitedLinkColor(),
			Browser_GetHoveredLinkColor(),
		};
		
		for (INT i = 0; i < ARRAYSIZE(szColors); i++) 
		{
			COLORREF c = szColors[i];
			szColors[i] = RGBTOHTML(c);
		}
		
		HRESULT hr = StringCchPrintfEx(szHostCss, ARRAYSIZE(szHostCss), NULL, &remaining, 
						STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION,
						L"body { "
							L"background-color: #%06X;"
							L"color: #%06X;"
							L"scrollbar-face-color: #%06X;"
							L"scrollbar-track-color: #%06X;"
							L"scrollbar-3dlight-color: #%06X;"
							L"scrollbar-shadow-color: #%06X;"
							L"scrollbar-darkshadow-color: #%06X;"
							L"scrollbar-highlight-color: #%06X;"
							L"scrollbar-arrow-color: #%06X;"
						L"}"
						L"a:link {color: #%06X;}"
						L"a:active {color: #%06X;}"
						L"a:visited {color: #%06X;}"
						L"a:hover {color: #%06X;}", 
						szColors[0], szColors[1], szColors[2], szColors[3], szColors[4], szColors[5], szColors[6], 
						szColors[7], szColors[8], szColors[9], szColors[10], szColors[11], szColors[12]			
						);
		if (FAILED(hr))
			return E_UNEXPECTED;

		cchHostCss = ARRAYSIZE(szHostCss) - remaining;

	}

	LPWSTR buffer = (LPWSTR)CoTaskMemAlloc(sizeof(WCHAR) * (cchHostCss + 1));
	if (NULL == buffer) return E_OUTOFMEMORY;
	
	if (FAILED(StringCchCopy(buffer, cchHostCss + 1, szHostCss)))
		return E_UNEXPECTED;
	
	*ppchHostCss = buffer;
	return S_OK;
}

COLORREF SkinHelper::Browser_GetBackColor(void)
{
	COLORREF rgb;
	if (FAILED(GetColorEx(MLSO_OMBROWSER, BP_BACKGROUND, 0, &rgb)))
		rgb = GetSysColor(COLOR_WINDOW);
	return rgb;
}

COLORREF SkinHelper::Browser_GetTextColor(void)
{
	COLORREF rgb;
	if (FAILED(GetColorEx(MLSO_OMBROWSER, BP_TEXT, BTS_NORMAL, &rgb)))
		rgb = GetSysColor(COLOR_WINDOWTEXT);
	return rgb;
}

COLORREF SkinHelper::Browser_GetLinkColor(void)
{
	COLORREF rgb;
	if (FAILED(GetColorEx(MLSO_OMBROWSER, BP_LINK, BLS_NORMAL, &rgb)))
		rgb = GetSysColor(COLOR_HOTLIGHT);
	return rgb;
}

COLORREF SkinHelper::Browser_GetActiveLinkColor(void)
{
	COLORREF rgb;
	if (FAILED(GetColorEx(MLSO_OMBROWSER, BP_LINK, BLS_ACTIVE, &rgb)))
		rgb = GetSysColor(COLOR_HOTLIGHT);
	return rgb;
}

COLORREF SkinHelper::Browser_GetVisitedLinkColor(void)
{
	COLORREF rgb;
	if (FAILED(GetColorEx(MLSO_OMBROWSER, BP_LINK, BLS_VISITED, &rgb)))
		rgb = GetSysColor(COLOR_WINDOWTEXT);
	return rgb;
}

COLORREF SkinHelper::Browser_GetHoveredLinkColor(void)
{
	COLORREF rgb;
	if (FAILED(GetColorEx(MLSO_OMBROWSER, BP_LINK, BLS_HOVER, &rgb)))
		rgb = GetSysColor(COLOR_HOTLIGHT);
	return rgb;
}

#define CBCLASS SkinHelper
START_MULTIPATCH;
 START_PATCH(MPIID_SKINHELPER)
  M_CB(MPIID_SKINHELPER, ifc_skinhelper, ADDREF, AddRef);
  M_CB(MPIID_SKINHELPER, ifc_skinhelper, RELEASE, Release);
  M_CB(MPIID_SKINHELPER, ifc_skinhelper, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_SKINHELPER, ifc_skinhelper, API_GETCOLOR, GetColor);
  M_CB(MPIID_SKINHELPER, ifc_skinhelper, API_GETCOLOREX, GetColorEx);
  M_CB(MPIID_SKINHELPER, ifc_skinhelper, API_GETBRUSH, GetBrush);
  M_CB(MPIID_SKINHELPER, ifc_skinhelper, API_GETFONT, GetFont);
  M_CB(MPIID_SKINHELPER, ifc_skinhelper, API_SKINWINDOW, SkinWindow);
  M_CB(MPIID_SKINHELPER, ifc_skinhelper, API_SKINCONTROL, SkinControl);
  M_CB(MPIID_SKINHELPER, ifc_skinhelper, API_UNSKINWINDOW, UnskinWindow);
 NEXT_PATCH(MPIID_SKINNEDMENU)
  M_CB(MPIID_SKINNEDMENU, ifc_skinnedmenu, ADDREF, AddRef);
  M_CB(MPIID_SKINNEDMENU, ifc_skinnedmenu, RELEASE, Release);
  M_CB(MPIID_SKINNEDMENU, ifc_skinnedmenu, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_SKINNEDMENU, ifc_skinnedmenu, API_TRACKPOPUP, TrackPopup);
  M_CB(MPIID_SKINNEDMENU, ifc_skinnedmenu, API_ISENABLED, IsPopupEnabled);
  M_CB(MPIID_SKINNEDMENU, ifc_skinnedmenu, API_INITPOPUPHOOK, InitPopupHook);
  M_CB(MPIID_SKINNEDMENU, ifc_skinnedmenu, API_REMOVEPOPUPHOOK, RemovePopupHook);
 NEXT_PATCH(MPIID_SKINNEDRATING)
  M_CB(MPIID_SKINNEDRATING, ifc_skinnedrating, ADDREF, AddRef);
  M_CB(MPIID_SKINNEDRATING, ifc_skinnedrating, RELEASE, Release);
  M_CB(MPIID_SKINNEDRATING, ifc_skinnedrating, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_SKINNEDRATING, ifc_skinnedrating, API_DRAW, RatingDraw);
  M_CB(MPIID_SKINNEDRATING, ifc_skinnedrating, API_HITTEST, RatingHitTest);
  M_CB(MPIID_SKINNEDRATING, ifc_skinnedrating, API_CALCMINRECT, RatingCalcMinRect);
  M_CB(MPIID_SKINNEDRATING, ifc_skinnedrating, API_CREATEMENUCUSTOMIZER, RatingCreateMenuCustomizer);
 NEXT_PATCH(MPIID_SKINNEDBROWSER)
  M_CB(MPIID_SKINNEDBROWSER, ifc_skinnedbrowser, ADDREF, AddRef);
  M_CB(MPIID_SKINNEDBROWSER, ifc_skinnedbrowser, RELEASE, Release);
  M_CB(MPIID_SKINNEDBROWSER, ifc_skinnedbrowser, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_SKINNEDBROWSER, ifc_skinnedbrowser, API_GETHOSTCSS, Browser_GetHostCss);
  M_CB(MPIID_SKINNEDBROWSER, ifc_skinnedbrowser, API_GETBACKCOLOR, Browser_GetBackColor);
  M_CB(MPIID_SKINNEDBROWSER, ifc_skinnedbrowser, API_GETTEXTCOLOR, Browser_GetTextColor);
  M_CB(MPIID_SKINNEDBROWSER, ifc_skinnedbrowser, API_GETLINKCOLOR, Browser_GetLinkColor);
  M_CB(MPIID_SKINNEDBROWSER, ifc_skinnedbrowser, API_GETACTIVELINKCOLOR, Browser_GetActiveLinkColor);
  M_CB(MPIID_SKINNEDBROWSER, ifc_skinnedbrowser, API_GETVISITEDLINKCOLOR, Browser_GetVisitedLinkColor);
  M_CB(MPIID_SKINNEDBROWSER, ifc_skinnedbrowser, API_GETHOVEREDLINKCOLOR, Browser_GetHoveredLinkColor);
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS