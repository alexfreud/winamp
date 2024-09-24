#include "main.h"
#include "./colors.h"
#include "api__gen_ml.h"
#include "./ml_ipc_0313.h"
#include "../Wasabi/bfc/platform/types.h"
#include <tataki/color/skinclr.h>
#include <shlwapi.h>
#include <math.h>

#define RGB_MENU_BACKGROUND				0
#define RGB_MENU_BACKGROUND_SELECTED		1
#define RGB_MENU_TEXT					2
#define RGB_MENU_TEXT_SELECTED			3
#define RGB_MENU_TEXT_GRAYED				4
#define RGB_MENU_FRAME					5
#define RGB_MENU_SEPARATOR				6
#define RGB_MENU_BACKROUND_SELECTEDFRAME	7

#define RGB_TOOLTIP_BACKGROUND			8
#define RGB_TOOLTIP_TEXT				9
#define RGB_TOOLTIP_FRAME				10

#define RGB_OMBROWSER_BACKGROUND			11
#define RGB_OMBROWSER_TEXT				12
#define RGB_OMBROWSER_TEXT_DISABLED		13
#define RGB_OMBROWSER_LINK				14
#define RGB_OMBROWSER_LINK_ACTIVE		15
#define RGB_OMBROWSER_LINK_VISITED		16
#define RGB_OMBROWSER_LINK_HOVER			17

#define COLORCACHE_MAX				(RGB_OMBROWSER_LINK_HOVER + 1)

static COLORREF  szColorCache[COLORCACHE_MAX];
static BOOL	requestColorReset = TRUE;
static HBITMAP bitmapCache = NULL;


#define WACOLOR(__index) WADlg_getColor(__index)
#define BlendWaColors(__index1, __index2, __k)  BlendColors(WACOLOR(__index1), WACOLOR(__index2), __k)

INT GetColorDistance(COLORREF rgb1, COLORREF rgb2)
{
	return (1000 * ((GetRValue(rgb1) - GetRValue(rgb2)) + 
			(GetGValue(rgb1) - GetGValue(rgb2)) +
			(GetBValue(rgb1) - GetBValue(rgb2))))/ (3 * 255);
}

COLORREF GetDarkerColor(COLORREF rgb1, COLORREF rgb2)
{
	INT g1 = (GetRValue(rgb1)*299 + GetGValue(rgb1)*587 + GetBValue(rgb1)*114);
	INT g2 = (GetRValue(rgb2)*299 + GetGValue(rgb2)*587 + GetBValue(rgb2)*114);
	return (g1 < g2) ? rgb1 : rgb2;
}

COLORREF BlendColors(COLORREF rgbTop, COLORREF rgbBottom, INT alpha)
{
	if (alpha > 254) return rgbTop;
	if (alpha < 0) return rgbBottom;

	WORD k = (((255 - alpha)*255 + 127)/255);
	
	return RGB( (GetRValue(rgbTop)*alpha + k*GetRValue(rgbBottom) + 127)/255, 
				(GetGValue(rgbTop)*alpha + k*GetGValue(rgbBottom) + 127)/255, 
				(GetBValue(rgbTop)*alpha + k*GetBValue(rgbBottom) + 127)/255);
}

static COLORREF GetSkinColor(LPCWSTR colorName, COLORREF colorDefault)
{
	return SkinColor::GetColor(colorName, NULL, colorDefault);
}


#define QUERYSKINCOLOR(__cacheColorIndex, __skinColorId, __fallback)\
		{ szColorCache[(__cacheColorIndex)] = GetSkinColor((__skinColorId), (__fallback)); }


void ResetColors(BOOL fImmediate)
{
	requestColorReset = TRUE;

	if (FALSE == fImmediate)
	{
		bitmapCache = NULL;
	}
	else
	{
		WADlg_init(plugin.hwndParent);
		bitmapCache = WADlg_getBitmap();
	}
}

static BOOL UpdateColorCache()
{
	if (NULL == bitmapCache || bitmapCache != WADlg_getBitmap())
	{
		WADlg_init(plugin.hwndParent);
		bitmapCache = WADlg_getBitmap();
	}
	   

	COLORREF rgbFrame1 = WACOLOR((WACOLOR(WADLG_WNDBG) == WACOLOR(WADLG_HILITE)) ?  WADLG_LISTHEADER_BGCOLOR : WADLG_HILITE);
	COLORREF rgbSelected = 	GetSkinColor(L"wasabi.popupmenu.background.selected", WACOLOR(WADLG_HILITE));

	QUERYSKINCOLOR(RGB_MENU_BACKGROUND,				L"wasabi.popupmenu.background",			WACOLOR(WADLG_WNDBG));

	if (rgbSelected == szColorCache[RGB_MENU_BACKGROUND])
		rgbSelected = WACOLOR(WADLG_SELBAR_BGCOLOR);

	szColorCache[RGB_MENU_BACKGROUND_SELECTED] = BlendColors(rgbSelected, szColorCache[RGB_MENU_BACKGROUND], 127);
	szColorCache[RGB_MENU_BACKROUND_SELECTEDFRAME] = ColorAdjustLuma(rgbSelected, -100, TRUE);
	
	QUERYSKINCOLOR(RGB_MENU_TEXT,					L"wasabi.popupmenu.text",				WACOLOR(WADLG_WNDFG));
	QUERYSKINCOLOR(RGB_MENU_TEXT_SELECTED,			L"wasabi.popupmenu.text.selected",		WACOLOR(WADLG_WNDFG));
	QUERYSKINCOLOR(RGB_MENU_TEXT_GRAYED,				L"wasabi.popupmenu.text.inactive",		BlendWaColors(WADLG_WNDFG, WADLG_WNDBG, 76));
	QUERYSKINCOLOR(RGB_MENU_FRAME,					L"wasabi.popupmenu.frame",				rgbFrame1);
	QUERYSKINCOLOR(RGB_MENU_SEPARATOR,				L"wasabi.popupmenu.separator",			rgbFrame1);
	
	QUERYSKINCOLOR(RGB_TOOLTIP_BACKGROUND,			L"wasabi.tooltip.background",			WACOLOR(WADLG_WNDBG));
	QUERYSKINCOLOR(RGB_TOOLTIP_TEXT,					L"wasabi.tooltip.text",					WACOLOR(WADLG_WNDFG));
	QUERYSKINCOLOR(RGB_TOOLTIP_FRAME,				L"wasabi.tooltip.frame",					rgbFrame1);

	COLORREF rgbLinkBase, rgbBrowserBk;
	INT szLinkBaseIndex[] = { WADLG_SELBAR_FGCOLOR, WADLG_SELBAR_BGCOLOR, WADLG_SELCOLOR, WADLG_HILITE, WADLG_ITEMFG};

	rgbBrowserBk = WACOLOR(WADLG_ITEMBG);
	for(INT i = 0; i < ARRAYSIZE(szLinkBaseIndex); i++)
	{
		rgbLinkBase = WACOLOR(szLinkBaseIndex[i]);
		INT d = GetColorDistance(rgbLinkBase, rgbBrowserBk);
		if (d < 0) d = - d;
		if (d > 40) break;
	}
	
	QUERYSKINCOLOR(RGB_OMBROWSER_BACKGROUND,			L"wasabi.ombrowser.background",			rgbBrowserBk);
	QUERYSKINCOLOR(RGB_OMBROWSER_TEXT,				L"wasabi.ombrowser.text",				BlendWaColors(WADLG_ITEMFG, WADLG_ITEMBG, 230));
	QUERYSKINCOLOR(RGB_OMBROWSER_TEXT_DISABLED,		L"wasabi.ombrowser.text.disabled",		BlendWaColors(WADLG_ITEMFG, WADLG_ITEMBG, 120));
	QUERYSKINCOLOR(RGB_OMBROWSER_LINK,				L"wasabi.ombrowser.link",				BlendColors(rgbLinkBase, rgbBrowserBk, 225));
	QUERYSKINCOLOR(RGB_OMBROWSER_LINK_ACTIVE,		L"wasabi.ombrowser.link.active",			rgbLinkBase);
	QUERYSKINCOLOR(RGB_OMBROWSER_LINK_VISITED,		L"wasabi.ombrowser.link.visited",		BlendColors(rgbLinkBase, rgbBrowserBk, 135));
	QUERYSKINCOLOR(RGB_OMBROWSER_LINK_HOVER,			L"wasabi.ombrowser.link.hover",			BlendColors(rgbLinkBase, rgbBrowserBk, 245));

	return TRUE;
}


HRESULT MLGetSkinColor(UINT uObject, UINT uPart, UINT uState, COLORREF *pColor)
{
	
	#define SELECT_PART(__id)  case (__id): switch(uPart) {
	#define SELECT_STATE(__id)  case (__id): switch(uState) {
	#define END_SELECT		} break;
	#define SETRGB(__id, __rgb)  case (__id): *pColor = (__rgb); return S_OK; 
	#define SETWACOLOR(__id, __waIndex)  case (__id): *pColor = WACOLOR(__waIndex); return S_OK; 
	#define SETCACHECOLOR(__id, __cacheIndex)  case (__id): *pColor = szColorCache[(__cacheIndex)]; return S_OK; 

	if (FALSE != requestColorReset && FALSE != UpdateColorCache())
		requestColorReset = FALSE;
	
	switch(uObject)
	{
		SELECT_PART(MLSO_WINDOW)
			SELECT_STATE(WP_BACKGROUND)
				SETWACOLOR(WBS_NORMAL, WADLG_WNDBG)
				SETWACOLOR(WBS_HILITED, WADLG_HILITE)
			END_SELECT
			SETWACOLOR(WP_TEXT, WADLG_WNDFG)
			SETWACOLOR(WP_FRAME, WADLG_HILITE)
		END_SELECT

		SELECT_PART(MLSO_MENU)
			SELECT_STATE(MP_BACKGROUND)
				SETCACHECOLOR(MBS_NORMAL, RGB_MENU_BACKGROUND)
				SETCACHECOLOR(MBS_SELECTED, RGB_MENU_BACKGROUND_SELECTED)
				SETCACHECOLOR(MBS_SELECTEDFRAME, RGB_MENU_BACKROUND_SELECTEDFRAME)
			END_SELECT
			SELECT_STATE(MP_TEXT)
				SETCACHECOLOR(MTS_NORMAL, RGB_MENU_TEXT)
				SETCACHECOLOR(MTS_SELECTED, RGB_MENU_TEXT_SELECTED)
				SETCACHECOLOR(MTS_DISABLED, RGB_MENU_TEXT_GRAYED)
			END_SELECT
			SETCACHECOLOR(MP_SEPARATOR, RGB_MENU_SEPARATOR)
			SETCACHECOLOR(MP_FRAME, RGB_MENU_FRAME)
		END_SELECT

		SELECT_PART(MLSO_TOOLTIP)
			SETCACHECOLOR(TTP_BACKGROUND, RGB_TOOLTIP_BACKGROUND)
			SETCACHECOLOR(TTP_TEXT, RGB_TOOLTIP_TEXT)
			SETCACHECOLOR(TTP_FRAME, RGB_TOOLTIP_FRAME)
		END_SELECT

		SELECT_PART(MLSO_OMBROWSER)
			SETCACHECOLOR(BP_BACKGROUND, RGB_OMBROWSER_BACKGROUND)
			SELECT_STATE(BP_TEXT)
				SETCACHECOLOR(BTS_NORMAL, RGB_OMBROWSER_TEXT)
				SETCACHECOLOR(BTS_DISABLED, RGB_OMBROWSER_TEXT_DISABLED)
			END_SELECT
			SELECT_STATE(BP_LINK)
				SETCACHECOLOR(BLS_NORMAL, RGB_OMBROWSER_LINK)
				SETCACHECOLOR(BLS_ACTIVE, RGB_OMBROWSER_LINK_ACTIVE)
				SETCACHECOLOR(BLS_VISITED, RGB_OMBROWSER_LINK_VISITED)
				SETCACHECOLOR(BLS_HOVER, RGB_OMBROWSER_LINK_HOVER)
			END_SELECT
		END_SELECT
	
	}
	return E_INVALIDARG;
}
