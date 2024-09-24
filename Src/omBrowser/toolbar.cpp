#include "main.h"
#include "./toolbar.h"
#include "./graphics.h"
#include "./resource.h"
#include "./toolbarItem.h"
#include "./toolbarStatic.h"
#include "./toolbarButton.h"
#include "./toolbarRating.h"
#include "./toolbarProgress.h"
#include "./toolbarAddress.h"
#include "../winamp/wa_dlg.h"
#include "../Plugins/General/gen_ml/ml_ipc_0313.h"
#include "./ifc_imageloader.h"
#include "./ifc_skinhelper.h"
#include "./ifc_omservice.h"
#include "./ifc_omservicecommand.h"
#include "./menu.h"
#include "./ifc_wasabihelper.h"

#include "./browserUiCommon.h"


#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <strsafe.h>
#include <vector>

#define TOOLBAR_SPACE_LEFT			0
#define TOOLBAR_SPACE_TOP			4
#define TOOLBAR_SPACE_RIGHT			1
#define TOOLBAR_SPACE_BOTTOM		3

#define TOOLBAR_HIDDENHEIGHT		3

#define TOOLBAR_ICONSTATE_NORMAL			0
#define TOOLBAR_ICONSTATE_HIGHLIGHTED	1
#define TOOLBAR_ICONSTATE_PRESSED		2
#define TOOLBAR_ICONSTATE_DISABLED		3
#define TOOLBAR_ICONSTATE_COUNT			4

#define TBS_MENULOOP			0x00000001
#define TBS_HIDDEN			0x00000002
#define TBS_HIDETIMERSET		0x00000004
#define TBS_NOFOCUSRECT		0x00000008

#define ICON_NONE				(-1)
#define ICON_CHEVRON			(-2)  // - resolved on the fly
#define ICON_SEPARATOR			0
#define ICON_CHEVRON_BOTTOM		1
#define ICON_CHEVRON_TOP		2
#define ICON_BACK				3
#define ICON_FORWARD				4
#define ICON_REFRESH			5
#define ICON_STOP				6
#define ICON_HOME				7
#define ICON_ERROR				8
#define ICON_LOCK				9
#define ICON_HISTORY			10
#define ICON_ADDRESSBAR			11

#define ICON_SEPARATOR_WIDTH	6
#define ICON_CHEVRON_WIDTH		13
#define ICON_HISTORY_WIDTH		9
#define ICON_ARROW_WIDTH		17

#define TOOLBRUSH_BACK		0
#define TOOLBRUSH_FRAME		1
#define TOOLBRUSH_ITEMBK	2
#define TOOLBRUSH_LAST		TOOLBRUSH_ITEMBK

#define ID_CHEVRON_CLICKED			1

#define TOOLBAR_BKCOLOR		WADLG_LISTHEADER_BGCOLOR
#define TOOLBAR_FGCOLOR		WADLG_LISTHEADER_FONTCOLOR

#define EDIT_ALPHA				50
#define TEXT_ALPHA				127
#define HIGHLIGHT_ALPHA			160
#define TIMER_AUTOHIDE_ID		14
#define TIMER_AUTOHIDE_DELAY		300

#define AUTOHIDE_ANIMATETIME		200

typedef std::vector<ToolbarItem*> ItemList;

typedef struct __TOOLBAR
{
	UINT		ref;
	UINT		flags;
	HBRUSH		szBrushes[TOOLBRUSH_LAST + 1];
	HIMAGELIST	imageList;
	HFONT		textFont;
	ItemList		*items;
	ToolbarItem *chevron;
	ToolbarItem *pressed;
	ToolbarItem *highlighted;
	UINT		resolveCache;
	HFONT		ownedFont;
	HWND			hTooltip;
	COLORREF	rgbBk;
	COLORREF	rgbFg;
	COLORREF	rgbText;
	COLORREF	rgbHilite;
	COLORREF	rgbFrame;
	COLORREF	rgbEdit;
	COLORREF	rgbEditBk;
	size_t		iFocused;
	HWND			hBrowser;
	TOOLBARTEXTMETRIC textMetric;
} TOOLBAR;

typedef struct __TOOLBARMOUSEHOOK
{
	HHOOK hHook;
	HWND	 hwnd;
} TOOLBARMOUSEHOOK;

static size_t tlsIndex = TLS_OUT_OF_INDEXES;

typedef ToolbarItem *(CALLBACK *TOOLBARITEMFACTORY)(ToolbarItem::Template * /*itemTemplate*/);

typedef struct __TOOLBARITEMCLASS
{
	LPCSTR name;
	TOOLBARITEMFACTORY creator;
	LPCWSTR text;
	LPCWSTR description;
	INT iconId;
	INT commandId;
	UINT style;
} TOOLBARITEMCLASS;

#define REGISTER_ITEM(__name, __creator, __textId, __descriptionId, __iconId, __commandId, __style)\
	{ (__name), (__creator), MAKEINTRESOURCE(__textId),\
	MAKEINTRESOURCE(__descriptionId), (__iconId), (__commandId), (__style) }

#define REGISTER_STATIC(__name, __textId, __descriptionId, __iconId, __style)\
	REGISTER_ITEM(__name, ToolbarStatic::CreateInstance, __textId, __descriptionId, __iconId, 0,  __style)

#define REGISTER_BUTTON(__name, __textId, __descriptionId, __iconId, __commandId, __style)\
	REGISTER_ITEM(__name, ToolbarButton::CreateInstance, __textId, __descriptionId, __iconId, __commandId,  __style)

#define REGISTER_RATING(__name, __textId, __descriptionId, __style)\
	REGISTER_ITEM(__name, ToolbarRating::CreateInstance, __textId, __descriptionId, ICON_NONE, 0, __style)

#define REGISTER_PROGRESS(__name, __textId, __descriptionId, __style)\
	REGISTER_ITEM(__name, ToolbarProgress::CreateInstance, __textId, __descriptionId, ICON_NONE, 0, __style)

#define REGISTER_ADDRESSBAR(__name, __style)\
	REGISTER_ITEM(__name, ToolbarAddress::CreateInstance, NULL, NULL, ICON_NONE, 0, __style)

const static TOOLBARITEMCLASS szRegisteredToolItems[] =
{
	REGISTER_STATIC(TOOLITEM_SEPARATOR, IDS_SEPARATOR, 0, ICON_SEPARATOR, ToolbarStatic::styleSeparator),
	REGISTER_STATIC(TOOLITEM_SPACE, IDS_SPACE, 0, ICON_NONE, ToolbarStatic::styleSpacer),
	REGISTER_STATIC(TOOLITEM_FLEXSPACE, IDS_FLEXSPACE, 0, ICON_NONE, ToolbarStatic::styleSpacer | ToolbarItem::styleFlexible),
	REGISTER_RATING(TOOLITEM_USERRATING, IDS_RATED, NULL, ToolbarItem::styleWantKey),
	REGISTER_PROGRESS(TOOLITEM_DOWNLOADPROGRESS, NULL, NULL, ToolbarItem::stateDisabled),
	REGISTER_ADDRESSBAR(TOOLITEM_ADDRESSBAR, ToolbarItem::styleFlexible | ToolbarItem::styleWantKey | ToolbarItem::styleTabstop),

	REGISTER_BUTTON(TOOLITEM_CHEVRON, IDS_MORE, IDS_MORE_DESCRIPTION, ICON_CHEVRON, 
					ID_CHEVRON_CLICKED, ToolbarItem::styleWantKey),
	REGISTER_BUTTON(TOOLITEM_BUTTON_HOME, IDS_HOME, IDS_HOME_DESCRIPTION, ICON_HOME,
					ID_NAVIGATION_HOME, ToolbarItem::styleWantKey),
	REGISTER_BUTTON(TOOLITEM_BUTTON_BACK, IDS_BACK, IDS_BACK_DESCRIPTION, ICON_BACK,
					ID_NAVIGATION_BACK, ToolbarItem::styleWantKey),
	REGISTER_BUTTON(TOOLITEM_BUTTON_FORWARD, IDS_FORWARD, IDS_FORWARD_DESCRIPTION, ICON_FORWARD,
					ID_NAVIGATION_FORWARD, ToolbarItem::styleWantKey),
	REGISTER_BUTTON(TOOLITEM_BUTTON_STOP, IDS_STOP, IDS_STOP_DESCRIPTION, ICON_STOP, 
					ID_NAVIGATION_STOP, ToolbarItem::styleWantKey),
	REGISTER_BUTTON(TOOLITEM_BUTTON_REFRESH, IDS_REFRESH, IDS_REFRESH_DESCRIPTION, ICON_REFRESH, 
					ID_NAVIGATION_REFRESH, ToolbarItem::styleWantKey),
	REGISTER_BUTTON(TOOLITEM_BUTTON_HISTORY, IDS_HISTORY, IDS_HISTORY_DESCRIPTION, ICON_HISTORY, 
					ID_NAVIGATION_HISTORY, ToolbarItem::styleWantKey),

	REGISTER_BUTTON(TOOLITEM_CMDLINK_INFO, IDS_SERVICE_GETINFO, IDS_SERVICE_GETINFO_DESCRIPTION, ICON_NONE, 
					ID_SERVICE_GETINFO, ToolbarItem::styleWantKey | ToolbarButton::styleCommandLink),
	REGISTER_BUTTON(TOOLITEM_CMDLINK_REPORT, IDS_SERVICE_REPORT, IDS_SERVICE_REPORT_DESCRIPTION, ICON_NONE, 
					ID_SERVICE_REPORT, ToolbarItem::styleWantKey | ToolbarButton::styleCommandLink),
	REGISTER_BUTTON(TOOLITEM_CMDLINK_UNSUBSCRIBE, IDS_SERVICE_UNSUBSCRIBE, IDS_SERVICE_UNSUBSCRIBE_DESCRIPTION, ICON_NONE, 
					ID_SERVICE_UNSUBSCRIBE, ToolbarItem::styleWantKey | ToolbarButton::styleCommandLink),
	
	REGISTER_BUTTON(TOOLITEM_BUTTON_SECURECONNECTION, IDS_SECURE_CONNECTION, IDS_SECURE_CONNECTION, ICON_LOCK, 
					ID_BROWSER_SECURECONNECTION, ToolbarItem::styleWantKey),

	REGISTER_BUTTON(TOOLITEM_BUTTON_SCRIPTERROR, IDS_SCRIPT_ERROR, IDS_SCRIPT_ERROR_DESCRIPTION, ICON_ERROR, 
					ID_BROWSER_SCRIPTERROR, ToolbarItem::styleWantKey),



};

#define GetToolbar(__hwnd) ((TOOLBAR*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))
static LRESULT CALLBACK Toolbar_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK Toolbar_MouseHook(INT code, WPARAM wParam, LPARAM lParam);


typedef struct __TOOLBARITEMINSERTREC
{
	LPCSTR	name;
	UINT	style;
} TOOLBARITEMINSERTREC;

BOOL Toolbar_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	ATOM klassAtom;
	ifc_wasabihelper *wasabi;

	if (GetClassInfo(hInstance, NWC_ONLINEMEDIATOOLBAR, &wc)) 
		return TRUE;

	ZeroMemory(&wc, sizeof(WNDCLASS));

	wc.hInstance		= hInstance;
	wc.lpszClassName	= NWC_ONLINEMEDIATOOLBAR;
	wc.lpfnWndProc	= Toolbar_WindowProc;
	wc.style			= 0;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.cbWndExtra	= sizeof(TOOLBAR*);
	
	klassAtom = RegisterClassW(&wc);
	if (0 == klassAtom)
		return FALSE;
	
	if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabi)))
	{
		api_application *application;
		if (SUCCEEDED(wasabi->GetApplicationApi(&application)))
		{
			application->DirectMouseWheel_RegisterSkipClass(klassAtom);
			application->Release();
		}
		wasabi->Release();
	}
	return TRUE;
}

static BOOL Toolbar_ClearItems(TOOLBAR *toolbar)
{
	if (NULL == toolbar) return FALSE;
	if (NULL == toolbar->items) return TRUE;

	size_t index = toolbar->items->size();

	while(index--)
	{
		ToolbarItem *item = toolbar->items->at(index);
		if (NULL != item) item->Release();
	}
	toolbar->items->clear();
	return TRUE;

}
static ULONG Toolbar_AddRef(TOOLBAR *toolbar)
{
	if (NULL == toolbar) return 0;
	return InterlockedIncrement((LONG*)&toolbar->ref); 
}

static ULONG Toolbar_Release(TOOLBAR *toolbar)
{
	if (0 == toolbar || 0 == toolbar->ref) return 0;
	LONG r = InterlockedDecrement((LONG*)&toolbar->ref);
	if (0 == r)
	{
		if (NULL != toolbar->hTooltip)
			DestroyWindow(toolbar->hTooltip);

		if (NULL != toolbar->chevron)
			toolbar->chevron->Release();

		if (NULL != toolbar->items)
		{
			Toolbar_ClearItems(toolbar);
			delete(toolbar->items);
		}

		if (NULL != toolbar->imageList)
			ImageList_Destroy(toolbar->imageList);

		if (NULL != toolbar->ownedFont)
			DeleteObject(toolbar->ownedFont);

		
		free(toolbar);
	}
	return r;
}

static const TOOLBARITEMCLASS *Toolbar_FindClass(LPCSTR pszName)
{
	if (NULL == pszName) return NULL;

	for (INT i =0; i < ARRAYSIZE(szRegisteredToolItems); i++)
	{
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, 
							szRegisteredToolItems[i].name, -1, pszName, -1))
		{
			return &szRegisteredToolItems[i];
		}
	}
	return NULL;
}

static ToolbarItem* Toolbar_CreateItem(LPCSTR pszName, UINT styleOverride)
{
	const TOOLBARITEMCLASS *iClass = Toolbar_FindClass(pszName);
	if (NULL == iClass || NULL == iClass->creator) 
		return NULL;

	ToolbarItem::Template iTemplate;
	ZeroMemory(&iTemplate, sizeof(ToolbarItem::Template));

	iTemplate.name		= iClass->name;
	iTemplate.text		= iClass->text;
	iTemplate.description= iClass->description;
	iTemplate.iconId		= iClass->iconId;
	iTemplate.commandId	= iClass->commandId;
	iTemplate.style		= iClass->style | styleOverride;
	return iClass->creator(&iTemplate);
}

static BOOL Toolbar_GetClientRect(HWND hwnd, RECT *prc)
{
	if (!GetClientRect(hwnd, prc))
		return FALSE;
	prc->left += TOOLBAR_SPACE_LEFT;
	prc->right -= TOOLBAR_SPACE_RIGHT;

	if (0 != (TBS_BOTTOMDOCK & GetWindowLongPtr(hwnd, GWL_STYLE)))
	{
		prc->top += TOOLBAR_SPACE_TOP;
		prc->bottom -= TOOLBAR_SPACE_BOTTOM;
	}
	else
	{
		prc->top += TOOLBAR_SPACE_BOTTOM;
		prc->bottom -= TOOLBAR_SPACE_TOP;
	}
	return TRUE;
}
static void Toolbar_UpdateColorTable(HWND hwnd, ifc_skinhelper *skinHelper)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	if (NULL == skinHelper || FAILED(skinHelper->GetColor(TOOLBAR_BKCOLOR, &toolbar->rgbBk)))
		toolbar->rgbBk = GetSysColor(COLOR_WINDOW);
	
	if (NULL == skinHelper || FAILED(skinHelper->GetColor(TOOLBAR_FGCOLOR, &toolbar->rgbFg)))
			toolbar->rgbFg = GetSysColor(COLOR_WINDOWTEXT);
		
	COLORREF rgbItemBk;
	if (NULL == skinHelper || FAILED(skinHelper->GetColor(WADLG_ITEMBG, &rgbItemBk)))
		rgbItemBk = GetSysColor(COLOR_WINDOW);

	COLORREF rgbItem;
	if (NULL == skinHelper || FAILED(skinHelper->GetColor(WADLG_ITEMFG, &rgbItem)))
		rgbItem = GetSysColor(COLOR_WINDOWTEXT);
		
	if (NULL != skinHelper)
	{
		const INT szFrameColors[] = 
		{	
			WADLG_HILITE,
			WADLG_LISTHEADER_FRAME_MIDDLECOLOR,
			WADLG_LISTHEADER_FRAME_BOTTOMCOLOR,
			WADLG_LISTHEADER_FRAME_TOPCOLOR,
		};

		for (INT i = 0; i < ARRAYSIZE(szFrameColors); i++)
		{		
			if (SUCCEEDED(skinHelper->GetColor(szFrameColors[i], &toolbar->rgbFrame)))
				toolbar->rgbFrame = GetSysColor(COLOR_GRAYTEXT);

			INT distance = GetColorDistance(toolbar->rgbFrame, rgbItemBk);
			if (distance < 0) distance = -distance; 
			if (distance >= 40)	break;
		}
	}
	else
		toolbar->rgbFrame = GetSysColor(COLOR_GRAYTEXT);

	toolbar->rgbEdit = BlendColors(toolbar->rgbBk, rgbItem, EDIT_ALPHA);
	toolbar->rgbEditBk = BlendColors(toolbar->rgbBk, rgbItemBk, EDIT_ALPHA);
	toolbar->rgbText = BlendColors(toolbar->rgbFg, toolbar->rgbBk, TEXT_ALPHA);
	toolbar->rgbHilite = BlendColors(toolbar->rgbFg, toolbar->rgbBk, HIGHLIGHT_ALPHA);

}
static void Toolbar_UpdateTextMetrics(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	ZeroMemory(&toolbar->textMetric, sizeof(TOOLBARTEXTMETRIC));

	RECT clientRect;
	if (!Toolbar_GetClientRect(hwnd, &clientRect))
		return;

	INT iconCX, iconCY;
	if (NULL == toolbar->imageList || 
		FALSE == ImageList_GetIconSize(toolbar->imageList, &iconCX, &iconCY))
	{
		iconCX = 0;
		iconCY = 0;
	}
	else
	{
		iconCY = iconCY / TOOLBAR_ICONSTATE_COUNT;
	}


	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL == hdc) return;

	HFONT originalFont = (HFONT)SelectObject(hdc, toolbar->textFont);

	TEXTMETRIC tm;
	if (GetTextMetrics(hdc, &tm))
	{
		toolbar->textMetric.height = tm.tmHeight;
		toolbar->textMetric.aveCharWidth = tm.tmAveCharWidth;
		toolbar->textMetric.overhang = tm.tmOverhang;

		INT clientHeight = clientRect.bottom - clientRect.top;
		if (tm.tmHeight >= iconCY)
		{
			toolbar->textMetric.baseY = tm.tmAscent - tm.tmInternalLeading;
			INT t = clientHeight - toolbar->textMetric.baseY;
			toolbar->textMetric.origY = clientRect.top + t/2;
			if (0 != t%2) 
			{
				toolbar->textMetric.origY += 1;
				toolbar->textMetric.baseY += 1;
			}
		}
		else
		{
			toolbar->textMetric.baseY  = (clientHeight - iconCY)/2;
			if (tm.tmDescent > toolbar->textMetric.baseY) 
				toolbar->textMetric.baseY  = tm.tmDescent;
			//toolbar->textMetric.baseY += 8;
			toolbar->textMetric.baseY = clientHeight - toolbar->textMetric.baseY;
			toolbar->textMetric.origY = (clientRect.top + toolbar->textMetric.baseY + tm.tmDescent) - tm.tmHeight;
			toolbar->textMetric.baseY -= toolbar->textMetric.origY;
		}
	}

	SelectObject(hdc, originalFont);
	ReleaseDC(hwnd, hdc);
}
static HIMAGELIST Toolbar_LoadImagelist(TOOLBAR *toolbar, HINSTANCE hInstance, LPCWSTR pszPath, INT iconWidth, INT iconStatesCount)
{
	if (iconWidth <= 0 || iconStatesCount <= 0 || NULL == pszPath)
		return NULL;
	
	HBITMAP bitmap = NULL;

	ifc_omimageloader *loader;
	if (SUCCEEDED(Plugin_QueryImageLoader(hInstance, pszPath, FALSE, &loader)))
	{
		loader->LoadBitmap(&bitmap, NULL, NULL);
		loader->Release();
	}
	
	DIBSECTION dib;
	if (NULL == bitmap ||
		sizeof(DIBSECTION) != GetObject(bitmap, sizeof(DIBSECTION), &dib))
	{
		if (NULL != bitmap) 	DeleteObject(bitmap);
		return NULL;
	}

	if (dib.dsBm.bmHeight < 0) dib.dsBm.bmHeight = -dib.dsBm.bmHeight;

	Image_Colorize((BYTE*)dib.dsBm.bmBits, dib.dsBm.bmWidth, dib.dsBm.bmHeight, dib.dsBm.bmBitsPixel,
					ColorAdjustLuma(toolbar->rgbBk, -140, TRUE), toolbar->rgbFg, FALSE);

	Image_BlendOnColorEx((BYTE*)dib.dsBm.bmBits, dib.dsBm.bmWidth, dib.dsBm.bmHeight, 
					0, 0, dib.dsBm.bmWidth, dib.dsBm.bmHeight, dib.dsBm.bmBitsPixel, FALSE, toolbar->rgbBk);

	
	if (iconWidth > dib.dsBm.bmWidth) iconWidth = dib.dsBm.bmWidth;

	INT iconHeight = dib.dsBm.bmHeight / iconStatesCount;
	INT initial = dib.dsBm.bmWidth / iconWidth;

	HIMAGELIST himl = ImageList_Create(iconWidth, iconHeight * iconStatesCount, ILC_COLOR24, initial, 1); 
	if (NULL != himl)
	{
		INT index = ImageList_Add(himl, bitmap, NULL);
		if (-1 == index)
		{
			ImageList_Destroy(himl);
			himl = NULL;
		}
	}
	DeleteObject(bitmap);
	return himl;
}

static LONG Toolbar_ShowChevron(HWND hwnd, const RECT *prcClient) // return client cx change
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL != toolbar && NULL != toolbar->chevron && toolbar->chevron->IsRectEmpty())
	{
		RECT chevronRect;
		CopyRect(&chevronRect, prcClient); 
		if (toolbar->chevron->AdjustRect(hwnd, &chevronRect))
		{
			chevronRect.left = prcClient->right - (chevronRect.right - chevronRect.left);
			chevronRect.right = prcClient->right;
			if (toolbar->chevron->SetRect(&chevronRect))
				return (chevronRect.left - prcClient->right);
		}
	}
	return 0;
}

static ToolbarItem *Toolbar_GetItem(TOOLBAR *toolbar, size_t index)
{
	if (NULL == toolbar || NULL == toolbar->items) 
		return NULL;
	
	size_t count = toolbar->items->size();
	
	if (index < count) 
		return toolbar->items->at(index);
	if (index == count) 
		return toolbar->chevron;

	return NULL;
}

static BOOL Toolbar_IsItemAcceptFocus(ToolbarItem *item, BOOL fTabstopOnly)
{
	if (NULL == item) 
		return FALSE;
	
	UINT itemStyle = item->GetStyle();
	if (0 != ((ToolbarItem::stateDisabled | ToolbarItem::stateHidden) & itemStyle))
		return FALSE;

	UINT mask = ToolbarItem::styleTabstop;
	if (FALSE == fTabstopOnly) mask |= ToolbarItem::styleWantKey;
	if (0 == (mask & itemStyle))
		return FALSE;

	return (FALSE == item->IsRectEmpty());
}

static void Toolbar_UpdateTabstop(HWND hwnd)
{
	BOOL fEnable = FALSE;
	UINT windowStyle = GetWindowStyle(hwnd);

	if (0 != (TBS_TABSTOP & windowStyle))
	{
		fEnable = TRUE;
	}
	else
	{
		TOOLBAR *toolbar = GetToolbar(hwnd);
		if (NULL != toolbar && NULL != toolbar->items)
		{
			size_t index = toolbar->items->size();
			while(index--)
			{
				if (FALSE != Toolbar_IsItemAcceptFocus(toolbar->items->at(index), TRUE))
				{
					fEnable = TRUE;
					break;
				}
			}
		}
	}

	if (FALSE != fEnable)
	{
		if (0 == (WS_TABSTOP & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_TABSTOP);
	}
	else
	{
		if (0 != (WS_TABSTOP & windowStyle))
		{
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_TABSTOP);
		}
	}
}
static INT Toolbar_InsertItemInternal(TOOLBAR *toolbar, ToolbarItem *item, INT insertBefore, HWND hwnd)
{
	if (NULL == toolbar || NULL == toolbar->items || NULL == item)
		return ITEM_ERR;
	
	INT index = ITEM_ERR;

	if (TBIP_FIRST == insertBefore) 
	{
		toolbar->items->insert(toolbar->items->begin(), item);
		index = 0;
	}
	else if (TBIP_LAST == insertBefore)
	{
		toolbar->items->push_back(item);
		index = ((INT)toolbar->items->size() - 1);
	}
	else
	{
		if (insertBefore < 0) insertBefore = 0;
		else if (insertBefore >= (INT)toolbar->items->size()) insertBefore = (INT)toolbar->items->size();
		toolbar->items->insert(toolbar->items->begin() + insertBefore, item);
		index = insertBefore;
	}

	if (ITEM_ERR != index)
	{
		item->AddRef();
		item->UpdateSkin(hwnd);
	}

	return index;
}

static size_t Toolbar_GetFocusIndex(HWND hwnd, size_t focusIndex, INT searchDirection, BOOL fTabstopOnly)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar || NULL == toolbar->items || 0 == toolbar->items->size())
		return ((size_t)-1);
	

	ToolbarItem *item = Toolbar_GetItem(toolbar, focusIndex);
	if (NULL != item && Toolbar_IsItemAcceptFocus(item, fTabstopOnly))
		return focusIndex;
	
	if (0 == searchDirection)
		return ((size_t)-1);
	
	if (searchDirection > 0)
	{
		size_t count = toolbar->items->size();
		size_t i = focusIndex + 1;
		for (; i < count; i++)
		{
			if (Toolbar_IsItemAcceptFocus(toolbar->items->at(i), fTabstopOnly))
				return i;
		}
		if (i == count && Toolbar_IsItemAcceptFocus(toolbar->chevron, fTabstopOnly))
			return i;
	}
	else
	{
		size_t count = toolbar->items->size();
		if (focusIndex == count && Toolbar_IsItemAcceptFocus(toolbar->chevron, fTabstopOnly))
			return count;

		size_t i = focusIndex;
		if (i <= count)
		{
			while(i--)
			{
				if (Toolbar_IsItemAcceptFocus(toolbar->items->at(i), fTabstopOnly))
					return i;
			}
		}
	}

	return ((size_t)-1);
}

static void Toolbar_OffsetItems(ToolbarItem **itemList, INT count, INT offsetX, INT offsetY)
{
	while(count-- > 0)
	{
		if (NULL != itemList[count])
			itemList[count]->OffsetRect(offsetX, offsetY);
	}
}
static void Toolbar_HideItems(ToolbarItem **itemList, INT count, INT *chevronItemCount)
{
	INT chevronItems = 0;
	while(count-- > 0)
	{
		if (NULL != itemList[count])
		{
			if (0 == (ToolbarItem::styleNoChevron & itemList[count]->GetStyle()))
				chevronItems++;
			itemList[count]->SetRectEmpty();
		}
	}

	if (NULL != chevronItemCount)
		*chevronItemCount = chevronItems;
}

static LONG Toolbar_CutoffItems(ToolbarItem **itemList, INT count, LONG cutoffCX)
{
	RECT elementRect;
	LONG left = cutoffCX;
	ToolbarItem *item;
	while(count-- > 0)
	{
		item = itemList[count];
		if (NULL == item)
			continue;

		DWORD style = item->GetStyle();
		if (0 == (ToolbarItem::styleChevronOnly & style) &&
			(0 == (ToolbarItem::stateHidden & style) || 0 == (ToolbarItem::stylePopup & style)) &&
			item->GetRect(&elementRect))
		{
			if (elementRect.right <= cutoffCX)
				return elementRect.right;
			
			if (0 == count) 
				left = elementRect.left;

			item->SetRectEmpty();
		}
	}
	return left;
}

static LONG Toolbar_CalculateClient(HWND hwnd, ToolbarItem **itemList, INT count, const RECT *prcClient, INT *deltaCX)
{	
	ToolbarItem *item;
	RECT elementRect;
	INT index;
	UINT style;
	LONG elementLeft = prcClient->left;

	for (index = 0; index < count; index++)
	{		
		item = itemList[index];
		if (NULL != item)
		{
			style = item->GetStyle();
			if (0 != (ToolbarItem::styleChevronOnly & style) ||
				(0 != (ToolbarItem::stateHidden & style) && 0!= (ToolbarItem::stylePopup & style)))
			{
				item->SetRectEmpty();
				continue;
			}
			
			CopyRect(&elementRect, prcClient);
			elementRect.left = elementLeft;

			if (0 != (ToolbarItem::styleFlexible & style))
			{	
				RECT testRect;
				INT flexMinimum = 0;
				SetRect(&testRect, elementRect.left, elementRect.top, elementRect.left, elementRect.bottom);
				if (item->AdjustRect(hwnd, &testRect) && testRect.right > elementRect.left)
				{
					flexMinimum = (testRect.right - elementRect.left);
				}

				if (index != (count -1))
				{
					INT delta = 0;
					INT section;
					
					if ((elementRect.left + flexMinimum) < prcClient->right)
					{
						CopyRect(&testRect, &elementRect);
						testRect.left += flexMinimum;

						section = Toolbar_CalculateClient(hwnd, itemList + (index + 1), 
											count - index - 1, &testRect, &delta);
					}
					else
					{
						section = prcClient->right - (elementRect.left + flexMinimum);
					}

					if (section < 0)
					{ // we need to move back
						item->SetRectEmpty();
						elementLeft += (flexMinimum - section);
						if (index > 0)
							elementLeft = Toolbar_CutoffItems(itemList, index, elementLeft);
						return (elementLeft - prcClient->left);
					}

					if (NULL != deltaCX)
						*deltaCX += delta;
					
					elementRect.right = elementRect.right - section + delta;
					if (elementRect.right < elementRect.left)
						elementRect.right = elementRect.left;
				}

				if (item->AdjustRect(hwnd, &elementRect))
				{	
					item->SetRect(&elementRect);
					Toolbar_OffsetItems(itemList + (index + 1), count - index - 1, elementRect.right - elementRect.left - flexMinimum, 0);
					elementLeft = prcClient->right;
					break;
				}
			}
			
			if (item->AdjustRect(hwnd, &elementRect))
			{
				if (elementRect.right > prcClient->right)
				{ 
					INT chevronItems;
					Toolbar_HideItems(itemList + index, count - index, &chevronItems); 

					if (chevronItems > 0)
					{
						LONG delta = Toolbar_ShowChevron(hwnd, prcClient);
						if (NULL != deltaCX) *deltaCX += delta;
						elementLeft = (index > 0) ? 
										Toolbar_CutoffItems(itemList, index, prcClient->right + delta) :
										(prcClient->left + delta);
					}
					break;
				}
				
				item->SetRect(&elementRect);
				elementLeft += (elementRect.right - elementRect.left);
			}
		}
	}
	return (elementLeft - prcClient->left);
}


static void Toolbar_UpdateLayout(HWND hwnd, BOOL fRedraw)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	if (0 != (TBS_LOCKUPDATE & windowStyle)) return;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	RECT clientRect;
	if (!Toolbar_GetClientRect(hwnd, &clientRect)) return;

	Toolbar_UpdateTextMetrics(hwnd);
		
	size_t count = (NULL != toolbar->items) ? toolbar->items->size() : 0;

	if (NULL != toolbar->chevron)
	{
		INT chevronOnly = 0;
		ToolbarItem *item;
		for (size_t i = 0;  i < count; i++)
		{
			item = toolbar->items->at(i);
			UINT style = item->GetStyle();

			if (NULL != item && 
				0 != (ToolbarItem::styleChevronOnly & style) &&
				0 == (ToolbarItem::stateHidden & style))
			{
				chevronOnly++;
			}
		}

		toolbar->chevron->SetRectEmpty();
		if (0 != chevronOnly)
			clientRect.right += Toolbar_ShowChevron(hwnd, &clientRect);
	}

	
	Toolbar_CalculateClient(hwnd, toolbar->items->size() ? &toolbar->items->at(0) : nullptr, (INT)count, &clientRect, NULL);

	if (((size_t)-1) != toolbar->iFocused)
	{
		size_t focusIndex = Toolbar_GetFocusIndex(hwnd, toolbar->iFocused, -1, FALSE); 
		if (focusIndex != toolbar->iFocused)
		{
			ToolbarItem *itemNew, *itemOld;
			
			itemOld = Toolbar_GetItem(toolbar, toolbar->iFocused);
			itemNew = Toolbar_GetItem(toolbar, focusIndex);
			toolbar->iFocused = focusIndex;

			if (NULL != itemOld) 
				itemOld->SetFocus(hwnd, itemNew, FALSE);
			if (NULL != itemNew) 
				itemNew->SetFocus(hwnd, itemOld, TRUE);
		}
	}

	if (FALSE != fRedraw)
	{
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_NOERASE);
	}
}

static ToolbarItem *Toolbar_HitTest(HWND hwnd, POINT pt)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
    if (NULL != toolbar)
	{
		ToolbarItem *item;
		size_t count = (NULL != toolbar->items) ? toolbar->items->size() : 0;
		for (size_t i = 0; i <= count; i++)
		{
			item = (i < count) ? toolbar->items->at(i) : toolbar->chevron;

			if (NULL != item && item->PtInItem(pt))
				return item;
		}
	}
	return NULL;
}
static HFONT Toolbar_CreateFont(HFONT skinFont)
{
	LOGFONT lf;
	if (NULL == skinFont) return NULL;

	INT skinHeight = (sizeof(LOGFONT) == GetObject(skinFont, sizeof(LOGFONT), &lf)) ? lf.lfHeight : -11;
	ZeroMemory(&lf, sizeof(LOGFONT));
		
	lf.lfHeight = skinHeight;
	lf.lfWeight = FW_DONTCARE;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), L"MS Shell Dlg 2");

	return CreateFontIndirect(&lf);
}


static HWND Toolbar_CreateTooltip(HINSTANCE hInstance, HWND hOwner)
{
	HWND hTooltip = CreateWindowExW(WS_EX_TRANSPARENT , TOOLTIPS_CLASS, NULL,
								WS_POPUP | TTS_ALWAYSTIP,
								0, 0, 0, 0, 
								hOwner, NULL, hInstance, NULL);

	if (NULL == hTooltip)
		return NULL;

	SetWindowPos(hTooltip, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	SendMessage(hTooltip, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELPARAM(1000, 0)); 
	SendMessage(hTooltip, TTM_SETDELAYTIME, TTDT_RESHOW, MAKELPARAM(-2, 0)); 
		
	OSVERSIONINFO ov;
	ov.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (::GetVersionEx(&ov) && 
		ov.dwMajorVersion >= 6 && 
		VER_PLATFORM_WIN32_NT == ov.dwPlatformId)
	{
		RECT rcMargin;
		SetRect(&rcMargin, 3, 1, 3, 1);
		SendMessage(hTooltip, TTM_SETMARGIN, 0, (LPARAM)&rcMargin); 
	}

	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hOwner;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage(hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);

	SendMessage(hTooltip, TTM_SETMAXTIPWIDTH, 0, 1000);

	return hTooltip;
}
static void Toolbar_RelayTooltipMsg(HWND hTooltip, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	
	msg.hwnd = hwnd;
	msg.message = uMsg;
	msg.wParam = wParam;
	msg.lParam = lParam;

	//DWORD pts = GetMessagePos();
	//POINTSTOPOINT(msg.pt, pts);
	//msg.time = GetMessageTime();

	SendMessage(hTooltip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
}
static void Toolbar_SetTooltipItem(HWND hwnd, ToolbarItem *item)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	TOOLINFO ti;

	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hwnd;
	ti.uId = 0;
	ti.lpszText = NULL;

	if (SendMessage(toolbar->hTooltip, TTM_GETTOOLINFO, 0, (LPARAM)&ti))
	{
		RECT clientRect, toolRect;
		Toolbar_GetClientRect(hwnd, &clientRect);

		if (NULL == item || FALSE == item->GetRect(&toolRect))
		{
			SendMessage(toolbar->hTooltip, TTM_ACTIVATE, FALSE, 0L);
			return;
		}

		toolRect.top = clientRect.top;
		toolRect.bottom = clientRect.bottom;

		if (ti.lParam != (LPARAM)item || !::EqualRect(&ti.rect, &toolRect))
		{
			CopyRect(&ti.rect, &toolRect);
			ti.lParam = (LPARAM)item;
			SendMessage(toolbar->hTooltip, TTM_SETTOOLINFO, 0, (LPARAM)&ti);
		}
		SendMessage(toolbar->hTooltip, TTM_ACTIVATE, TRUE, 0L);
	}

}
static void Toolbar_ClearBrushCache(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	for (INT i = 0; i < ARRAYSIZE(toolbar->szBrushes); i++)
	{
		if (NULL != toolbar->szBrushes[i])
		{
			DeleteObject(toolbar->szBrushes[i]);
			toolbar->szBrushes[i] = NULL;
		}
	}
}

static HBRUSH Toolbar_GetBrush(HWND hwnd, UINT brushId)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return NULL;

	if (brushId >= ARRAYSIZE(toolbar->szBrushes))
		return NULL;

	if (NULL == toolbar->szBrushes[brushId])
	{
		COLORREF brushColor = 0x00FF00FF;
		switch(brushId)
		{
			case TOOLBRUSH_BACK:	brushColor = toolbar->rgbBk;break;
			case TOOLBRUSH_FRAME:	brushColor = toolbar->rgbFrame;	break;
			case TOOLBRUSH_ITEMBK:	brushColor = toolbar->rgbEditBk; break;
		}
		toolbar->szBrushes[brushId] = CreateSolidBrush(brushColor);
	}
	return toolbar->szBrushes[brushId];
}


static HRGN Toolbar_GetFrameRgn(const RECT *prcToolbar, BOOL bottomDock)
{
	if (TOOLBAR_SPACE_TOP < 1) 
		return NULL;

	HRGN regionFrame;
	INT origY = (FALSE != bottomDock) ? prcToolbar->top : prcToolbar->bottom - 1;
	regionFrame = CreateRectRgn(prcToolbar->left, origY, prcToolbar->right, origY + 1);
	return regionFrame;
}

static void Toolbar_PaintBack(HWND hwnd, HDC hdc, const RECT *prcToolbar, HRGN regionPaint)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;
	
	UINT windowStyle = GetWindowStyle(hwnd);

	if (0 == (TBS_HIDDEN & toolbar->flags))
	{
		HRGN regionFrame = Toolbar_GetFrameRgn(prcToolbar, (0 != (TBS_BOTTOMDOCK & windowStyle)));
		if (NULL != regionFrame)
		{		
			if (FillRgn(hdc, regionFrame, Toolbar_GetBrush(hwnd, TOOLBRUSH_FRAME)))
				CombineRgn(regionPaint, regionPaint, regionFrame, RGN_DIFF);
			DeleteObject(regionFrame);
		}
	}

	FillRgn(hdc, regionPaint, Toolbar_GetBrush(hwnd, TOOLBRUSH_BACK));
}



static void Toolbar_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	
	DWORD windowStyle = GetWindowStyle(hwnd);

	HRGN paintRegion = CreateRectRgnIndirect(prcPaint);
	
	if (0 == (TBS_HIDDEN & toolbar->flags))
	{
		INT savedDC = SaveDC(hdc);
		
		HRGN itemRegion = CreateRectRgn(0,0,0,0);
	
		SetBkColor(hdc, toolbar->rgbBk);
		SetTextColor(hdc, toolbar->rgbText);
		SelectObject(hdc, toolbar->textFont);
		
		ToolbarItem *item;
		RECT paintRect;
		UINT style;

		BOOL fFocused = (GetFocus() == hwnd);
		UINT commonState= 0;

		if (0 != (WS_DISABLED & windowStyle))
			commonState |= ToolbarItem::stateDisabled;

		size_t count = (NULL != toolbar->items) ? toolbar->items->size() : 0;
		for (size_t i = 0; i <= count; i++)
		{
			item = (i < count) ? toolbar->items->at(i) : toolbar->chevron;

			if (NULL != item && item->IntersectRect(&paintRect, prcPaint))
			{
				style = item->GetStyle() | commonState;
				if (i == toolbar->iFocused && fFocused)
					style |= ToolbarItem::stateFocused;
				
				if (0 != (TBS_NOFOCUSRECT & toolbar->flags))
					style |= ToolbarItem::stateNoFocusRect;

				if (0 == (ToolbarItem::stateHidden & style) &&
					item->Paint(hwnd, hdc, &paintRect, style))
				{
					SetRectRgn(itemRegion, 	paintRect.left, paintRect.top, paintRect.right, paintRect.bottom);
					CombineRgn(paintRegion, paintRegion, itemRegion, RGN_DIFF);
				}
			}
		}
		DeleteObject(itemRegion);
		RestoreDC(hdc, savedDC);
	}

	Toolbar_PaintBack(hwnd, hdc, &clientRect, paintRegion);
	DeleteObject(paintRegion);

}

static INT Toolbar_FindListItem(ToolbarItem **itemList, INT start,  INT count, LPCSTR pszName, INT cchName)
{
	ToolbarItem *item;
	for (INT i = start; i < count; i++)
	{
		item = itemList[i];
		if (NULL != item && item->IsEqual(pszName, cchName))
		{
			return i;
		}
	}
	return ITEM_ERR;
}
static INT Toolbar_ResolveName(HWND hwnd, LPCSTR pszName)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar || NULL == toolbar->items) 
		return ITEM_ERR;

	UINT index;
	UINT count = (UINT)toolbar->items->size();
	
	if (IS_INTRESOURCE(pszName))
	{
		index = (UINT)(UINT_PTR)pszName;
		
		if (index == count && NULL != toolbar->chevron)
			return index;

		return (index < count) ? index : ITEM_ERR;
	}
	
	INT cchName = lstrlenA(pszName);
	if (0 == cchName) return ITEM_ERR;
	
	if (count == toolbar->resolveCache)
	{
		if (toolbar->chevron->IsEqual(pszName, cchName))
			index = count;
		else
		{
			index = ITEM_ERR;
			toolbar->resolveCache = 0;
		}
	}
	else
	{
		index = ITEM_ERR;
	}

	if (ITEM_ERR == index)
		index = Toolbar_FindListItem(toolbar->items->size() ? & toolbar->items->at(0) : nullptr, toolbar->resolveCache, count, pszName, cchName);
	
	if (ITEM_ERR == index && 0 != toolbar->resolveCache)
		index = Toolbar_FindListItem(toolbar->items->size() ? &toolbar->items->at(0) : nullptr, 0, toolbar->resolveCache, pszName, cchName);
	
	if (ITEM_ERR == index && NULL != toolbar->chevron && toolbar->chevron->IsEqual(pszName, cchName))
		index = count;
	

	toolbar->resolveCache = (ITEM_ERR != index) ? index : 0;
	return index;
}

static BOOL Toolbar_DisplayChevronMenu(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar || NULL == toolbar->items) return FALSE;

	HMENU hMenu = CreatePopupMenu();
	if (NULL == hMenu) return FALSE;

	size_t count = toolbar->items->size();
	UINT insertedCount = 0;
	MENUITEMINFO mi;
	mi.cbSize = sizeof(MENUITEMINFO);
	
	UINT style;
	WCHAR szBuffer[80] = {0};
	INT commandId = 0;
	ToolbarItem *item;
	BOOL insertBreak = FALSE;
	for (size_t i = 0; i < count; i++)
	{
		item = toolbar->items->at(i);
		if (NULL != item)
		{			
			style = item->GetStyle();
			if(0 == ((ToolbarItem::stateHidden | ToolbarItem::styleNoChevron) & style) &&
				item->IsRectEmpty() &&
				FALSE != item->FillMenuInfo(hwnd, &mi, szBuffer, ARRAYSIZE(szBuffer)))
			{						
				if (MIIM_FTYPE == mi.fMask && MFT_MENUBREAK == mi.fType)
				{
					if (insertedCount > 0)
						insertBreak = TRUE;
				}
				else
				{
					if (FALSE != InsertMenuItem(hMenu, insertedCount, TRUE, &mi)) 
					{
						if (insertBreak)
						{
							mi.fMask = MIIM_FTYPE;
							mi.fType = MFT_MENUBREAK;
							if (InsertMenuItem(hMenu, insertedCount, TRUE, &mi))
								insertedCount++;
							insertBreak = FALSE;
						}
						insertedCount++;
					}
				}
			
			}
		}
	}

	if (NULL != hMenu && insertedCount > 0)
	{
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);
		UINT windowStyle = GetWindowStyle(hwnd);
		UINT menuStyle = TPM_RIGHTALIGN | TPM_RETURNCMD;
		POINT menuOrig;
		menuOrig.x = windowRect.right;
		if (0 != (TBS_BOTTOMDOCK & windowStyle))
		{
			menuOrig.y = windowRect.top;
			menuStyle |= (TPM_BOTTOMALIGN | TPM_VERNEGANIMATION);
		}
		else
		{
			menuOrig.y = windowRect.bottom;
			menuStyle |= (TPM_TOPALIGN | TPM_VERPOSANIMATION);
		}

		commandId = Menu_TrackPopup(hMenu, menuStyle, menuOrig.x, menuOrig.y, hwnd, NULL);
		

	}
	
	DestroyMenu(hMenu);

	if (0 != commandId)
		Toolbar_SendCommand(hwnd, commandId);
	

	return TRUE;
}

static void Toolbar_DisplayContextMenu(HWND hwnd, INT x, INT y)
{
	HMENU hMenu = Menu_GetMenu(MENU_TOOLBAR, 0);
	if (NULL != hMenu)
	{
		UINT windowStyle = GetWindowStyle(hwnd);
		CheckMenuRadioItem(hMenu, ID_TOOLBAR_DOCKTOP, ID_TOOLBAR_DOCKBOTTOM,
				(0 != (TBS_BOTTOMDOCK & windowStyle)) ? ID_TOOLBAR_DOCKBOTTOM :ID_TOOLBAR_DOCKTOP,
				MF_BYCOMMAND);

		CheckMenuItem(hMenu, ID_TOOLBAR_AUTOHIDE, 
			MF_BYCOMMAND | ((0 != (TBS_AUTOHIDE & windowStyle)) ? MF_CHECKED :MF_UNCHECKED));

		CheckMenuItem(hMenu, ID_TOOLBAR_TABSTOP, 
			MF_BYCOMMAND | ((0 != (TBS_TABSTOP & windowStyle)) ? MF_CHECKED :MF_UNCHECKED));
	
		Menu_TrackPopup(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, hwnd, NULL);

		Menu_ReleaseMenu(hMenu, MENU_TOOLBAR);
	}

}

static BOOL Toolbar_InstallMouseHook(HWND hwnd)
{
	if (TLS_OUT_OF_INDEXES == tlsIndex)
	{
		tlsIndex = Plugin_TlsAlloc();
		if (TLS_OUT_OF_INDEXES == tlsIndex) return FALSE;
	}

	TOOLBARMOUSEHOOK *hook = (TOOLBARMOUSEHOOK*)calloc(1, sizeof(TOOLBARMOUSEHOOK));
	if (NULL == hook) return FALSE;
	
	hook->hwnd = hwnd;
	hook->hHook = SetWindowsHookEx(WH_MOUSE, Toolbar_MouseHook, NULL, GetCurrentThreadId());
	if (NULL == hook->hHook)
	{
		free(hook);
		return FALSE;
	}
	
	Plugin_TlsSetValue(tlsIndex, hook); 
	return TRUE;
}

static void Toolbar_TrackMouseLeave(HWND hwnd)
{
	TRACKMOUSEEVENT tm;
	tm.cbSize = sizeof(TRACKMOUSEEVENT);
	tm.dwFlags = TME_QUERY;
	tm.hwndTrack = hwnd;
	if (TrackMouseEvent(&tm) && 0 == (TME_LEAVE & tm.dwFlags))
	{
		tm.cbSize = sizeof(TRACKMOUSEEVENT);
		tm.dwFlags = TME_LEAVE;
		tm.hwndTrack = hwnd;
		TrackMouseEvent(&tm);
	}
}

static BOOL Toolbar_RemoveMouseHook(HWND hwnd, BOOL fOnlyMine)
{
	if (TLS_OUT_OF_INDEXES == tlsIndex) return FALSE;

	TOOLBARMOUSEHOOK *hook = (TOOLBARMOUSEHOOK*)Plugin_TlsGetValue(tlsIndex);
    if (NULL == hook) return FALSE;
	
	if (FALSE != fOnlyMine && hwnd != hook->hwnd)
		return FALSE;

	Plugin_TlsSetValue(tlsIndex, NULL); 
	
	if (NULL != hook->hHook) 
		UnhookWindowsHookEx(hook->hHook);
	
	if (NULL != hook->hwnd && hwnd != hook->hwnd)
	{
		Toolbar_TrackMouseLeave(hook->hwnd);
	}

	free(hook);
	return TRUE;
}

static void Toolbar_AutoShowWindow(HWND hwnd)
{
	KillTimer(hwnd, TIMER_AUTOHIDE_ID);

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	HWND hFocus = GetFocus();
	
	BOOL fFocused = (hFocus == hwnd);

	UINT windowStyle = GetWindowStyle(hwnd);
	if ((TBS_AUTOHIDE | WS_VISIBLE) != ((TBS_AUTOHIDE | WS_VISIBLE) & windowStyle)) 
		return;

    if (0 == (TBS_HIDDEN & toolbar->flags))
	{
		if (0 != (TBS_HIDETIMERSET & toolbar->flags))
			toolbar->flags &= ~TBS_HIDETIMERSET;
		
		if (FALSE == fFocused)
			Toolbar_TrackMouseLeave(hwnd);
		return;
	}
	
	Toolbar_RemoveMouseHook(hwnd, FALSE);

	if (FALSE == fFocused && FALSE == Toolbar_InstallMouseHook(hwnd))
		return;

	toolbar->flags &= ~TBS_HIDDEN;	

	RECT windowRect;
	if (GetWindowRect(hwnd, &windowRect))
	{			
		HWND hParent = GetParent(hwnd);
		if(NULL != hParent)
		{
			MapWindowPoints(HWND_DESKTOP, hParent, (POINT*)&windowRect, 2);
			INT height = Toolbar_GetIdealHeight(hwnd);
			INT x = windowRect.left;
			INT y = windowRect.top;
			UINT swpFlags = SWP_NOMOVE | SWP_NOREDRAW;
			UINT animateFlags = AW_SLIDE | AW_VER_POSITIVE;
			
			if (0 != (TBS_BOTTOMDOCK & windowStyle))
			{
				y = windowRect.bottom - height;
				swpFlags &= ~SWP_NOMOVE;
				animateFlags =  AW_SLIDE | AW_VER_NEGATIVE;
			}
			SetWindowPos(hwnd, HWND_TOP, x, y, windowRect.right - windowRect.left, height, swpFlags);
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
			BOOL result = AnimateWindow(hwnd, AUTOHIDE_ANIMATETIME,  animateFlags);
			if (!result)
			{
				SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
		}			
	}


	if (FALSE == fFocused)
		Toolbar_TrackMouseLeave(hwnd);
}

static void CALLBACK Toolbar_AutoHideTimerProc(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD time)
{	
	KillTimer(hwnd, eventId);

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	UINT windowStyle = GetWindowStyle(hwnd);
	if (0 == (TBS_AUTOHIDE & windowStyle)) return;

	toolbar->flags &= ~TBS_HIDETIMERSET;
	if (0 != ((TBS_HIDDEN | TBS_MENULOOP) & toolbar->flags))
		return;
	
	HWND hFocus = ::GetFocus();
	if (hwnd == hFocus || IsChild(hwnd, hFocus))
		return;

	Toolbar_RemoveMouseHook(hwnd, TRUE);
	RECT windowRect;
	if (GetWindowRect(hwnd, &windowRect))
	{			
		HWND hParent = GetParent(hwnd);
		if(NULL != hParent)
		{
			MapWindowPoints(HWND_DESKTOP, hParent, (POINT*)&windowRect, 2);
			INT height = TOOLBAR_HIDDENHEIGHT;
			INT x = windowRect.left;
			INT y = windowRect.top;
			UINT swpFlags = SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW;
			UINT animateFlags = AW_HIDE | AW_SLIDE | AW_VER_NEGATIVE;
			HWND zOrder = HWND_TOP;
			
			if (0 != (TBS_BOTTOMDOCK & windowStyle))
			{
				y = windowRect.bottom - height;
				swpFlags &= ~SWP_NOMOVE;
				zOrder = HWND_BOTTOM;
				animateFlags = AW_HIDE | AW_SLIDE | AW_VER_POSITIVE;
			}
						
			AnimateWindow(hwnd, AUTOHIDE_ANIMATETIME, animateFlags);

			if (NULL != toolbar->hBrowser)
			{
				RECT invalidRect;
				CopyRect(&invalidRect, &windowRect);
				MapWindowPoints(hParent, toolbar->hBrowser, (POINT*)&invalidRect, 2);
                RedrawWindow(toolbar->hBrowser, &invalidRect, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN); 
			}

			SetWindowPos(hwnd, zOrder, x, y, windowRect.right - windowRect.left, height, swpFlags);

			if (0 != (WS_VISIBLE & windowStyle))
			{
				SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
		}	
	}

	toolbar->flags |= TBS_HIDDEN;
}

static void Toolbar_AutoHideWindow(HWND hwnd, BOOL fImmediate)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;
	if (0 != ((TBS_HIDDEN | TBS_MENULOOP) & toolbar->flags)) 
	{
		return;
	}

    UINT windowStyle = GetWindowStyle(hwnd);
	if (0 == (TBS_AUTOHIDE & windowStyle)) return;

	if (FALSE == fImmediate)
	{
		if (SetTimer(hwnd, TIMER_AUTOHIDE_ID, TIMER_AUTOHIDE_DELAY, Toolbar_AutoHideTimerProc))
			toolbar->flags |= TBS_HIDETIMERSET;
	}
	else
	{
		Toolbar_AutoHideTimerProc(hwnd, WM_TIMER, TIMER_AUTOHIDE_ID, GetTickCount());
	}
}

static void Toolbar_UpdateUiFlags(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	UINT uiState = (UINT)SendMessage(hwnd, WM_QUERYUISTATE, 0, 0L);

	if (0 != (UISF_HIDEFOCUS & uiState)) toolbar->flags |= TBS_NOFOCUSRECT;
	else toolbar->flags &= ~TBS_NOFOCUSRECT;
}
static void Toolbar_NotifyParent(HWND hwnd, UINT eventId)
{
	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;
	UINT controlId = (UINT)GetWindowLongPtr(hwnd, GWLP_ID);
	SENDCMD(hParent, controlId, eventId, hwnd);
}
static LRESULT Toolbar_OnCreate(HWND hwnd, CREATESTRUCT *pcs)
{
	TOOLBAR *toolbar = (TOOLBAR*)calloc(1, sizeof(TOOLBAR));
	if (NULL != toolbar)
	{
		Toolbar_AddRef(toolbar);

		SetLastError(ERROR_SUCCESS);
		if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)toolbar) && ERROR_SUCCESS != GetLastError())
		{
			free(toolbar);
			toolbar = NULL;
		}
	}

	if (NULL == toolbar)
	{
		DestroyWindow(hwnd);
		return -1;
	}

	toolbar->chevron = Toolbar_CreateItem(TOOLITEM_CHEVRON, 0);
	toolbar->items = new ItemList();
	toolbar->hTooltip = Toolbar_CreateTooltip(pcs->hInstance, hwnd);

	if (NULL != toolbar->hTooltip)
	{
		ifc_skinhelper *skinHelper;
		if (SUCCEEDED(Plugin_GetSkinHelper(&skinHelper)))
		{
			skinHelper->SkinControl(toolbar->hTooltip, SKINNEDWND_TYPE_TOOLTIP, 
					SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT);
			
			skinHelper->Release();
		}
	}

	if (0 != (TBS_AUTOHIDE & pcs->style))
		toolbar->flags |= TBS_HIDDEN;

	return 0;
}

static void Toolbar_OnDestroy(HWND hwnd)
{
	Toolbar_RemoveMouseHook(hwnd, TRUE);
	Toolbar_ClearBrushCache(hwnd);

	TOOLBAR *toolbar = GetToolbar(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);

	Toolbar_Release(toolbar);
}

static void Toolbar_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			Toolbar_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void Toolbar_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	Toolbar_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}

static void Toolbar_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags))
		return;

	Toolbar_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & pwp->flags));
}

static void Toolbar_OnSetRedraw(HWND hwnd, BOOL allowRedraw)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	if (FALSE == allowRedraw)
	{	
		if (0 == (TBS_LOCKUPDATE & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle | TBS_LOCKUPDATE);
	}
	else
	{
		if (0 != (TBS_LOCKUPDATE & windowStyle))
		{
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~TBS_LOCKUPDATE);
			Toolbar_UpdateTabstop(hwnd);
			Toolbar_UpdateLayout(hwnd, FALSE);
			InvalidateRect(hwnd, NULL, TRUE);
		}
	}
	DefWindowProcW(hwnd, WM_SETREDRAW, (WPARAM)allowRedraw, 0L);
}


static void Toolbar_OnEnterMenuLoop(HWND hwnd, BOOL fContext)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	toolbar->flags |= TBS_MENULOOP;
}
static void Toolbar_OnExitMenuLoop(HWND hwnd, BOOL fContext)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	toolbar->flags &= ~TBS_MENULOOP;

	Toolbar_TrackMouseLeave(hwnd);
}

static void Toolbar_OnSetFocus(HWND hwnd, HWND hFocus)
{
	UINT windowStyle = GetWindowStyle(hwnd);

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	
	BOOL focusDirLeft = FALSE;
	if (0 != (0x8000 & GetAsyncKeyState(VK_LEFT)))
	{
		focusDirLeft = TRUE;
	}
	else if (0 != (0x8000 & GetAsyncKeyState(VK_TAB)))
	{
		if (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)))
			focusDirLeft = TRUE;
	}
	else if (0 != (0x8000 & GetAsyncKeyState(VK_LBUTTON)) || 0 != (0x8000 & GetAsyncKeyState(VK_RBUTTON)) || 
			0 != (0x8000 & GetAsyncKeyState(VK_MBUTTON)) || 0 != (0x8000 & GetAsyncKeyState(VK_XBUTTON1)) ||
			0 != (0x8000 & GetAsyncKeyState(VK_XBUTTON2)))
	{
		// mouse press (?)
	}

	if ((size_t)-1 != toolbar->iFocused)
		toolbar->iFocused = Toolbar_GetFocusIndex(hwnd, toolbar->iFocused, 0, FALSE);
	
	if ((size_t)-1 == toolbar->iFocused)
	{
		if (0 != (TBS_TABSTOP & windowStyle))
		{
			if (FALSE == focusDirLeft)
				toolbar->iFocused = Toolbar_GetFocusIndex(hwnd, 0, 1, FALSE);
			else
			{
				size_t last = (NULL != toolbar->items) ? toolbar->items->size() : 0;
				if (last > 0)
					toolbar->iFocused = Toolbar_GetFocusIndex(hwnd, last - 1, -1, FALSE);
			}
		}
		else
		{
			size_t lim, index;
			INT inc;
			if (FALSE == focusDirLeft)
			{
				lim = toolbar->items->size();
				index =  0;
				inc = 1;
			}
			else
			{
				lim = (size_t)-1;
				index = toolbar->items->size();
				if (index > 0) index--;
				inc = -1;
			}
			
			for (; index != lim; index += inc)
			{
				UINT itemStyle = toolbar->items->at(index)->GetStyle();
				UINT mask = ToolbarItem::styleTabstop | ToolbarItem::stateDisabled | ToolbarItem::stateHidden;
				if (ToolbarItem::styleTabstop == (mask & itemStyle))
				{
					toolbar->iFocused = index;
					break;
				}
			}
		}
	}

	if (((size_t)-1) != toolbar->iFocused) 
	{
		ToolbarItem *item = Toolbar_GetItem(toolbar, toolbar->iFocused);
		if (NULL != item) item->SetFocus(hwnd, NULL, TRUE);
	}
	
	
	if (0 != (WS_TABSTOP & windowStyle))
		Toolbar_AutoShowWindow(hwnd);
}

static void Toolbar_OnKillFocus(HWND hwnd, HWND hFocus)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL != toolbar && ((size_t)-1) != toolbar->iFocused) 
	{
		ToolbarItem *item = Toolbar_GetItem(toolbar, toolbar->iFocused);
		if (NULL != item) item->SetFocus(hwnd, NULL, FALSE);
		toolbar->iFocused = ((size_t)-1);
	}
	Toolbar_AutoHideWindow(hwnd, TRUE);
}

static void Toolbar_OnKeyDown(HWND hwnd, INT vKey, UINT flags)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	Toolbar_AutoShowWindow(hwnd);

	INT searchDirection = 0;
	if (VK_LEFT == vKey) searchDirection = -1;
	else if (VK_RIGHT == vKey) searchDirection = 1;

	toolbar->iFocused = Toolbar_GetFocusIndex(hwnd, toolbar->iFocused, searchDirection, FALSE);
	
	ToolbarItem *focusedItem = Toolbar_GetItem(toolbar, toolbar->iFocused);
	
	BOOL fHandled = (NULL != focusedItem && focusedItem->KeyDown(hwnd, vKey, flags));
	if (!fHandled)
	{
		switch(vKey)
		{
			case VK_LEFT:
				Toolbar_NextItem(hwnd, TBNS_PREVITEM, FALSE);
				break;
			case VK_RIGHT:
				Toolbar_NextItem(hwnd, TBNS_NEXTITEM, FALSE);
				break;
			case VK_HOME:
			case VK_PRIOR:
				{
					size_t first = 0;
					first = Toolbar_GetFocusIndex(hwnd, first, 1, FALSE);
					if (((size_t)-1) != first)
						Toolbar_NextItem(hwnd, MAKEINTRESOURCE(first), TRUE);
				}
				break;
			case VK_END:
			case VK_NEXT:
				{
					size_t last = toolbar->items->size();
					last = Toolbar_GetFocusIndex(hwnd, last, -1, FALSE);
					if (((size_t)-1) != last)
						Toolbar_NextItem(hwnd, MAKEINTRESOURCE(last), TRUE);
				}
				break;

		}
	}
}

static void Toolbar_OnKeyUp(HWND hwnd, INT vKey, UINT flags)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	toolbar->iFocused = Toolbar_GetFocusIndex(hwnd, toolbar->iFocused, FALSE, FALSE);
	
	ToolbarItem *focusedItem = Toolbar_GetItem(toolbar, toolbar->iFocused);

	BOOL fHandled = (NULL != focusedItem && focusedItem->KeyUp(hwnd, vKey, flags));
}

static void Toolbar_OnMouseMove(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;
	
	Toolbar_AutoShowWindow(hwnd);
	
	POINT pt;
	POINTSTOPOINT(pt, pts);
	
	RECT clientRect;
	Toolbar_GetClientRect(hwnd, &clientRect);

	if (NULL != toolbar->pressed)
	{
		if (!PtInRect(&clientRect, pt))
		{
			toolbar->pressed->MouseLeave(hwnd);
		}
		else
		{
			toolbar->pressed->MouseMove(hwnd, mouseFlags, pt);
		}
		return;
	}

	ToolbarItem *item = (PtInRect(&clientRect, pt)) ? Toolbar_HitTest(hwnd, pt) : NULL;

	ToolbarItem *prevHighlighted = toolbar->highlighted;
	if (NULL != toolbar->highlighted)
	{
		if (item != toolbar->highlighted)
		{
			toolbar->highlighted->MouseLeave(hwnd);
			toolbar->highlighted = NULL;
		}
	}

	if (NULL != item)
	{		
		UINT style = item->GetStyle();
		if (0 == ((ToolbarItem::stateDisabled | ToolbarItem::stateHidden | ToolbarItem::styleStatic) & style))
		{
			toolbar->highlighted = item;
			item->MouseMove(hwnd, mouseFlags, pt);
			Toolbar_TrackMouseLeave(hwnd);
		}		
	}

	if (NULL != toolbar->hTooltip)
	{
		if (prevHighlighted == toolbar->highlighted || NULL != prevHighlighted)
			Toolbar_RelayTooltipMsg(toolbar->hTooltip, hwnd, WM_MOUSEMOVE, (WPARAM)mouseFlags, *((LPARAM*)(&pts)));

		if (prevHighlighted != toolbar->highlighted)
		{
			Toolbar_SetTooltipItem(hwnd, toolbar->highlighted);
			if (NULL != toolbar->highlighted)
				Toolbar_RelayTooltipMsg(toolbar->hTooltip, hwnd, WM_MOUSEMOVE, (WPARAM)mouseFlags, *((LPARAM*)(&pts)));

		}
	}
}

static void Toolbar_OnMouseLeave(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	if (NULL != toolbar->highlighted)
	{		
		toolbar->highlighted->MouseLeave(hwnd);
		toolbar->highlighted = NULL;
	}

	Toolbar_SetTooltipItem(hwnd, NULL);

	
	POINT cursor;
	if (GetFocus() != hwnd && GetCursorPos(&cursor))
	{
		HWND hCursor = WindowFromPoint(cursor);
		if (NULL != hCursor && 
			GetWindowThreadProcessId(hwnd, NULL) != GetWindowThreadProcessId(hCursor, NULL))
		{
			Toolbar_AutoHideWindow(hwnd, FALSE);
		}
	}
}

static void Toolbar_OnLButtonDown(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;
	
	Toolbar_SetTooltipItem(hwnd, NULL);

	if (NULL != toolbar->highlighted)
		Toolbar_RelayTooltipMsg(toolbar->hTooltip, hwnd, WM_LBUTTONDOWN, (WPARAM)mouseFlags, *((LPARAM*)(&pts)));

	POINT pt;
	POINTSTOPOINT(pt, pts);

	RECT clientRect;
	Toolbar_GetClientRect(hwnd, &clientRect);

	ToolbarItem *item = (PtInRect(&clientRect, pt)) ? Toolbar_HitTest(hwnd, pt) : NULL;
	
   	if (NULL != item)
	{			
		if (0 == ((ToolbarItem::stateDisabled | ToolbarItem::stateHidden | ToolbarItem::styleStatic) & item->GetStyle()))
		{
            item->LButtonDown(hwnd, mouseFlags, pt);
			toolbar->pressed = item;
		}
	}

	SetCapture(hwnd);
}

static void Toolbar_OnLButtonUp(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	if (NULL != toolbar->highlighted)
		Toolbar_RelayTooltipMsg(toolbar->hTooltip, hwnd, WM_LBUTTONUP, (WPARAM)mouseFlags, *((LPARAM*)(&pts)));

	POINT pt;
	POINTSTOPOINT(pt, pts);
	
	RECT clientRect;
	Toolbar_GetClientRect(hwnd, &clientRect);

	ToolbarItem *item = (PtInRect(&clientRect, pt)) ? Toolbar_HitTest(hwnd, pt) : NULL;

	if (NULL != toolbar->pressed)
	{
		ToolbarItem *pressed = toolbar->pressed;
		toolbar->pressed = NULL;
		Toolbar_AddRef(toolbar);
		pressed->AddRef();
		if (pressed == item)
			pressed->Click(hwnd, mouseFlags, pt);
		
		pressed->LButtonUp(hwnd, mouseFlags, pt);
		pressed->Release();
		Toolbar_Release(toolbar);
	}

	if (hwnd == GetCapture())
		ReleaseCapture();
}

static void Toolbar_OnRButtonDown(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	Toolbar_SetTooltipItem(hwnd, NULL);

	if (NULL != toolbar->highlighted)
		Toolbar_RelayTooltipMsg(toolbar->hTooltip, hwnd, WM_RBUTTONDOWN, (WPARAM)mouseFlags, *((LPARAM*)(&pts)));

}

static void Toolbar_OnRButtonUp(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	if (NULL != toolbar->highlighted)
		Toolbar_RelayTooltipMsg(toolbar->hTooltip, hwnd, WM_RBUTTONUP, (WPARAM)mouseFlags, *((LPARAM*)(&pts)));
}

static void Toolbar_OnMButtonDown(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	Toolbar_SetTooltipItem(hwnd, NULL);

	if (NULL != toolbar->highlighted)
		Toolbar_RelayTooltipMsg(toolbar->hTooltip, hwnd, WM_MBUTTONDOWN, (WPARAM)mouseFlags, *((LPARAM*)(&pts)));
}

static void Toolbar_OnMButtonUp(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	if (NULL != toolbar->highlighted)
		Toolbar_RelayTooltipMsg(toolbar->hTooltip, hwnd, WM_MBUTTONUP, (WPARAM)mouseFlags, *((LPARAM*)(&pts)));
}

static void Toolbar_OnContextMenu(HWND hwnd, HWND hOwner, POINTS pts)
{
	POINT pt;
	POINTSTOPOINT(pt, pts);

	if (-1 == pt.x || -1 == pt.y)
	{
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);
        if (!GetCursorPos(&pt) || !PtInRect(&windowRect, pt))
		{
			pt.x = windowRect.left + 2;
			pt.y = windowRect.top + 2;
		}
	}
	else
	{		
		POINT localPt = pt;
		MapWindowPoints(HWND_DESKTOP, hwnd, &localPt, 1);

		RECT clientRect;
		Toolbar_GetClientRect(hwnd, &clientRect);
		ToolbarItem *item = (PtInRect(&clientRect, localPt)) ? Toolbar_HitTest(hwnd, localPt) : NULL;
	
   		if (NULL != item)
		{			
			UINT itemStyle = item->GetStyle();
			if (0 == ((ToolbarItem::stateHidden | ToolbarItem::stateDisabled) & itemStyle))
			{
				RECT itemRect;
				if (item->GetRect(&itemRect) && PtInRect(&itemRect, localPt))
				{
					if (FALSE != item->DisplayContextMenu(hwnd, pt.x, pt.y))
						return;
				}
			}
		}
	}

	Toolbar_DisplayContextMenu(hwnd, pt.x, pt.y);
}
static void Toolbar_OnCaptureChanged(HWND hwnd, HWND hCapture)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;
	Toolbar_TrackMouseLeave(hwnd);
}
static LRESULT Toolbar_OnGetFont(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return NULL;
	return (LRESULT)toolbar->textFont;
}


static void Toolbar_OnSetFont(HWND hwnd, HFONT hFont, BOOL fRedraw)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	toolbar->textFont = hFont;

	Toolbar_UpdateTextMetrics(hwnd);

	if (FALSE != fRedraw)
		InvalidateRect(hwnd, NULL, TRUE);
}


static void Toolbar_GetTootipDispInfo(HWND hwnd, NMTTDISPINFO *pdisp)
{
	ToolbarItem *item = (ToolbarItem*)pdisp->lParam;
	if (NULL != item)
	{	
		item->GetTip(pdisp->szText, ARRAYSIZE(pdisp->szText));
	}
}

static LRESULT Toolbar_OnTooltipNotify(HWND hwnd, NMHDR *pnmh)
{
	switch(pnmh->code)
	{
		case TTN_GETDISPINFO:
			Toolbar_GetTootipDispInfo(hwnd, (NMTTDISPINFO*)pnmh);
			break;
	}
	return 0;
}
static LRESULT Toolbar_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return 0;

	if (toolbar->hTooltip == pnmh->hwndFrom && NULL != toolbar->hTooltip)
		return Toolbar_OnTooltipNotify(hwnd, pnmh);
	
	return 0;
}

static void Toolbar_OnCommand(HWND hwnd, INT commandId, INT eventId, HWND hControl)
{
	BOOL fEnabled;
	switch(commandId)
	{
		case ID_TOOLBAR_DOCKTOP: 
		case ID_TOOLBAR_DOCKBOTTOM:	
			fEnabled = (ID_TOOLBAR_DOCKBOTTOM == commandId);
			if (fEnabled != Toolbar_EnableBottomDock(hwnd, fEnabled))
				Toolbar_NotifyParent(hwnd, TBN_DOCKCHANGED);
			break;
		case ID_TOOLBAR_AUTOHIDE:	
			fEnabled = (0 == (TBS_AUTOHIDE & GetWindowStyle(hwnd)));
			if (fEnabled != Toolbar_EnableAutoHide(hwnd, fEnabled))
				Toolbar_NotifyParent(hwnd, TBN_AUTOHIDECHANGED);
			break;
		case ID_TOOLBAR_TABSTOP:
			fEnabled = (0 == (TBS_TABSTOP & GetWindowStyle(hwnd)));
			if (fEnabled != Toolbar_EnableTabStop(hwnd, fEnabled))
				Toolbar_NotifyParent(hwnd, TBN_TABSTOPCHANGED);
			break;
	}
}

static BOOL Toolbar_ProcessTabKey(HWND hwnd, UINT messageId)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return FALSE;
	
	BOOL focusDirLeft = (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)));
				
	size_t lim, index;
	INT inc;
	if (FALSE == focusDirLeft)
	{
		inc = 1;
		lim = toolbar->items->size();
		index =  toolbar->iFocused;
		if ((size_t)-1 == index) index = 0;
		index++;
		if (index >= lim) return FALSE;
	}
	else
	{
		inc = -1;
		lim = (size_t)-1;
		index = toolbar->iFocused;
		if ((size_t)-1 == index) index = toolbar->items->size();
		index--;
		if (index >= toolbar->items->size()) return FALSE;
	}
		
	for (; index != lim; index += inc)
	{
		ToolbarItem *item = toolbar->items->at(index);
		if (FALSE != Toolbar_IsItemAcceptFocus(item, TRUE))
		{
			item->SetFocus(hwnd, NULL, TRUE);
			return TRUE;
		}
	}

	return FALSE;
}

static LRESULT Toolbar_OnGetDlgCode(HWND hwnd, INT vKey, MSG *pMsg)
{
	if (NULL != pMsg)
	{
		switch(vKey)
		{
			case VK_TAB:
				if (FALSE == Toolbar_ProcessTabKey(hwnd, pMsg->message))
				{				
					if (WM_KEYDOWN == pMsg->message)
						Toolbar_AutoHideWindow(hwnd, TRUE);

					return 0;
				}
				break;

		}
	}

	return DLGC_WANTALLKEYS;
}


static void Toolbar_OnUpdateUiState(HWND hwnd, INT action, INT state)
{
	DefWindowProc(hwnd, WM_UPDATEUISTATE, MAKEWPARAM(action, state), 0L);
	Toolbar_UpdateUiFlags(hwnd);
}

static void Toolbar_OnEnable(HWND hwnd, BOOL fEnable)
{
	InvalidateRect(hwnd, NULL, FALSE);
}

static LRESULT Toolbar_OnSetCursor(HWND hwnd, HWND hCursor, UINT hitTest, UINT messageId)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return FALSE;

	if (0 == (TBS_MENULOOP & toolbar->flags) && hwnd == hCursor)
	{
		ToolbarItem *item = (NULL != toolbar->pressed) ? toolbar->pressed : toolbar->highlighted;
		if (NULL != item && FALSE != item->SetCursor(hwnd, hCursor, hitTest, messageId))
		{
			return TRUE;
		}
	}

	return DefWindowProcW(hwnd, WM_SETCURSOR, (WPARAM)hCursor, MAKELPARAM(hitTest, messageId));
}

static LRESULT Toolbar_OnColorEdit(HWND hwnd, HDC hdcCtrl, HWND hwndCtrl)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return 0;

	SetTextColor(hdcCtrl, toolbar->rgbEdit);
	SetBkColor(hdcCtrl, toolbar->rgbEditBk);
	return (LRESULT)Toolbar_GetBrush(hwnd, TOOLBRUSH_ITEMBK);
}

static LRESULT Toolbar_OnColorStatic(HWND hwnd, HDC hdcCtrl, HWND hwndCtrl)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return 0;

	SetTextColor(hdcCtrl, toolbar->rgbText);
	SetBkColor(hdcCtrl, toolbar->rgbBk);
	return (LRESULT)Toolbar_GetBrush(hwnd, TOOLBRUSH_BACK);
}


static LRESULT Toolbar_OnGetIdealHeight(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
    if (NULL == toolbar || NULL == toolbar->imageList) return 0;

	INT toolbarHeight, iconWidth;
	if (!ImageList_GetIconSize(toolbar->imageList, &iconWidth, &toolbarHeight))
		toolbarHeight = 0;
	else
		toolbarHeight /= TOOLBAR_ICONSTATE_COUNT;

	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
	{
		HFONT originalFont = (HFONT)SelectObject(hdc, toolbar->textFont);
		TEXTMETRIC tm;
		if (GetTextMetrics(hdc, &tm))
		{
			tm.tmHeight += ((tm.tmHeight - (tm.tmAscent - tm.tmInternalLeading))/2 + 2);
			if (toolbarHeight <= tm.tmHeight)
				toolbarHeight = tm.tmHeight + 4;
		}
		SelectObject(hdc, originalFont);
		ReleaseDC(hwnd, hdc);
	}
	return toolbarHeight + TOOLBAR_SPACE_TOP + TOOLBAR_SPACE_BOTTOM;
}

static void Toolbar_OnUpdateSkin(HWND hwnd, BOOL fRedraw)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return;

	ifc_skinhelper *skinHelper;
	if (FAILED(Plugin_GetSkinHelper(&skinHelper)))
		skinHelper = NULL;

	Toolbar_UpdateColorTable(hwnd, skinHelper);
	Toolbar_ClearBrushCache(hwnd);

	if (NULL != toolbar->imageList)
		ImageList_Destroy(toolbar->imageList);

	toolbar->imageList = Toolbar_LoadImagelist(toolbar, Plugin_GetInstance(), 
							MAKEINTRESOURCE(IDR_TOOLBARLARGE_IMAGE),
							21, TOOLBAR_ICONSTATE_COUNT);
	
	if (NULL != toolbar->ownedFont)
	{
		DeleteObject(toolbar->ownedFont);
	}

	HFONT font = (NULL != skinHelper) ? skinHelper->GetFont() : NULL;

	if (0/* create custom font */)
	{
		toolbar->ownedFont = Toolbar_CreateFont(font);
		if (NULL != toolbar->ownedFont) 	
			font = toolbar->ownedFont;
	}
	else
	{
		toolbar->ownedFont = NULL;
	}
	
	SendMessage(hwnd, WM_SETFONT, (WPARAM)font, FALSE);

	if (NULL != toolbar->hTooltip)
	{
		SendMessage(toolbar->hTooltip, WM_SETFONT, (WPARAM)(toolbar->textFont), TRUE);
	}

	Toolbar_UpdateUiFlags(hwnd);

	if (NULL != toolbar->items)
	{
		size_t index = toolbar->items->size();
		while(index-- > 0)
		{
			ToolbarItem *item = toolbar->items->at(index);
            if (NULL != item) 
			{
				item->UpdateSkin(hwnd);
			}
		}
	}

	Toolbar_UpdateLayout(hwnd, FALSE);
	
	if (FALSE != fRedraw)
		InvalidateRect(hwnd, NULL, TRUE);

	if (NULL != skinHelper)
		skinHelper->Release();
}


static LRESULT Toolbar_OnGetIconSize(HWND hwnd, INT iconIndex, SIZE *pSize)
{
	if (NULL == pSize) return FALSE;
	TOOLBAR *toolbar = GetToolbar(hwnd);

	if (ICON_CHEVRON == iconIndex)
	{
		UINT windowStyle = GetWindowStyle(hwnd);
		iconIndex = (0 != (TBS_BOTTOMDOCK & windowStyle)) ? ICON_CHEVRON_BOTTOM : ICON_CHEVRON_TOP;
	}

	if (NULL == toolbar || 
		NULL == toolbar->imageList ||
		iconIndex < 0 ||
		iconIndex >= ImageList_GetImageCount(toolbar->imageList))
	{
		ZeroMemory(pSize, sizeof(SIZE));
		return FALSE;
	}

	INT cx, cy;
	BOOL result = ImageList_GetIconSize(toolbar->imageList, &cx, &cy);
	
	if (FALSE != result)
	{
		pSize->cy = cy / TOOLBAR_ICONSTATE_COUNT;
		switch(iconIndex)
		{
			case ICON_SEPARATOR:
				pSize->cx = ICON_SEPARATOR_WIDTH;
				break;
			case ICON_CHEVRON_TOP:
			case ICON_CHEVRON_BOTTOM:
				pSize->cx = ICON_CHEVRON_WIDTH;
				break;
			case ICON_HISTORY:
				pSize->cx = ICON_HISTORY_WIDTH;
				break;
			case ICON_BACK:
			case ICON_FORWARD:
				pSize->cx =ICON_ARROW_WIDTH;
				break;
			default:
				pSize->cx = cx;
				break;
		}
		
	}
	else
		ZeroMemory(pSize, sizeof(SIZE));
	return result;
}

static LRESULT Toolbar_OnSendCommand(HWND hwnd, INT commandId)
{
	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return FALSE;

	if (ID_CHEVRON_CLICKED == commandId)
	{
		Toolbar_DisplayChevronMenu(hwnd);
		return 0;
	}
	
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL != toolbar && NULL != toolbar->items)
	{
		size_t index= toolbar->items->size();
		while(index--)
			toolbar->items->at(index)->CommandSent(hwnd, commandId);
	}
	return SendMessage(hParent, WM_COMMAND, MAKEWPARAM(commandId, 0), (LPARAM)hwnd);
}

static LRESULT Toolbar_OnDrawIcon(HWND hwnd, TOOLBARDRAWICONPARAM *iconParam)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar || NULL == iconParam) return FALSE;
	INT iconIndex = iconParam->iconIndex;
	if (ICON_CHEVRON == iconIndex)
	{
		UINT windowStyle = GetWindowStyle(hwnd);
		iconIndex = (0 != (TBS_BOTTOMDOCK & windowStyle)) ? ICON_CHEVRON_BOTTOM : ICON_CHEVRON_TOP;
	}

	if  (NULL == toolbar->imageList || 
		iconIndex < 0 || 
		iconIndex >= ImageList_GetImageCount(toolbar->imageList))
	{
		return FALSE;
	}
	
	

	INT frameHeight, frameWidth;
	if (!ImageList_GetIconSize(toolbar->imageList, &frameWidth, &frameHeight))
	{
		frameHeight = 0;
		frameWidth = 0;
	}

	INT offsetY;
	if (0 != (ToolbarItem::stateDisabled & iconParam->itemState))
		offsetY = TOOLBAR_ICONSTATE_DISABLED;
	else if (0 != (ToolbarItem::statePressed & iconParam->itemState))
		offsetY = TOOLBAR_ICONSTATE_PRESSED;
	else if (0 != (ToolbarItem::stateHighlighted & iconParam->itemState))
		offsetY = TOOLBAR_ICONSTATE_HIGHLIGHTED;
	else
		offsetY = TOOLBAR_ICONSTATE_NORMAL;

	frameHeight = frameHeight / TOOLBAR_ICONSTATE_COUNT;

	IMAGELISTDRAWPARAMS param;
	param.cbSize = sizeof(IMAGELISTDRAWPARAMS) - sizeof(DWORD) * 3;
	param.himl = toolbar->imageList;
	param.i = iconIndex;
	param.hdcDst = iconParam->hdcDst;
	param.x = iconParam->x;
	param.y = iconParam->y;
	param.cx = (iconParam->cx > frameWidth) ? frameWidth : iconParam->cx;
	param.cy = (iconParam->cy > frameHeight) ? frameHeight : iconParam->cy;
	param.xBitmap = 0;
	param.yBitmap = frameHeight * offsetY;
	param.rgbBk = CLR_NONE;
	param.rgbFg = CLR_NONE;
	param.fStyle = ILD_NORMAL;
	param.dwRop = SRCCOPY;
	param.fState = ILS_NORMAL;
	param.Frame = 0;
	param.crEffect = 0;
	
	return ImageList_DrawIndirect(&param);
}

static LRESULT Toolbar_OnGetItemCount(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar || NULL == toolbar->items) return 0;
	return (INT)toolbar->items->size();
}

static LRESULT Toolbar_OnClear(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return FALSE; 
	if (NULL == toolbar->items) return TRUE; 

	Toolbar_ClearItems(toolbar);

	UINT windowStyle = GetWindowStyle(hwnd);
	if (0 == (TBS_LOCKUPDATE & windowStyle))
	{
		Toolbar_UpdateTabstop(hwnd);
		Toolbar_UpdateLayout(hwnd, FALSE);
		InvalidateRect(hwnd, NULL, TRUE);
	}
	
	return TRUE;
}

static LRESULT Toolbar_OnInsertItem(HWND hwnd, TOOLBARINSERTITEM *insertItem)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar || NULL == toolbar->items || 
		NULL == insertItem || insertItem->cbSize != sizeof(TOOLBARINSERTITEM)) 
	{
		return -1;
	}
	
	INT index = ITEM_ERR;

	UINT styleOverride = 0;
	if (0 != (TBIS_HIDDEN & insertItem->style)) 	styleOverride |= ToolbarItem::stateHidden;
	if (0 != (TBIS_DISABLED & insertItem->style)) styleOverride |= ToolbarItem::stateDisabled;
	if (0 != (TBIS_CHEVRONONLY & insertItem->style)) styleOverride |= ToolbarItem::styleChevronOnly;
	if (0 != (TBIS_NOCHEVRON & insertItem->style)) styleOverride |= ToolbarItem::styleNoChevron;
	if (0 != (TBIS_POPUP & insertItem->style)) styleOverride |= ToolbarItem::stylePopup;

	ToolbarItem *item = Toolbar_CreateItem(insertItem->pszName, styleOverride);
	if (NULL != item)
	{
		index = Toolbar_InsertItemInternal(toolbar, item, insertItem->insertBefore, hwnd);
		item->Release();
		if (ITEM_ERR != index)
		{
			UINT windowStyle = GetWindowStyle(hwnd);
			if (0 == (TBS_LOCKUPDATE & windowStyle))
			{				
				Toolbar_UpdateTabstop(hwnd);
				Toolbar_UpdateLayout(hwnd, FALSE);
				InvalidateRect(hwnd, NULL, TRUE);
			}
		}
	}
	
	return index;
}

static LRESULT Toolbar_OnFindItem(HWND hwnd, LPCSTR pszName)
{
	return Toolbar_ResolveName(hwnd, pszName);
}

static LRESULT Toolbar_OnRemoveItem(HWND hwnd, LPCSTR pszName)
{
	INT index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index) return FALSE;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar || NULL == toolbar->items || index == toolbar->items->size()) 
		return FALSE; 
		
	ToolbarItem *item = toolbar->items->at(index);
	toolbar->items->erase(toolbar->items->begin() + index);
	if (NULL != item) item->Release();

	UINT windowStyle = GetWindowStyle(hwnd);
	if (0 == (TBS_LOCKUPDATE & windowStyle))
	{	
		Toolbar_UpdateTabstop(hwnd);
		Toolbar_UpdateLayout(hwnd, FALSE);
		InvalidateRect(hwnd, NULL, TRUE);
	}
	
	return TRUE;
}


static LRESULT Toolbar_OnSetItemInt(HWND hwnd, LPCSTR pszName, INT value)
{
	INT index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index) return FALSE;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (index == toolbar->items->size()) return FALSE;
	return toolbar->items->at(index)->SetValueInt(hwnd, value);
}

static LRESULT Toolbar_OnSetItemString(HWND hwnd, LPCSTR pszName, LPCWSTR value)
{
	INT index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index) return FALSE;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (index == toolbar->items->size()) return FALSE;
	return toolbar->items->at(index)->SetValueStr(hwnd, value);
}
static COLORREF Toolbar_OnGetBkColor(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	return  (NULL != toolbar) ? toolbar->rgbBk : 0x00FF00FF;
}
static COLORREF Toolbar_OnGetFgColor(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	return  (NULL != toolbar) ? toolbar->rgbFg : 0x00FF00FF;
}

static COLORREF Toolbar_OnGetTextColor(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	return  (NULL != toolbar) ? toolbar->rgbText : 0x00FF00FF;
}

static COLORREF Toolbar_OnGetHiliteColor(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	return  (NULL != toolbar) ? toolbar->rgbHilite : 0x00FF00FF;
}

static COLORREF Toolbar_OnGetEditColor(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	return  (NULL != toolbar) ? toolbar->rgbEdit : 0x00FF00FF;
}

static COLORREF Toolbar_OnGetEditBkColor(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	return  (NULL != toolbar) ? toolbar->rgbEditBk : 0x00FF00FF;
}

static LRESULT Toolbar_OnEnableItem(HWND hwnd, LPCSTR pszName, BOOL fEnable)
{
	INT index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index) return FALSE;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (index == toolbar->items->size()) return FALSE;

	ToolbarItem *item = toolbar->items->at(index);
	if (NULL == item) return FALSE;

	item->SetStyle(hwnd, (0 != fEnable) ? 0 : ToolbarItem::stateDisabled, ToolbarItem::stateDisabled);
	if (FALSE == fEnable && toolbar->highlighted == item)
		Toolbar_SetTooltipItem(hwnd, NULL);

	Toolbar_UpdateTabstop(hwnd);

	return TRUE;
}

static LRESULT Toolbar_OnGetItemStyle(HWND hwnd, LPCSTR pszName, UINT fMask)
{
	INT index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index) return 0;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (index == toolbar->items->size()) return 0;

	UINT itemStyle = toolbar->items->at(index)->GetStyle();
    return (itemStyle & fMask);
}

static LRESULT Toolbar_OnGetItemCommand(HWND hwnd, LPCSTR pszName)
{
	INT index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index) return 0;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (index == toolbar->items->size()) return 0;

	return toolbar->items->at(index)->GetCommandId();
}

static LRESULT Toolbar_OnShowItem(HWND hwnd, LPCSTR pszName, BOOL fShow)
{
	INT index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index) return FALSE;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (index == toolbar->items->size()) return FALSE;

	ToolbarItem *item = toolbar->items->at(index);
	if (NULL == item) return FALSE;

	UINT style = item->GetStyle();
	if ((0 != (ToolbarItem::stateHidden & style)) == (FALSE == fShow))
		return TRUE;

	item->SetStyle(hwnd, (0 != fShow) ? 0 : ToolbarItem::stateHidden, ToolbarItem::stateHidden);

	UINT windowStyle = GetWindowStyle(hwnd);
	if (0 == (TBS_LOCKUPDATE & windowStyle))
	{		
		Toolbar_UpdateTabstop(hwnd);	
		Toolbar_UpdateLayout(hwnd, FALSE);
		InvalidateRect(hwnd, NULL, TRUE);
	}

	return TRUE;
}

static LRESULT Toolbar_OnSetItemDescription(HWND hwnd, LPCSTR pszName, LPCWSTR pszDescription)
{
	INT index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index) return FALSE;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (index == toolbar->items->size()) return FALSE;

	ToolbarItem *item = toolbar->items->at(index);
	if (NULL == item) return FALSE;

	if (FALSE == item->SetDescription(hwnd, pszDescription))
		return FALSE;
	
	if (IsWindowVisible(hwnd) && IsWindowEnabled(hwnd))
		Toolbar_UpdateTip(hwnd);
	
	return TRUE;
}


static void Toolbar_OnUpdateTip(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL != toolbar && NULL != toolbar->hTooltip)
	{
		TOOLINFO ti;
		ti.cbSize = sizeof(TOOLINFO);
		ti.uId = 0;
		ti.hinst = NULL;
		ti.hwnd = hwnd;
		ti.lpszText = LPSTR_TEXTCALLBACK;
		SendMessage(toolbar->hTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	}

}

static LRESULT Toolbar_OnGetTextMetrics(HWND hwnd, TOOLBARTEXTMETRIC *metric)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return FALSE;
	CopyMemory(metric, &toolbar->textMetric, sizeof(TOOLBARTEXTMETRIC));
	return TRUE;
}

static LRESULT Toolbar_OnGetBkBrush(HWND hwnd)
{
	return (LRESULT)Toolbar_GetBrush(hwnd, TOOLBRUSH_BACK);
}

static LRESULT Toolbar_OnLayout(HWND hwnd, TOOLBARLAYOUT *layout)
{
	if (NULL == layout)
		return FALSE;
	
	CopyRect(&layout->clientRect, layout->prcParent);
	DWORD windowStyle = GetWindowStyle(hwnd);

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar || 0 == (WS_VISIBLE & windowStyle))
	{
		layout->insertAfter = HWND_BOTTOM;
		SetRectEmpty(&layout->toolbarRect);
		return TRUE;
	}
	
	CopyRect(&layout->toolbarRect, layout->prcParent);
	
	INT toolbarHeight, clientInflate;

	if (0 != (TBS_AUTOHIDE & windowStyle))
	{
		clientInflate = TOOLBAR_HIDDENHEIGHT;
		toolbarHeight = (0 == (TBS_HIDDEN & toolbar->flags)) ? 
						Toolbar_GetIdealHeight(hwnd) : TOOLBAR_HIDDENHEIGHT; 
	}
	else
	{
		toolbarHeight = Toolbar_GetIdealHeight(hwnd);
		clientInflate = toolbarHeight;
	}
			
	
	if (0 != (TBS_BOTTOMDOCK & windowStyle))
	{
		layout->toolbarRect.top = layout->toolbarRect.bottom - toolbarHeight;
		if (layout->toolbarRect.top < layout->prcParent->top)
			layout->toolbarRect.top = layout->prcParent->top;
		layout->clientRect.bottom -= clientInflate;
		layout->insertAfter = (0 == (TBS_AUTOHIDE & windowStyle) || 0 != (TBS_HIDDEN & toolbar->flags)) ?	
								HWND_BOTTOM : HWND_TOP;
	}
	else
	{
		layout->toolbarRect.bottom = layout->toolbarRect.top + toolbarHeight;
		if (layout->toolbarRect.bottom > layout->prcParent->bottom)
			layout->toolbarRect.bottom = layout->prcParent->bottom;
		layout->clientRect.top += clientInflate;
		layout->insertAfter = HWND_TOP;
	}
		
	return TRUE;
}

static BOOL Toolbar_OnNextItem(HWND hwnd, LPCSTR pszName, BOOL fUseName)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return FALSE;

	size_t focusIndex = toolbar->iFocused;
	if (FALSE == fUseName)
	{
		focusIndex = Toolbar_GetFocusIndex(hwnd, toolbar->iFocused, 0, FALSE);
		if (((size_t)-1) == focusIndex)
		{
			if (TBNS_NEXTITEM == pszName)
			{
				focusIndex = toolbar->items->size();
				if (focusIndex > 0) focusIndex--;
			}
			else
			{
				focusIndex = 0;
			}
		}
		else
		{
			if (TBNS_NEXTITEM == pszName)
			{
				if (focusIndex <= toolbar->items->size())
					focusIndex = Toolbar_GetFocusIndex(hwnd, focusIndex + 1, 1, FALSE);
			}
			else
			{
				if (focusIndex > 0)
					focusIndex = Toolbar_GetFocusIndex(hwnd, focusIndex - 1, -1, FALSE);
			}
			
			if (((size_t)-1) == focusIndex)	
				focusIndex = toolbar->iFocused;
		}
	}
	else
	{
		INT index = Toolbar_ResolveName(hwnd, pszName);
		if (ITEM_ERR == index || 
			!Toolbar_IsItemAcceptFocus(Toolbar_GetItem(toolbar, index), FALSE)) 
			return FALSE;
		focusIndex = (size_t)index;
	}

	BOOL focusChanged = FALSE;
	if (focusIndex != toolbar->iFocused)
	{
		ToolbarItem *itemNew, *itemOld;
		
		itemOld = Toolbar_GetItem(toolbar, toolbar->iFocused);
		itemNew = Toolbar_GetItem(toolbar, focusIndex);
		toolbar->iFocused = focusIndex;

		if (NULL != itemOld) itemOld->SetFocus(hwnd, itemNew, FALSE);
		if (NULL != itemNew) itemNew->SetFocus(hwnd, itemOld, TRUE);

		focusChanged = TRUE;
	}

	HWND hFocus = GetFocus();
	if (hwnd != hFocus && FALSE == IsChild(hwnd, hFocus))
	{
		if (IsWindowVisible(hwnd) && IsWindowEnabled(hwnd))
		{
			HWND hRoot = GetAncestor(hwnd, GA_ROOT);
			if (NULL != hRoot)
				SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hwnd, TRUE);
			if (hwnd != GetFocus())
				SetFocus(hwnd);
		}
	}
	else if (FALSE == focusChanged)
	{
		ToolbarItem *item = Toolbar_GetItem(toolbar, toolbar->iFocused);
		if (NULL != item) item->SetFocus(hwnd, item, TRUE);
	}

	return TRUE;
}

static BOOL Toolbar_OnGetItemInfo(HWND hwnd, LPCSTR pszName, TBITEMINFO *itemInfo)
{
	if (NULL == itemInfo)
		return FALSE;

	INT index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index) return FALSE;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar || index == toolbar->items->size()) return FALSE;

	ToolbarItem *item = toolbar->items->at(index);
	if (NULL == item) return FALSE;

	BOOL result = TRUE;

	itemInfo->commandId = item->GetCommandId();
	itemInfo->style = item->GetStyle();

	if (NULL != itemInfo->pszText)
	{
		if (FAILED(item->GetText(itemInfo->pszText, itemInfo->cchText)))
			result = FALSE;
	}

	if (NULL != itemInfo->pszDescription)
	{
		if (FAILED(item->GetDescription(itemInfo->pszDescription, itemInfo->cchDescription)))
			result = FALSE;
	}
	return result;
}


static HRESULT Toolbar_GetServiceCommandState(HWND hBrowser,ifc_omservicecommand *serviceCommand, const GUID *commandGroup, UINT commandId, HRESULT defaultState)
{
	HRESULT state = (NULL != serviceCommand) ? serviceCommand->QueryState(hBrowser, commandGroup, commandId) : defaultState;
	if (E_NOTIMPL == state) 
		state = defaultState;
	return state;
}

static INT Toolbar_InsertItemHelper(HWND hwnd, LPCSTR pszName, UINT overrideStyle, INT insertBefore)
{
	INT index = ITEM_ERR;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL != toolbar && NULL != toolbar->items)
	{
		ToolbarItem *item = Toolbar_CreateItem(pszName, overrideStyle);
		if (NULL != item) 
		{
			index = Toolbar_InsertItemInternal(toolbar, item, insertBefore, hwnd);
			item->Release();
		}
	}
	return index;
}
static INT Toolbar_AddItemHelper(HWND hwnd, LPCSTR pszName, UINT overrideStyle)
{
	return Toolbar_InsertItemHelper(hwnd, pszName, overrideStyle, TBIP_LAST);
}

static INT Toolbar_AddItemHelper2(HWND hwnd, LPCSTR pszName, UINT overrideStyle, ifc_omservicecommand *serviceCommand,  const GUID *commandGroup, ULONG commandId, HRESULT defaultState)
{
	HWND hBrowser = GetParent(hwnd);
	HRESULT commandState = Toolbar_GetServiceCommandState(hBrowser, serviceCommand, commandGroup, commandId, defaultState);
	if (CMDSTATE_ENABLED != commandState)
		return ITEM_ERR;
	
	return Toolbar_AddItemHelper(hwnd, pszName, overrideStyle);
}

static LRESULT Toolbar_OnAutoPopulate(HWND hwnd, ifc_omservice *service, UINT flags)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar || NULL == toolbar->items) 
		return 0;

	ifc_omservicecommand *serviceCommand;
	if (NULL == service || FAILED(service->QueryInterface(IFC_OmServiceCommand, (void**)&serviceCommand)))
		serviceCommand = NULL;

	UINT windowStyle = GetWindowStyle(hwnd);

	Toolbar_ClearItems(toolbar);
		
	ToolbarItem *item;
	HRESULT commandStatus;
	size_t blockSize, currentSize;
		
	HWND hBrowser = GetParent(hwnd);

	// Back/Forward
	commandStatus = Toolbar_GetServiceCommandState(hBrowser, serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_BACKFORWARD, CMDSTATE_ENABLED);
	if (SUCCEEDED(commandStatus))
	{
		Toolbar_AddItemHelper(hwnd, TOOLITEM_BUTTON_BACK, ToolbarItem::stateDisabled);
		Toolbar_AddItemHelper(hwnd, TOOLITEM_BUTTON_FORWARD, ToolbarItem::stateDisabled);

		// History
		if (0 != toolbar->items->size())
		{
			Toolbar_AddItemHelper2(hwnd, TOOLITEM_BUTTON_HISTORY, ToolbarItem::stateDisabled | ToolbarItem::styleNoChevron, 
				serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_HISTORY, CMDSTATE_ENABLED);
		}
	}

	blockSize = toolbar->items->size();

	// Home
	Toolbar_AddItemHelper2(hwnd, TOOLITEM_BUTTON_HOME, ToolbarItem::stateDisabled, 
			serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_HOME, CMDSTATE_ENABLED);

	// Refresh
	Toolbar_AddItemHelper2(hwnd, TOOLITEM_BUTTON_REFRESH, ToolbarItem::stateDisabled, 
			serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_REFRESH, CMDSTATE_ENABLED);
	
	// Stop
	Toolbar_AddItemHelper2(hwnd, TOOLITEM_BUTTON_STOP, ToolbarItem::stateDisabled, 
			serviceCommand, &CMDGROUP_NAVIGATION, NAVCOMMAND_STOP, CMDSTATE_ENABLED);
	
	currentSize = toolbar->items->size();
	if (0 != blockSize && 0 != currentSize && blockSize != currentSize)
		Toolbar_InsertItemHelper(hwnd, TOOLITEM_SPACE, ToolbarItem::styleNoChevron, (INT)blockSize);
	
	BOOL fShowFlexSpace = TRUE;

	// Addressbar
	UINT style = ToolbarItem::styleNoChevron | ToolbarItem::stateDisabled | ToolbarItem::stylePopup;

	if (0  == (TBS_FORCEADDRESS	& windowStyle) &&
		CMDSTATE_ENABLED != Toolbar_GetServiceCommandState(hBrowser, serviceCommand, 
							&CMDGROUP_ADDRESSBAR, ADDRESSCOMMAND_VISIBLE, 
							(0 != (TBS_SHOWADDRESS & windowStyle)) ? CMDSTATE_ENABLED : CMDSTATE_DISABLED))
	{
		style |= ToolbarItem::stateHidden;
	}

	if (CMDSTATE_ENABLED == Toolbar_GetServiceCommandState(hBrowser, serviceCommand, 
			&CMDGROUP_ADDRESSBAR, ADDRESSCOMMAND_READONLY, 
			(0 != (TBPF_READONLYADDRESS & flags) ? CMDSTATE_ENABLED : CMDSTATE_DISABLED)))
	{
		style |= ToolbarAddress::styleAddressReadonly;
	}

	if (0 == (TBS_FANCYADDRESS & windowStyle))
		style |= ToolbarAddress::styleAddressShowReal;
				
	if (ITEM_ERR != Toolbar_AddItemHelper(hwnd, TOOLITEM_ADDRESSBAR, style))
	{
		currentSize++;
		if (0 == (ToolbarItem::stateHidden & style))
			fShowFlexSpace = FALSE; 
	}

	
	
	if (0 != currentSize)
	{			
		Toolbar_AddItemHelper(hwnd, TOOLITEM_FLEXSPACE, ToolbarItem::stylePopup | ((FALSE == fShowFlexSpace) ? ToolbarItem::stateHidden : 0));
	}

	if (NULL != serviceCommand && 0 == (TBPF_NOSERVICECOMMANDS & flags))
	{		
		blockSize = toolbar->items->size();

		// Rating
		commandStatus = Toolbar_GetServiceCommandState(hBrowser, serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_RATE, CMDSTATE_UNKNOWN);
		if (SUCCEEDED(commandStatus))
		{
			UINT rating;
			if (NULL != service && SUCCEEDED(service->GetRating(&rating)))
			{
				if (ITEM_ERR != Toolbar_AddItemHelper(hwnd, TOOLITEM_USERRATING, 0))
					Toolbar_SetItemInt(hwnd, TOOLITEM_USERRATING, rating);
			}
		}

		// Show Info
		Toolbar_AddItemHelper2(hwnd, TOOLITEM_CMDLINK_INFO, ToolbarItem::styleChevronOnly, 
			serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_SHOWINFO, CMDSTATE_UNKNOWN);

		// Report
		Toolbar_AddItemHelper2(hwnd, TOOLITEM_CMDLINK_REPORT, ToolbarItem::styleChevronOnly, 
			serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_REPORT, CMDSTATE_UNKNOWN);

		// Remove
		Toolbar_AddItemHelper2(hwnd, TOOLITEM_CMDLINK_UNSUBSCRIBE, ToolbarItem::styleChevronOnly, 
			serviceCommand, &CMDGROUP_SERVICE, SVCCOMMAND_UNSUBSCRIBE, CMDSTATE_UNKNOWN);

		currentSize = toolbar->items->size();

		if (0 != currentSize && blockSize != currentSize)
			Toolbar_AddItemHelper(hwnd, TOOLITEM_SEPARATOR, ToolbarItem::styleChevronOnly);
	
	}

	const TOOLBARITEMINSERTREC szBrowserTools[] = 
	{ 			
		{ TOOLITEM_BUTTON_SECURECONNECTION, ToolbarItem::stateHidden | ToolbarItem::stylePopup | ToolbarItem::styleNoChevron /*| TBIS_NOREMOVE*/ },
		{ TOOLITEM_BUTTON_SCRIPTERROR, ToolbarItem::stateHidden | ToolbarItem::stylePopup | ToolbarItem::styleNoChevron /*| TBIS_NOREMOVE*/},
		{ TOOLITEM_DOWNLOADPROGRESS,  ToolbarItem::styleNoChevron | ToolbarItem::stateDisabled /*| TBIS_NOREMOVE*/},
	};

	for (INT i = 0; i < ARRAYSIZE(szBrowserTools); i++)
	{
		item = Toolbar_CreateItem(szBrowserTools[i].name, szBrowserTools[i].style);
		if (NULL != item)
		{
			Toolbar_InsertItemInternal(toolbar, item, TBIP_LAST, hwnd);
			item->Release();
		}
	}
	
	if (0 == (TBS_LOCKUPDATE & windowStyle))
	{
		Toolbar_UpdateTabstop(hwnd);
		Toolbar_UpdateLayout(hwnd, FALSE);
		InvalidateRect(hwnd, NULL, TRUE);
	}
	
	if (NULL != serviceCommand)
		serviceCommand->Release();

	return (INT)(INT_PTR)toolbar->items->size();

}

static LRESULT Toolbar_OnEnableBottomDock(HWND hwnd, BOOL fEnable)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	UINT newStyle = windowStyle;

	if (FALSE == fEnable)
		newStyle &= ~TBS_BOTTOMDOCK;
	else
		newStyle |= TBS_BOTTOMDOCK;

	if(newStyle == windowStyle)
		return fEnable;
	
	if (0 != (TBS_AUTOHIDE & windowStyle))
	{
		HWND hFocus = GetFocus();
		if (hwnd == hFocus || IsChild(hwnd, hFocus))
			Toolbar_AutoHideWindow(hwnd, TRUE);
	}

	SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);

	
	HWND hParent =  GetParent(hwnd);
	if (NULL != hParent)
	{
		SetWindowPos(hParent, NULL, 0, 0, 0, 0, 
			SWP_NOSIZE |SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
	
	HWND insertAfter = HWND_TOP;
	if (0 != (TBS_BOTTOMDOCK & newStyle))
	{ 
		TOOLBAR *toolbar = GetToolbar(hwnd);
		if (NULL == toolbar || 
			0 == (TBS_AUTOHIDE & windowStyle) ||
			0 != (TBS_HIDDEN & toolbar->flags))
		{
			insertAfter = HWND_BOTTOM;
		}
	}
			
	SetWindowPos(hwnd, insertAfter, 0, 0, 0, 0,	SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);
	InvalidateRect(hwnd, NULL, TRUE);
	
	return !fEnable;
}

static LRESULT Toolbar_OnEnableAutoHide(HWND hwnd, BOOL fEnable)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return 0;

	UINT windowStyle = GetWindowStyle(hwnd);
	UINT newStyle = windowStyle;

	KillTimer(hwnd, TIMER_AUTOHIDE_ID);
	toolbar->flags &= ~TBS_HIDETIMERSET;

	if (FALSE == fEnable)
	{
		newStyle &= ~TBS_AUTOHIDE;
		toolbar->flags &= ~TBS_HIDDEN;
	}
	else
	{		
		newStyle |= TBS_AUTOHIDE;

		HWND hFocus = GetFocus();
		if (hwnd != hFocus && FALSE == IsChild(hwnd, hFocus))
			toolbar->flags |= TBS_HIDDEN;		
	}

	if(newStyle == windowStyle)
		return fEnable;

	
	SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);
	
	HWND hParent =  GetParent(hwnd);
	if (NULL != hParent)
	{
		SetWindowPos(hParent, NULL, 0, 0, 0, 0, 
			SWP_NOSIZE |SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
		
	InvalidateRect(hwnd, NULL, TRUE);
	
	return !fEnable;
}

static LRESULT Toolbar_OnEnableTabStop(HWND hwnd, BOOL fEnable)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	UINT newStyle = windowStyle;

	if (FALSE == fEnable)
		newStyle &= ~TBS_TABSTOP;
	else
		newStyle |= TBS_TABSTOP;

	if(newStyle == windowStyle)
		return fEnable;
	
	SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);
	
	Toolbar_UpdateTabstop(hwnd);

	return !fEnable;
}

static LRESULT Toolbar_OnEnableForceAddress(HWND hwnd, BOOL fEnable)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	UINT newStyle = windowStyle;

	if (FALSE == fEnable)
		newStyle &= ~TBS_FORCEADDRESS;
	else
		newStyle |= TBS_FORCEADDRESS;

	if(newStyle == windowStyle)
		return fEnable;
	
	SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL != toolbar && NULL != toolbar->items) 
	{
		size_t index = toolbar->items->size();
		while(index--)
		{
			ToolbarItem *item = toolbar->items->at(index);
			if (NULL != item && item->IsEqual(TOOLITEM_ADDRESSBAR, -1))
			{
				UINT itemStyle = item->GetStyle();
				if (FALSE != fEnable)
				{
					if (0 != (ToolbarItem::stateHidden & itemStyle))
					{
						Toolbar_LockUpdate(hwnd, TRUE);
						if (FALSE != Toolbar_ShowItem(hwnd, MAKEINTRESOURCE(index), TRUE))
						{
							size_t nextIndex = index + 1;
							if (nextIndex < toolbar->items->size())
							{
								ToolbarItem *nextItem = toolbar->items->at(nextIndex);
								if (NULL  != nextItem && nextItem->IsEqual(TOOLITEM_FLEXSPACE, -1))
								{
									Toolbar_ShowItem(hwnd, MAKEINTRESOURCE(nextIndex), FALSE);
								}
							}
						}
						Toolbar_LockUpdate(hwnd, FALSE);
						InvalidateRect(hwnd, NULL, TRUE);
					}
				}
				else 
				{
					HRESULT commandState = E_NOTIMPL;
					HWND hBrowser = GetParent(hwnd);
					ifc_omservice *service;
					if (NULL != hBrowser && FALSE != BrowserControl_GetService(hBrowser, &service))
					{
						ifc_omservicecommand *serviceCommand;
						if (SUCCEEDED(service->QueryInterface(IFC_OmServiceCommand, (void**)&serviceCommand)))
						{
							commandState = serviceCommand->QueryState(hBrowser, &CMDGROUP_ADDRESSBAR, ADDRESSCOMMAND_VISIBLE);
							serviceCommand->Release();
						}
						service->Release();
					}

					if (E_NOTIMPL == commandState)
						commandState = (0 != (TBS_SHOWADDRESS & windowStyle)) ? CMDSTATE_ENABLED : CMDSTATE_DISABLED;
					
					if (0 == (ToolbarItem::stateHidden & itemStyle) && CMDSTATE_ENABLED != commandState)
					{
						Toolbar_LockUpdate(hwnd, TRUE);
						if (FALSE != Toolbar_ShowItem(hwnd, MAKEINTRESOURCE(index), FALSE))
						{
							size_t nextIndex = index + 1;
							if (nextIndex < toolbar->items->size())
							{
								ToolbarItem *nextItem = toolbar->items->at(nextIndex);
								if (NULL  != nextItem && nextItem->IsEqual(TOOLITEM_FLEXSPACE, -1))
								{
									Toolbar_ShowItem(hwnd, MAKEINTRESOURCE(nextIndex), TRUE);
								}
							}
						}
						Toolbar_LockUpdate(hwnd, FALSE);
						InvalidateRect(hwnd, NULL, TRUE);
					}

					
				}
				break;
			}
		}
	}
	
	return !fEnable;
}

static LRESULT Toolbar_OnEnableFancyAddress(HWND hwnd, BOOL fEnable)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	UINT newStyle = windowStyle;

	if (FALSE == fEnable)
		newStyle &= ~TBS_FANCYADDRESS;
	else
		newStyle |= TBS_FANCYADDRESS;

	if(newStyle == windowStyle)
		return fEnable;
	
	SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL != toolbar && NULL != toolbar->items) 
	{
		size_t index = toolbar->items->size();
		while(index--)
		{
			ToolbarItem *item = toolbar->items->at(index);
			if (NULL != item && item->IsEqual(TOOLITEM_ADDRESSBAR, -1))
			{
				UINT style = (FALSE != fEnable) ? 0 : ToolbarAddress::styleAddressShowReal;
				item->SetStyle(hwnd, style, ToolbarAddress::styleAddressShowReal);
				break;
			}
		}
	}
		
	return !fEnable;
}

static LRESULT Toolbar_OnSetBrowserHost(HWND hwnd, HWND hBrowser)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return 0;

	toolbar->hBrowser = hBrowser;
	return TRUE;
}


static LRESULT Toolbar_OnGetImageListHeight(HWND hwnd)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	INT iconCX, iconCY;
	if (NULL == toolbar || 
		NULL == toolbar->imageList ||
		FALSE == ImageList_GetIconSize(toolbar->imageList, &iconCX, &iconCY))
	{
		return 0;
	}
	
	return iconCY/TOOLBAR_ICONSTATE_COUNT;
}

static LRESULT Toolbar_OnGetNextTabItem(HWND hwnd, LPCSTR pszName, BOOL fPrevious)
{
	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return ITEM_ERR;

	INT index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index)
		return ITEM_ERR;

	if (0 == index && FALSE == fPrevious) 
		return ITEM_ERR;

	UINT windowStyle = GetWindowStyle(hwnd);
	BOOL fTabstopOnly = (0 == (TBS_TABSTOP & windowStyle));
	INT direction = (FALSE == fPrevious) ? 1 : -1;
	return Toolbar_GetFocusIndex(hwnd, index + direction, direction, fTabstopOnly);
}

static void Toolbar_OnCheckHide(HWND hwnd, BOOL fImmediate)
{
	Toolbar_AutoHideWindow(hwnd, fImmediate);
}

static LRESULT Toolbar_OnGetTextLength(HWND hwnd, LPCSTR pszName, size_t *textLength)
{
	if (NULL == textLength) return FALSE;
	*textLength = 0;

	TOOLBAR *toolbar = GetToolbar(hwnd);
	if (NULL == toolbar) return FALSE;

	size_t index = Toolbar_ResolveName(hwnd, pszName);
	if (ITEM_ERR == index) return FALSE;

	ToolbarItem *item = Toolbar_GetItem(toolbar, index);
	if (NULL == item) return FALSE;

	if (FAILED(item->GetTextLength(textLength)))
		return FALSE;

	return TRUE;
}

static LRESULT CALLBACK Toolbar_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:				return Toolbar_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			Toolbar_OnDestroy(hwnd); break;
		case WM_PAINT:				Toolbar_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		Toolbar_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_WINDOWPOSCHANGED:	Toolbar_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SETREDRAW:			Toolbar_OnSetRedraw(hwnd, (BOOL)wParam); return 0;
		case WM_MOUSEMOVE:			Toolbar_OnMouseMove(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_MOUSELEAVE:			Toolbar_OnMouseLeave(hwnd); return 0;
		case WM_LBUTTONDOWN:
			Toolbar_OnLButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam));
			break;
		case WM_LBUTTONUP:			Toolbar_OnLButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); break;
		case WM_RBUTTONDOWN:		Toolbar_OnRButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); break;
		case WM_RBUTTONUP:			Toolbar_OnRButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); break;
		case WM_MBUTTONDOWN:		Toolbar_OnMButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); break;
		case WM_MBUTTONUP:			Toolbar_OnMButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); break;
		case WM_CAPTURECHANGED:		Toolbar_OnCaptureChanged(hwnd, (HWND)lParam); return 0;
		case WM_GETFONT:			return Toolbar_OnGetFont(hwnd);
		case WM_SETFONT:			Toolbar_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam)); return 0;
		case WM_NOTIFY:				return Toolbar_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam);
		case WM_COMMAND:			Toolbar_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return 0;
		case WM_ENTERMENULOOP:		Toolbar_OnEnterMenuLoop(hwnd, (BOOL)wParam); return 0;
		case WM_EXITMENULOOP:		Toolbar_OnExitMenuLoop(hwnd, (BOOL)wParam); return 0;
		case WM_SETFOCUS:			Toolbar_OnSetFocus(hwnd, (HWND)wParam); return 0;
		case WM_KILLFOCUS:			Toolbar_OnKillFocus(hwnd, (HWND)wParam); return 0;
		case WM_CONTEXTMENU:		Toolbar_OnContextMenu(hwnd, (HWND)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_KEYDOWN:			Toolbar_OnKeyDown(hwnd, (INT)wParam, (UINT)lParam); return 0;
		case WM_KEYUP:				Toolbar_OnKeyUp(hwnd, (INT)wParam, (UINT)lParam); return 0;
		case WM_GETDLGCODE:			return Toolbar_OnGetDlgCode(hwnd, (INT)wParam, (MSG*)lParam);
		case WM_UPDATEUISTATE:		Toolbar_OnUpdateUiState(hwnd, LOWORD(wParam), HIWORD(wParam)); return 0;
		case WM_ENABLE:				Toolbar_OnEnable(hwnd, (BOOL)wParam); return 0;
		case WM_SETCURSOR:			return Toolbar_OnSetCursor(hwnd, (HWND)wParam, LOWORD(lParam), HIWORD(lParam));
		case WM_CTLCOLOREDIT:		return Toolbar_OnColorEdit(hwnd, (HDC)wParam, (HWND)lParam); 
		case WM_CTLCOLORSTATIC:		return Toolbar_OnColorStatic(hwnd, (HDC)wParam, (HWND)lParam); 

		case TBM_UPDATESKIN:		Toolbar_OnUpdateSkin(hwnd, (BOOL)lParam); return 0;
		case TBM_GETIDEALHEIGHT:	return Toolbar_OnGetIdealHeight(hwnd);
		case TBM_GETICONSIZE:		return Toolbar_OnGetIconSize(hwnd, (INT)wParam, (SIZE*)lParam);
		case TBM_SENDCOMMAND:		return Toolbar_OnSendCommand(hwnd, (INT)wParam);
		case TBM_DRAWICON:			return Toolbar_OnDrawIcon(hwnd, (TOOLBARDRAWICONPARAM*)lParam);
		case TBM_GETITEMCOUNT:		return Toolbar_OnGetItemCount(hwnd);
		case TBM_CLEAR:				return Toolbar_OnClear(hwnd);
		case TBM_INSERTITEM:		return Toolbar_OnInsertItem(hwnd, (TOOLBARINSERTITEM*)lParam);
		case TBM_FINDITEM:			return Toolbar_OnFindItem(hwnd, (LPCSTR)lParam);
		case TBM_REMOVEITEM:		return Toolbar_OnRemoveItem(hwnd, (LPCSTR)lParam);
		case TBM_SETITEMINT:		return Toolbar_OnSetItemInt(hwnd, (LPCSTR)lParam, (INT)wParam);
		case TBM_SETITEMSTRING:		return Toolbar_OnSetItemString(hwnd, (LPCSTR)lParam, (LPCWSTR)wParam);
		case TBM_GETBKCOLOR:		return Toolbar_OnGetBkColor(hwnd);
		case TBM_GETFGCOLOR:		return Toolbar_OnGetFgColor(hwnd);
		case TBM_GETTEXTCOLOR:		return Toolbar_OnGetTextColor(hwnd);
		case TBM_GETHILITECOLOR:	return Toolbar_OnGetHiliteColor(hwnd);
		case TBM_ENABLEITEM:		return Toolbar_OnEnableItem(hwnd,(LPCSTR)lParam, (BOOL)wParam);
		case TBM_SHOWITEM:			return Toolbar_OnShowItem(hwnd,(LPCSTR)lParam, (BOOL)wParam);
		case TBM_UPDATETIP:			Toolbar_OnUpdateTip(hwnd); return 0;
		case TBM_GETTEXTMETRICS:	return Toolbar_OnGetTextMetrics(hwnd, (TOOLBARTEXTMETRIC*)lParam);
		case TBM_GETBKBRUSH:		return Toolbar_OnGetBkBrush(hwnd);
		case TBM_LAYOUT:			return Toolbar_OnLayout(hwnd, (TOOLBARLAYOUT*)lParam);
		case TBM_NEXTITEM:			return Toolbar_OnNextItem(hwnd, (LPCSTR)lParam, (BOOL)wParam);
		case TBM_GETITEMSTYLE:		return Toolbar_OnGetItemStyle(hwnd, (LPCSTR)lParam, (UINT)wParam);
		case TBM_GETITEMCOMMAND:	return Toolbar_OnGetItemCommand(hwnd, (LPCSTR)lParam);
		case TBM_SETITEMDESCRIPTION:return Toolbar_OnSetItemDescription(hwnd, (LPCSTR)lParam, (LPCWSTR)wParam);
		case TBM_GETITEMINFO:		return Toolbar_OnGetItemInfo(hwnd, (LPCSTR)lParam, (TBITEMINFO*)wParam);
		case TBM_AUTOPOPULATE:		return Toolbar_OnAutoPopulate(hwnd, (ifc_omservice*)lParam, (UINT)wParam);
		case TBM_ENABLEBOTTOMDOCK:	return Toolbar_OnEnableBottomDock(hwnd, (BOOL)lParam);
		case TBM_ENABLEAUTOHIDE:	return Toolbar_OnEnableAutoHide(hwnd, (BOOL)lParam);
		case TBM_ENABLETABSTOP:		return Toolbar_OnEnableTabStop(hwnd, (BOOL)lParam);
		case TBM_ENABLEFORCEADDRESS: return Toolbar_OnEnableForceAddress(hwnd, (BOOL)lParam);
		case TBM_ENABLEFANCYADDRESS: return Toolbar_OnEnableFancyAddress(hwnd, (BOOL)lParam);
		case TBM_SETBROWSERHOST:	return Toolbar_OnSetBrowserHost(hwnd, (HWND)lParam);
		case TBM_GETEDITCOLOR:		return Toolbar_OnGetEditColor(hwnd);
		case TBM_GETEDITBKCOLOR:	return Toolbar_OnGetEditBkColor(hwnd);
		case TBM_GETIMAGELISTHEIGHT:return Toolbar_OnGetImageListHeight(hwnd);
		case TBM_GETNEXTTABITEM:	return Toolbar_OnGetNextTabItem(hwnd, (LPCSTR)lParam, (BOOL)wParam);
		case TBM_CHECKHIDE:			Toolbar_OnCheckHide(hwnd, (BOOL)lParam); return 0;
		case TBM_GETTEXTLENGTH:		return Toolbar_OnGetTextLength(hwnd, (LPCSTR)lParam, (size_t*)wParam);
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK Toolbar_MouseHook(INT code, WPARAM wParam, LPARAM lParam)
{
	TOOLBARMOUSEHOOK *hook = (TOOLBARMOUSEHOOK*)Plugin_TlsGetValue(tlsIndex);
	if (NULL == hook || NULL == hook->hHook) return FALSE;

	
	if (code >= 0)
	{
		MOUSEHOOKSTRUCT *mouseHook = (MOUSEHOOKSTRUCT*)lParam;
		if (mouseHook->hwnd != hook->hwnd)
		{
			TOOLBAR *toolbar = GetToolbar(hook->hwnd);
			if (NULL != toolbar) 
			{
				if (0 != (TBS_HIDDEN & toolbar->flags))
				{
					Toolbar_RemoveMouseHook(hook->hwnd, TRUE);
				}
				else if (0 == ((TBS_MENULOOP | TBS_HIDETIMERSET) & toolbar->flags))
				{
					HWND hAncestor = GetAncestor(mouseHook->hwnd, GA_ROOT);
					BOOL autoHide;
					if (NULL == hAncestor || FALSE == IsChild(hAncestor, hook->hwnd))
					{
						autoHide = TRUE;
					}
					else
					{
						RECT toolbarRect;
						if (!GetWindowRect(hook->hwnd, &toolbarRect))
							SetRectEmpty(&toolbarRect);
						toolbarRect.top -= 6;
						toolbarRect.bottom += 6;
						autoHide = !PtInRect(&toolbarRect, mouseHook->pt);
					}

					if (FALSE != autoHide)
						Toolbar_AutoHideWindow(hook->hwnd, FALSE);
				}
			}
		}
	}		
	return CallNextHookEx(hook->hHook, code, wParam, lParam);
}