#include "api__gen_ml.h"
#include "main.h"
#include "./skinnedmenuwnd.h"
#include "./skinnedmenu.h"
#include "./skinning.h"

#include "./ml_imagelist.h"
#include "./colors.h"
#include "./resource.h"

#include "../winamp/wa_dlg.h"


#define MENU_BORDER_WIDTH	3

static HMLIMGLST hmlilCheck = NULL;
static INT imageCheckMark = -1;
static INT imageRadioMark = -1;
static INT imageExpandArrow = -1;

// menu hit test codes
#define MHF_NOWHERE		0xFFFFFFFF
#define MHF_SCROLLUP	0xFFFFFFFD
#define MHF_SCROLLDOWN	0xFFFFFFFC

#define MN_SIZEWINDOW		0x01E2
#define MN_SELECTITEM		0x01E5	// wParam - item position or MHF_XXX
#define MN_LBUTTONDOWN		0x01ED	// wParam - item position or MHF_XXX
#define MN_LBUTTONUP		0x01EF	// wParam - item position or MHF_XXX
#define MN_LBUTTONDBLCLK	0x01F1	// ?

// menu timer id
#define MTID_OPENSUBMENU	0x0000FFFE
#define MTID_SCROLLUP		0xFFFFFFFD
#define MTID_SCROLLDOWN		0xFFFFFFFC


#define MTID_EX_UNBLOCKDRAW	0x0001980

extern HMLIMGFLTRMNGR hmlifMngr;	// default gen_ml fitler manager

#define SMIF_BLOCKDRAW			0x00000001
#define SMIF_REMOVEREFLECTOR	0x00000002

static HBRUSH SkinnedMenuWnd_GetBackBrush(HMENU hMenu)
{
	MENUINFO mi = {0};
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_BACKGROUND;
	if (NULL == hMenu || !GetMenuInfo(hMenu, &mi))
		mi.hbrBack = NULL;
	return (NULL != mi.hbrBack) ? mi.hbrBack : GetSysColorBrush(COLOR_MENU);
}

static INT SkinnedMenuWnd_AddPngResource(HMLIMGLST imageList, INT resourceId)
{
	MLIMAGESOURCE_I src;
	ZeroMemory(&src, sizeof(MLIMAGESOURCE_I));
	src.type = SRC_TYPE_PNG_I;
	src.hInst = plugin.hDllInstance;
	src.lpszName = MAKEINTRESOURCEW(resourceId);
	return MLImageListI_Add(hmlilCheck, &src, MLIF_FILTER1_UID, 0);
}

static HBITMAP SkinnedMenuWnd_LoadPngResource(INT resourceId, COLORREF rgbBk, COLORREF rgbFg)
{
	MLIMAGESOURCE_I imageSource;
	ZeroMemory(&imageSource, sizeof(MLIMAGESOURCE_I));
	imageSource.type = SRC_TYPE_PNG_I;
	imageSource.hInst = plugin.hDllInstance;
	imageSource.lpszName = MAKEINTRESOURCEW(resourceId);
	HBITMAP hbmp = MLImageLoaderI_LoadDib(&imageSource);
	if (NULL != hbmp)
		MLImageFilterI_Apply(hmlifMngr, &MLIF_FILTER1_UID, hbmp, rgbBk, rgbFg, NULL); 
	return hbmp;
}

SkinnedMenuWnd::SkinnedMenuWnd(UINT menuExStyle, HMLIMGLST hmlil, INT forcedWidth, MENUCUSTOMIZEPROC _customProc, ULONG_PTR customParam) : 
	SkinnedWnd(FALSE)
{
	if (FAILED(SkinnedMenuThreadInfo::GetInstance(TRUE, &threadInfo)))
		threadInfo = NULL;

	hMenu = NULL;
	hOwner = NULL;
	this->menuExStyle	= menuExStyle;
	this->hmlil			= hmlil;
	this->lineWidth		= forcedWidth;
	bRestoreShadow		= FALSE;
	hBoldFont = NULL;

	menuFlags = 0;

	backBrush = NULL;
	borderPen = NULL;

	menuOrigBrush	= NULL;
	skinnedItems	= NULL;
	skinnedItemCount = 0;
	skinnedItemCursor = 0;
	prevSelectedItem = 0;

	shortcutCX = 0;
	textCX = 0;

	scrollBitmap = NULL;
	disabledScrollBitmap = NULL;

	this->customProc = _customProc;
	this->customParam = customParam;
}

SkinnedMenuWnd::~SkinnedMenuWnd(void)
{
	SetOwnerWindow(NULL);

	if (hMenu)
	{
		MENUINFO mi = {0};
		mi.cbSize = sizeof(MENUINFO);
		mi.fMask = MIM_BACKGROUND;
		if (GetMenuInfo(hMenu, &mi))
		{
			mi.fMask = 0;
			if (menuOrigBrush != mi.hbrBack)
			{
				mi.hbrBack = menuOrigBrush;
				mi.fMask |= MIM_BACKGROUND;
			}

			if (0 != mi.fMask)
				SetMenuInfo(hMenu, &mi);
		}

		if (NULL != skinnedItems && skinnedItemCount > 0)
		{
			MENUITEMINFOW mii = {0};
			mii.cbSize = sizeof(MENUITEMINFOW);

			for (INT i = 0; i < skinnedItemCount; i++)
			{
				SkinnedItemRecord *record = &skinnedItems[i];
				if(FALSE == record->failed)
				{
					mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_BITMAP;
					if (FALSE != GetMenuItemInfoW(hMenu, i, TRUE, &mii))
					{
						mii.fMask = 0;
						if (FALSE != record->skinned)
						{
							mii.fMask |= (MIIM_FTYPE | MIIM_BITMAP);
							mii.fType &= ~MFT_OWNERDRAW;
							record->skinned = FALSE;
						}

						if (record->itemId != record->originalId && 
							record->itemId == mii.wID)
						{
							mii.fMask |= MIIM_ID;
							mii.wID = record->originalId;
						}

						if (NULL != threadInfo)
						{
							threadInfo->ReleaseId(record->itemId);
							if (record->itemId != record->originalId)
								threadInfo->ReleaseId(record->originalId);
						}

						if (0 != mii.fMask)
						{
							if (FALSE == SetMenuItemInfoW(hMenu, i, TRUE, &mii))
							{
							}
						}
					}
				}
			}
		}

		if (NULL != threadInfo)
			threadInfo->UnregisterMenu(hMenu);
	}

	if (NULL != skinnedItems)
		free(skinnedItems);

    if (hwnd && bRestoreShadow) 
	{
		SetClassLongPtrW(hwnd, GCL_STYLE, GetClassLongPtrW(hwnd, GCL_STYLE) | 0x00020000/*CS_DROPSHADOW*/);
	}

	if (NULL != hBoldFont) 
		DeleteObject(hBoldFont);

	if (NULL != backBrush)
		DeleteObject(backBrush);

	if (NULL != borderPen)
		DeleteObject(borderPen);

	if (NULL != scrollBitmap)
		DeleteObject(scrollBitmap);

	if (NULL != disabledScrollBitmap)
		DeleteObject(disabledScrollBitmap);

	if (NULL != threadInfo)
	{
		threadInfo->RemoveValidationHook(this);
		threadInfo->Release();
	}
}

HMENU SkinnedMenuWnd::GetMenuHandle()
{
	return hMenu;
}

HWND SkinnedMenuWnd::GetOwnerWindow()
{
	return hOwner;
}

HWND SkinnedMenuWnd::SetOwnerWindow(HWND hwndOwner)
{
	if (hOwner == hwndOwner)
		return hOwner;

	HWND prevOwner = hOwner;

	if (NULL != hOwner && 
		0 != (SMIF_REMOVEREFLECTOR & menuFlags))
	{
		RemoveReflector(hOwner);
	}

	menuFlags &= ~SMIF_REMOVEREFLECTOR;

	hOwner = hwndOwner;

	if (NULL != hOwner && 
		S_OK == InstallReflector(hOwner))
	{
		menuFlags |= SMIF_REMOVEREFLECTOR;
	}

	return prevOwner;
}

BOOL SkinnedMenuWnd::AttachMenu(HMENU hMenuToAttach)
{
	MENUINFO mi = {0};
	MENUITEMINFOW mii = {0};

	if (NULL != hMenu || 
		NULL == hMenuToAttach) 
	{
		return FALSE;
	}

	hMenu = hMenuToAttach;

	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_BACKGROUND;

	if (GetMenuInfo(hMenu, &mi))
	{
		menuOrigBrush = mi.hbrBack;
		mi.fMask = 0;

		if (NULL == mi.hbrBack)
		{
			COLORREF rgb;

			if (0 != (SMS_SYSCOLORS & menuExStyle) || 
				FAILED(MLGetSkinColor(MLSO_MENU, MP_BACKGROUND, MBS_NORMAL, &rgb)))
			{
				rgb = GetSysColor(COLOR_MENU);
			}

			backBrush = CreateSolidBrush(rgb);

			mi.hbrBack = backBrush;
			mi.fMask |= MIM_BACKGROUND;
		}

		if (0 != mi.fMask)
			SetMenuInfo(hMenu, &mi);
	}

	if (NULL != threadInfo)
		threadInfo->RegisterMenu(hMenu, hwnd);

	mii.cbSize = sizeof(MENUITEMINFOW);

	INT count = GetMenuItemCount(hMenu);

	if (count > 0)
	{
		skinnedItems = (SkinnedItemRecord*)calloc(count, sizeof(SkinnedItemRecord));
		if (NULL != skinnedItems)
		{
			skinnedItemCount = count;
		}
	}

	if (NULL == skinnedItems)
		return FALSE;

	skinnedItemCursor = 0;

	for (int i = 0; i < count; i++)
	{
		SkinnedItemRecord *record = &skinnedItems[i];
		mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_BITMAP;  // MIIM_BITMAP - keep it... this forces menu to call WM_MEASUREITEM
		if (FALSE != GetMenuItemInfoW(hMenu, i, TRUE, &mii))
		{			
			record->originalId = mii.wID;
			record->itemId = record->originalId;

			if (NULL != threadInfo)
				threadInfo->ClaimId(record->originalId);

			if (0 == (MFT_OWNERDRAW & mii.fType))
			{
				mii.fType |= MFT_OWNERDRAW;
				mii.fMask &= ~MIIM_ID;

				// copes with separators and popup menus (menu and menuex types)
				if (0 == mii.wID || (UINT)-1 == mii.wID || 65535 == mii.wID)
				{
					if (NULL != threadInfo)
					{
						mii.wID = threadInfo->GetAvailableId();
						if ((unsigned int)-1 != mii.wID)
						{
							record->itemId = mii.wID;
							mii.fMask |= MIIM_ID;
						}
						else
							mii.wID = record->itemId;
					}
				}

				record->skinned = TRUE;
				if (FALSE == SetMenuItemInfoW(hMenu, i, TRUE, &mii))
				{
					record->skinned = FALSE;
					record->itemId = record->originalId;
				}
				else
				{
					if (record->itemId != record->originalId && 
						NULL != threadInfo)
					{
						threadInfo->ClaimId(record->itemId);
					}
				}
			}
		}
		else
		{
			record->failed = TRUE;
		}
	}

	return TRUE;
}

BOOL SkinnedMenuWnd::Attach(HWND hwndMenu, HWND hwndOwner)
{		
	menuFlags &= ~SMIF_BLOCKDRAW;
	
	if(!__super::Attach(hwndMenu)) 
		return FALSE;

	SetOwnerWindow(hwndOwner);

	DWORD windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
	DWORD windowStyleEx = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
	DWORD newStyle = windowStyle & ~(WS_BORDER | WS_THICKFRAME | WS_DLGFRAME);
	if (newStyle != windowStyle)
		SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);

	newStyle = windowStyleEx & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	if (newStyle != windowStyleEx)
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, newStyle);

	if (0 == (SMS_SYSCOLORS & menuExStyle))
	{
		SetStyle(SWS_USESKINCOLORS, FALSE);
	}

	SetType(SKINNEDWND_TYPE_POPUPMENU);

	if (!hmlilCheck)
		hmlilCheck = MLImageListI_Create(16, 16, MLILC_COLOR24_I, 2, 1, 2, hmlifMngr);

	if ((SMS_FORCEWIDTH & menuExStyle) && lineWidth > 0) 
	{
		lineWidth -= ((GetSystemMetrics(SM_CXMENUCHECK) - 1) + WASABI_API_APP->getScaleX(MENU_BORDER_WIDTH*2));
		if (lineWidth < 0) lineWidth = 0;
	}
	else lineWidth = 0;

	lineHeight = GetLineHeight();
	if (!hmlil || !MLImageListI_GetImageSize(hmlil, &imageWidth, &imageHeight)) { imageWidth = WASABI_API_APP->getScaleX(24); imageHeight = 0; }
	if (hmlilCheck)
	{
		INT imageCX, imageCY;
		if(MLImageListI_GetImageSize(hmlilCheck, &imageCX, &imageCY)) 
		{
			if (imageWidth < imageCX) imageWidth = imageCX;
			if (imageWidth < WASABI_API_APP->getScaleX(25)) imageWidth = WASABI_API_APP->getScaleX(25);	// clamp to a min width to better match the OS
			if (imageHeight < imageCY) imageHeight = imageCY;
		}
	}
	if (lineHeight < (imageHeight + WASABI_API_APP->getScaleY(4))) lineHeight = (imageHeight + WASABI_API_APP->getScaleY(4));

	if ((SMS_DISABLESHADOW & menuExStyle))
	{
		UINT cs = GetClassLongPtrW(hwnd, GCL_STYLE);
		if (0x00020000/*CS_DROPSHADOW*/ & cs)
		{
			bRestoreShadow = TRUE;
			SetClassLongPtrW(hwnd, GCL_STYLE, cs & ~0x00020000/*CS_DROPSHADOW*/);
		}
	}
	return TRUE;
}

HPEN SkinnedMenuWnd::GetBorderPen(void)
{
	if (NULL == borderPen)
	{
		COLORREF rgb;
		if (0 != (SMS_SYSCOLORS & menuExStyle))
			rgb = GetSysColor(COLOR_GRAYTEXT);
		else
			MLGetSkinColor(MLSO_MENU, MP_FRAME, 0, &rgb);
		borderPen = CreatePen(PS_SOLID, 0, rgb);
	}

	return borderPen;
}

INT SkinnedMenuWnd::GetLineHeight()
{
	INT h = 0;
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (hdc)
	{
		HFONT hf = GetMenuFont(TRUE);
		if (NULL != hf)
		{
			TEXTMETRICW tm = {0};
			HFONT hfo = (HFONT)SelectObject(hdc, hf);
			if (GetTextMetricsW(hdc, &tm)) h = tm.tmHeight + WASABI_API_APP->getScaleY(4);
			SelectObject(hdc, hfo);
		}
		ReleaseDC(hwnd, hdc);
	}
	return h;
}

HFONT SkinnedMenuWnd::GetMenuFont(BOOL fBold)
{
	HFONT hFont = NULL;
	if (SMS_USESKINFONT & menuExStyle) 
	{
		hFont = (HFONT)MlStockObjects_Get(SKIN_FONT);
	}

	if (NULL == hFont)
		hFont = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
	
	if (FALSE != fBold)
	{
		if (NULL == hBoldFont)
		{
			LOGFONTW lf = {0};
			if (sizeof(LOGFONTW) == GetObjectW(hFont, sizeof(LOGFONTW), &lf))
			{
				if (lf.lfWeight < FW_BOLD)
					lf.lfWeight = FW_BOLD;
				hBoldFont = CreateFontIndirectW(&lf);
			}
		}

		if (NULL != hBoldFont)
		{
			hFont = hBoldFont;
		}
	}

	return hFont;
}

INT SkinnedMenuWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *pncsp)
{
	DWORD windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	DWORD windowStyleEx = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	DWORD newStyle = windowStyle & ~(WS_BORDER | WS_THICKFRAME | WS_DLGFRAME);
	if (newStyle != windowStyle)
		SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);

	newStyle = windowStyleEx & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	if (newStyle != windowStyleEx)
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, newStyle);

	LRESULT result = CallPrevWndProc(WM_NCCALCSIZE, (WPARAM)bCalcValidRects, (LPARAM)pncsp);
	
	InflateRect(&pncsp->rgrc[0], WASABI_API_APP->getScaleX(-MENU_BORDER_WIDTH), WASABI_API_APP->getScaleY(-MENU_BORDER_WIDTH)); 
	if (bCalcValidRects)
	{
		InflateRect(&pncsp->rgrc[1], WASABI_API_APP->getScaleX(-MENU_BORDER_WIDTH), WASABI_API_APP->getScaleY(-MENU_BORDER_WIDTH)); 
		InflateRect(&pncsp->rgrc[2], WASABI_API_APP->getScaleX(-MENU_BORDER_WIDTH), WASABI_API_APP->getScaleY(-MENU_BORDER_WIDTH)); 
	}

	return (INT)result;
}

void SkinnedMenuWnd::PaintScrollButton(HDC hdc, const RECT *prc, UINT scrollButton, BOOL buttonState)
{
	COLORREF rgbBk, rgbFg;

	MLGetSkinColor(MLSO_MENU, MP_BACKGROUND, MBS_NORMAL, &rgbBk);
	MLGetSkinColor(MLSO_MENU, MP_TEXT, MTS_NORMAL, &rgbFg);

	HBITMAP hbmp;

	if (0 == (MENU_BUTTON_STATE_DISABLED & buttonState))
	{
		if (NULL == scrollBitmap)
			scrollBitmap = SkinnedMenuWnd_LoadPngResource(IDB_MENU_SCROLLARROW, rgbBk, rgbFg);
		hbmp = scrollBitmap;
	}
	else
	{
		if (NULL == disabledScrollBitmap)
			disabledScrollBitmap = SkinnedMenuWnd_LoadPngResource(IDB_MENU_SCROLLARROW_DISABLED, rgbBk, rgbFg);
		hbmp = disabledScrollBitmap;
	}

	BOOL imageFailed = TRUE;
	if (NULL != hbmp)
	{
		DIBSECTION bitmapInfo;
		if (!GetObjectW(hbmp, sizeof(bitmapInfo), &bitmapInfo))
			ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));

		INT h = abs(bitmapInfo.dsBm.bmHeight);
		INT w = bitmapInfo.dsBm.bmWidth;
		if (h > 0 && w > 0)
		{
			INT x, y, nWidth, nHeight;
			x = prc->left + ((prc->right - prc->left) - w) / 2;
			y = prc->top + ((prc->bottom - prc->top) - h) / 2;
			if (MENU_BUTTON_SCROLLDOWN == scrollButton)
			{
				nWidth = -w;
				nHeight = h;
				x += w;
			}
			else
			{
				nWidth = w;
				nHeight = -h;
				y += h;
			}

			SetBkColor(hdc, rgbBk);
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prc, NULL, 0, NULL);
			StretchDIBits(hdc, x, y, nWidth, nHeight, 0, 0, w, h, bitmapInfo.dsBm.bmBits, 
							(BITMAPINFO*)&bitmapInfo.dsBmih, DIB_RGB_COLORS, SRCCOPY);
			imageFailed = FALSE;
		}
	}

	if (imageFailed)
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prc, NULL, 0, NULL);
}

BOOL SkinnedMenuWnd::DrawScrollButton(HDC hdc, UINT scrollButton)
{
	RECT rc, rcWindow, rcPaint;

	if (0 == scrollButton || 
		0 == GetWindowRect(hwnd, &rcWindow) || 
		0 == GetClientRect(hwnd, &rc))
	{
		return FALSE;
	}

	MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&rc, 2);
		
	HDC hdcOwned = NULL;
	if (NULL == hdc)
	{
		hdcOwned = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_WINDOW | DCX_CLIPSIBLINGS);
		hdc = hdcOwned;
		
		if (NULL == hdcOwned)
			return FALSE;
	}

	rcPaint.left = rc.left - rcWindow.left;
	rcPaint.right = rc.right - rcWindow.left;

	BOOL scrollEnabled;

	POINT ptTest;
	ptTest.x = rc.left + (rc.right - rc.left) / 2;

	if (0 != (MENU_BUTTON_SCROLLUP & scrollButton))
	{
		rcPaint.top = MENU_BORDER_WIDTH;
		rcPaint.bottom = rc.top - rcWindow.top;

		if (rcPaint.bottom > rcPaint.top && rcPaint.right > rcPaint.left)
		{
			ptTest.y = rc.top;
			scrollEnabled = (MenuItemFromPoint(hwnd, hMenu, ptTest) > 0);
			PaintScrollButton(hdc, &rcPaint, MENU_BUTTON_SCROLLUP, (scrollEnabled) ? 0 : MENU_BUTTON_STATE_DISABLED);
		}
	}

	if (0 != (MENU_BUTTON_SCROLLDOWN & scrollButton))
	{
		rcPaint.top = rc.bottom - rcWindow.top;
		rcPaint.bottom = (rcWindow.bottom - rcWindow.top) - MENU_BORDER_WIDTH;

		if (rcPaint.bottom > rcPaint.top && rcPaint.right > rcPaint.left)
		{
			ptTest.y = rc.bottom - WASABI_API_APP->getScaleY(1);
			INT last = MenuItemFromPoint(hwnd, hMenu, ptTest);
			scrollEnabled = FALSE;
			if (last >= 0)
			{
				INT count = GetMenuItemCount(hMenu);
				if (last != (count - 1))
					scrollEnabled = TRUE;
			}
			PaintScrollButton(hdc, &rcPaint, MENU_BUTTON_SCROLLDOWN, (scrollEnabled) ? 0 : MENU_BUTTON_STATE_DISABLED);
		}
	}
	if (NULL != hdcOwned)
		ReleaseDC(hwnd, hdcOwned);

	return TRUE;
}

void SkinnedMenuWnd::DrawBorder(HDC hdc)
{
	RECT rc, rcWindow, rp;
	GetClientRect(hwnd, &rc);
	GetWindowRect(hwnd, &rcWindow);

	MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&rc, 2);

	OffsetRect(&rc, -rcWindow.left, -rcWindow.top);
	OffsetRect(&rcWindow, -rcWindow.left, -rcWindow.top);

	SkinnedWnd::DrawBorder(hdc, &rcWindow, BORDER_FLAT, GetBorderPen());

	HBRUSH brushBk = SkinnedMenuWnd_GetBackBrush(hMenu);

	SetRect(&rp, rcWindow.left + 1, rcWindow.top + 1, rc.left, rcWindow.bottom - 1); 
	FillRect(hdc, &rp, brushBk);
	SetRect(&rp, rc.left, rcWindow.top + 1, rc.right, rc.top);
    FillRect(hdc, &rp, brushBk);
	SetRect(&rp, rc.right, rcWindow.top + 1, rcWindow.right - 1, rcWindow.bottom - 1);
    FillRect(hdc, &rp, brushBk);
	SetRect(&rp, rc.left, rc.bottom, rc.right, rcWindow.bottom - 1);
    FillRect(hdc, &rp, brushBk);

	if ((rc.top - rcWindow.top) > MENU_BORDER_WIDTH || (rcWindow.bottom - rc.bottom) > MENU_BORDER_WIDTH)
		DrawScrollButton(hdc, MENU_BUTTON_SCROLLUP | MENU_BUTTON_SCROLLDOWN);
}

BOOL SkinnedMenuWnd::IsSkinnedItem(UINT itemId)
{
	if (NULL == skinnedItems)
		return TRUE;

	if (skinnedItemCursor >= skinnedItemCount)
		skinnedItemCursor = 0;

	INT start = skinnedItemCursor;

	while(itemId != skinnedItems[skinnedItemCursor].itemId)
	{
		skinnedItemCursor++;
		if (skinnedItemCursor == skinnedItemCount)
			skinnedItemCursor = 0;
		if (skinnedItemCursor == start)
		{
			skinnedItemCursor = 0;
			return FALSE;
		}
	}

	return skinnedItems[skinnedItemCursor].skinned;
}

static void SkinnedMenuWnd_DrawFrame(HDC hdc, const RECT *prc, INT width, COLORREF rgbFrame)
{	
	if (width > 0)
	{
		COLORREF rgbOld = SetBkColor(hdc, rgbFrame);	

		RECT rcPart;
		SetRect(&rcPart, prc->left, prc->top, prc->right, prc->top + width); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		SetRect(&rcPart, prc->left, prc->bottom - width, prc->right, prc->bottom); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		SetRect(&rcPart, prc->left, prc->top + width, 	prc->left + width, prc->bottom - width); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);
		SetRect(&rcPart, prc->right - width, prc->top + width, prc->right, prc->bottom - width); 
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcPart, NULL, 0, NULL);

		if (rgbOld != rgbFrame)
			SetBkColor(hdc, rgbOld);
	}
}

BOOL SkinnedMenuWnd::OnReflectedDrawItem(DRAWITEMSTRUCT *pdis)
{
	if (0 != (SMIF_BLOCKDRAW & menuFlags))
	{
		ExcludeClipRect(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, pdis->rcItem.right, pdis->rcItem.bottom);
	}

	if (!IsSkinnedItem(pdis->itemID))
		return FALSE;

	MENUITEMINFOW mii = {0};
	wchar_t szText[256] = {0};
	INT imageCX, imageCY, realIndex;
	LONG imageTop;

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_SUBMENU;
	mii.cch = ARRAYSIZE(szText);
	mii.dwTypeData = szText;

	if (!GetMenuItemInfoW((HMENU)pdis->hwndItem, pdis->itemID, FALSE, &mii)) 
		mii.cch = 0;

	COLORREF rgbText, rgbTextBk, rgbTextFrame;
	if (0 != (SMS_SYSCOLORS & menuExStyle))
	{
		INT foreIndex = (0 != (MFS_GRAYED & mii.fState)) ? COLOR_GRAYTEXT : COLOR_MENUTEXT;
		INT backIndex = (0 != (ODS_SELECTED & pdis->itemState)) ? COLOR_HIGHLIGHT : COLOR_MENU;

		rgbText = GetSysColor(foreIndex);
		rgbTextBk = GetSysColor(backIndex);
		rgbTextFrame = rgbTextBk;
	}
	else
	{
		MLGetSkinColor(MLSO_MENU, MP_BACKGROUND, (0 != (ODS_SELECTED & pdis->itemState)) ? MBS_SELECTED : MBS_NORMAL, &rgbTextBk);
		MLGetSkinColor(MLSO_MENU, MP_TEXT, (0 != (MFS_GRAYED & mii.fState)) ? MTS_DISABLED : ((0 != (ODS_SELECTED & pdis->itemState)) ? MBS_SELECTED : MBS_NORMAL), &rgbText);
		rgbTextFrame = rgbTextBk;
		if (0 != (ODS_SELECTED & pdis->itemState))
		{
			MLGetSkinColor(MLSO_MENU, MP_BACKGROUND, MBS_SELECTEDFRAME, &rgbTextFrame);
		}
	}

	COLORREF origText = SetTextColor(pdis->hDC, rgbText);
	COLORREF origTextBk = SetBkColor(pdis->hDC, rgbTextBk);

	if (0 != (MFT_MENUBARBREAK & mii.fType))
	{
		RECT rect;
		if (FALSE != GetClientRect(hwnd, &rect))
		{
			COLORREF lineColor, prevColor;
			HBRUSH brush = SkinnedMenuWnd_GetBackBrush(hMenu);
			if(NULL == brush)
				brush = GetSysColorBrush(COLOR_WINDOW);

			rect.right = pdis->rcItem.left - WASABI_API_APP->getScaleX(1);
			rect.left = pdis->rcItem.left - WASABI_API_APP->getScaleX(2);
			FillRect(pdis->hDC, &rect, brush);

			if (0 != (SMS_SYSCOLORS & menuExStyle) || 
				FAILED(MLGetSkinColor(MLSO_MENU, MP_SEPARATOR, 0, &lineColor)))
			{
				lineColor = GetSysColor(COLOR_3DSHADOW);
			}

			prevColor = SetBkColor(pdis->hDC, lineColor);
			OffsetRect(&rect, -1, 0);
			ExtTextOut(pdis->hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
			SetBkColor(pdis->hDC, prevColor);
		}
	}

	if (NULL != customProc)
	{
		INT customResult  = customProc(MLMENU_ACTION_DRAWITEM, (HMENU)pdis->hwndItem, pdis->hDC, (LPARAM)pdis, customParam);
		if (MLMENU_WANT_DRAWPART != customResult && FALSE != customResult)
			return TRUE;
	}

	INT type = ((MFT_STRING | MFT_SEPARATOR) & mii.fType);
	switch(type)
	{
		case MFT_STRING:
			if (NULL == customProc ||
				FALSE == customProc(MLMENU_ACTION_DRAWBACK, (HMENU)pdis->hwndItem, pdis->hDC, (LPARAM)pdis, customParam))
			{
				ExtTextOutW(pdis->hDC, 0, 0, ETO_OPAQUE, &pdis->rcItem, NULL, 0, NULL);
				if (rgbTextFrame != rgbTextBk)
				{
					SkinnedMenuWnd_DrawFrame(pdis->hDC, &pdis->rcItem, 1, rgbTextFrame);
				}
			}

			if (NULL == customProc ||
				FALSE == customProc(MLMENU_ACTION_DRAWICON, (HMENU)pdis->hwndItem, pdis->hDC, (LPARAM)pdis, customParam))
			{
				if (hmlil)
				{
					if (MLImageListI_GetImageSize(hmlil, &imageCX, &imageCY))
					{
						imageTop = pdis->rcItem.top + (pdis->rcItem.bottom - pdis->rcItem.top - imageCY) / 2;
						if (imageTop < pdis->rcItem.top) imageTop = pdis->rcItem.top;
									
						INT index = MLImageListI_GetIndexFromTag(hmlil, pdis->itemID);
						if (-1 != index)
						{
							HIMAGELIST himl = MLImageListI_GetRealList(hmlil);
							realIndex = MLImageListI_GetRealIndex(hmlil, index, rgbTextBk, rgbText);
							if (-1 != realIndex)
							{
								ImageList_Draw(himl, realIndex, pdis->hDC, pdis->rcItem.left + WASABI_API_APP->getScaleX(1) + (imageWidth - imageCX) / 2, imageTop, ILD_NORMAL);
							}
						}
					}
				}
				else if ((MFS_CHECKED & mii.fState) && hmlilCheck)
				{
					if (0 != (MFT_RADIOCHECK & mii.fType))
					{
						if (-1 == imageRadioMark)
							imageRadioMark = SkinnedMenuWnd_AddPngResource(hmlilCheck, IDB_MENU_RADIOMARK);
					}
					else
					{
						if (-1 == imageCheckMark)
							imageCheckMark = SkinnedMenuWnd_AddPngResource(hmlilCheck, IDB_MENU_CHECKMARK);
					}

					if (MLImageListI_GetImageSize(hmlilCheck, &imageCX, &imageCY))
					{	
						imageTop = pdis->rcItem.top + (pdis->rcItem.bottom - pdis->rcItem.top - imageCY) / 2;
						if (imageTop < pdis->rcItem.top) imageTop = pdis->rcItem.top;
						HIMAGELIST himl = MLImageListI_GetRealList(hmlilCheck);
						realIndex = MLImageListI_GetRealIndex(hmlilCheck, (MFT_RADIOCHECK & mii.fType) ? imageRadioMark : imageCheckMark, rgbTextBk, rgbText);
						if (-1 != realIndex)
						{
							ImageList_Draw(himl, realIndex, pdis->hDC, pdis->rcItem.left + WASABI_API_APP->getScaleX(1) + (imageWidth - imageCX) / 2, imageTop, ILD_NORMAL);
						}
					}
				}
			}

			if (NULL != mii.hSubMenu && hmlilCheck) 
			{
				if (-1 == imageExpandArrow)
					imageExpandArrow = SkinnedMenuWnd_AddPngResource(hmlilCheck, IDB_MENU_EXPANDARROW);

				if (MLImageListI_GetImageSize(hmlilCheck, &imageCX, &imageCY))
				{	
					imageTop = pdis->rcItem.top + (pdis->rcItem.bottom - pdis->rcItem.top - imageCY) / 2;
					if (imageTop < pdis->rcItem.top) imageTop = pdis->rcItem.top;
					HIMAGELIST himl = MLImageListI_GetRealList(hmlilCheck);
					realIndex = MLImageListI_GetRealIndex(hmlilCheck, imageExpandArrow, rgbTextBk, rgbText);
					if (-1 != realIndex)
						ImageList_Draw(himl, realIndex, pdis->hDC, pdis->rcItem.right - imageCX - WASABI_API_APP->getScaleX(1), imageTop, ILD_NORMAL);
				}
			}

			if (NULL == customProc ||
				FALSE == customProc(MLMENU_ACTION_DRAWTEXT, (HMENU)pdis->hwndItem, pdis->hDC, (LPARAM)pdis, customParam))
			{
				if (mii.cch)
				{
					RECT rt;
					CopyRect(&rt, &pdis->rcItem);
					if (imageWidth && imageHeight) rt.left += imageWidth + WASABI_API_APP->getScaleX(6);
					rt.right -= imageWidth;

					HFONT hFont = GetMenuFont(FALSE);
					HFONT originalFont = (NULL != hFont) ? (HFONT)SelectObject(pdis->hDC, hFont) : NULL;
					INT originalBkMode = SetBkMode(pdis->hDC, TRANSPARENT);

					UINT len = mii.cch;
					if (len > 0)
					{
						while(--len > 0 && L'\t' != mii.dwTypeData[len]);
						if (0 == len)	
							len = mii.cch;
					}

					BOOL showPrefix = FALSE;
					if (!SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &showPrefix, 0))
						showPrefix = FALSE;

					if (!showPrefix && 0 == (ODS_NOACCEL & pdis->itemState))
						showPrefix = TRUE;

					UINT drawtextFlags = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOCLIP;
					if (!showPrefix)
						drawtextFlags |= 0x000100000; /*DT_HIDEPREFIX*/

					if (0 != (MFS_DEFAULT & mii.fState))
					{
						SetTextColor(pdis->hDC, BlendColors(rgbText, rgbTextBk, 110));
						OffsetRect(&rt, 1,0);
						DrawTextW(pdis->hDC, mii.dwTypeData, len, &rt, drawtextFlags);
						OffsetRect(&rt, -1,0);
						SetTextColor(pdis->hDC, rgbText);
					}

					DrawTextW(pdis->hDC, mii.dwTypeData, len, &rt, drawtextFlags);

					if (mii.cch > (len + 1)) 
					{
						len++;
						rt.left = rt.right - shortcutCX;
						SetTextColor(pdis->hDC, BlendColors(rgbText, rgbTextBk, 192));
						DrawTextW(pdis->hDC, mii.dwTypeData + len, mii.cch - len, &rt, DT_LEFT | DT_VCENTER | 0x000100000/*DT_HIDEPREFIX*/ | DT_SINGLELINE | DT_NOCLIP);
					}

					SelectObject(pdis->hDC, originalFont);

					if (TRANSPARENT != originalBkMode)
						SetBkMode(pdis->hDC, originalBkMode);
				}
			}

			ExcludeClipRect(pdis->hDC, pdis->rcItem.left + WASABI_API_APP->getScaleX(23),
							pdis->rcItem.top, pdis->rcItem.right, pdis->rcItem.bottom);
			break;

		case MFT_SEPARATOR:
			{
				COLORREF rgbSeparator;
				HPEN hPen, originalPen;
				INT y = (pdis->rcItem.top + (pdis->rcItem.bottom - pdis->rcItem.top) / 2);

				if (0 != (SMS_SYSCOLORS & menuExStyle) || 
					FAILED(MLGetSkinColor(MLSO_MENU, MP_SEPARATOR, 0, &rgbSeparator)))
				{
					rgbSeparator = GetSysColor(COLOR_3DSHADOW/*COLOR_GRAYTEXT*/);
				}

				hPen = CreatePen(PS_SOLID, 0, rgbSeparator);
				originalPen = (HPEN)SelectObject(pdis->hDC, hPen);

				ExtTextOutW(pdis->hDC, 0, 0, ETO_OPAQUE, &pdis->rcItem, NULL, 0, NULL);
				MoveToEx(pdis->hDC, pdis->rcItem.left + WASABI_API_APP->getScaleX(23), y, NULL);
				LineTo(pdis->hDC, pdis->rcItem.right, y);

				SelectObject(pdis->hDC, originalPen);
				DeleteObject(hPen);

				// draws a edge on the separator so it looks more like the OS when trying to 'fake it'
				if (0 != (SMS_SYSCOLORS & menuExStyle))
				{
					RECT bottomRect = pdis->rcItem;
					HBRUSH brush = GetSysColorBrush(COLOR_3DHIGHLIGHT);
					
					y += WASABI_API_APP->getScaleY(1);
					bottomRect.top = y;
					bottomRect.bottom = y + WASABI_API_APP->getScaleY(1);
					FillRect(pdis->hDC,&bottomRect, brush);
				}
			}
			ExcludeClipRect(pdis->hDC, pdis->rcItem.left + WASABI_API_APP->getScaleX(23),
							pdis->rcItem.top, pdis->rcItem.right, pdis->rcItem.bottom);
			break;
	}

	{
		COLORREF rgbSeparator;
		if (0 != (SMS_SYSCOLORS & menuExStyle) || 
			FAILED(MLGetSkinColor(MLSO_MENU, MP_SEPARATOR, 0, &rgbSeparator)))
		{
			rgbSeparator = GetSysColor(COLOR_3DSHADOW/*COLOR_GRAYTEXT*/);
		}

		HPEN hPen = CreatePen(PS_SOLID, 0, rgbSeparator);
		HPEN originalPen = (HPEN)SelectObject(pdis->hDC, hPen);

		MoveToEx(pdis->hDC, pdis->rcItem.left + WASABI_API_APP->getScaleX(22), pdis->rcItem.top, NULL);
		LineTo(pdis->hDC, pdis->rcItem.left + WASABI_API_APP->getScaleX(22), pdis->rcItem.top + (pdis->rcItem.bottom - pdis->rcItem.top));

		SelectObject(pdis->hDC, originalPen);
		DeleteObject(hPen);
	}

	if (origText != rgbText)
		SetTextColor(pdis->hDC, origText);
	if (origTextBk != rgbTextBk)
		SetBkColor(pdis->hDC, origTextBk);

	return TRUE;
}

BOOL SkinnedMenuWnd::OnReflectedMeasureItem(MEASUREITEMSTRUCT *pmis)
{
	pmis->itemHeight = lineHeight;
	pmis->itemWidth = lineWidth;

	if (!IsSkinnedItem(pmis->itemID))
		return FALSE;

	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (NULL == hdc) return FALSE;

	MENUITEMINFOW mii = {0};
	wchar_t szText[128] = {0};

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE;
	mii.cch = ARRAYSIZE(szText);
	mii.dwTypeData = szText;

	if (!GetMenuItemInfoW(hMenu, pmis->itemID, FALSE, &mii))
		mii.cch = 0;

	HFONT originalFont = (HFONT)SelectObject(hdc, GetMenuFont(0 != (MFS_DEFAULT & mii.fState)));

	if (NULL == customProc ||
		FALSE == customProc(MLMENU_ACTION_MEASUREITEM, hMenu, hdc, (LPARAM)pmis, customParam))
	{
		if (0 == lineWidth || 0 == lineHeight)
		{			
			INT type = ((MFT_STRING | MFT_SEPARATOR) & mii.fType);
			switch(type)
			{
				case MFT_STRING:
					if (mii.cch != 0)
					{
						SIZE sz;
						UINT len = mii.cch;
						while(--len > 0 && L'\t' != mii.dwTypeData[len]);
						if (0 == len) len = mii.cch;

						if (len != mii.cch)
						{
							szText[len] = L' ';
							if (GetTextExtentPoint32W(hdc, szText + len, mii.cch - len, &sz) &&
								shortcutCX < sz.cx)
							{
								shortcutCX = sz.cx;
							}
						}

						if (GetTextExtentPoint32W(hdc, szText, len, &sz))
						{
							if (textCX <= sz.cx) 
								textCX = sz.cx;

							if (lineHeight < sz.cy) 
								pmis->itemHeight = sz.cy + WASABI_API_APP->getScaleY(2);
							
							if (0 == lineWidth)
								pmis->itemWidth = textCX + shortcutCX + 8;
						}
					}
					if(imageHeight > (INT)(pmis->itemHeight + WASABI_API_APP->getScaleY(2))) pmis->itemHeight = imageHeight + WASABI_API_APP->getScaleY(2);
					if (0 == lineWidth && imageWidth) pmis->itemWidth += imageWidth;
					pmis->itemWidth -= (GetSystemMetrics(SM_CXMENUCHECK) - imageWidth - WASABI_API_APP->getScaleX(36));
					break;
				case MFT_SEPARATOR:
					pmis->itemHeight = WASABI_API_APP->getScaleY(7);
					break;
			}
		}
	}

	SelectObject(hdc, originalFont);
	ReleaseDC(hwnd, hdc);
	return TRUE;
}

void SkinnedMenuWnd::OnNcPaint(HRGN rgnUpdate)
{
	if (0 != (SMIF_BLOCKDRAW & menuFlags))
		return;

	UINT flags = DCX_PARENTCLIP | DCX_WINDOW | DCX_CLIPSIBLINGS |
				 DCX_INTERSECTUPDATE | DCX_VALIDATE;

	HDC hdc = GetDCEx(hwnd, ((HRGN)NULLREGION != rgnUpdate) ? rgnUpdate : NULL, flags);
	if (NULL == hdc) 
		return;

	DrawBorder(hdc);
	ReleaseDC(hwnd, hdc);
}

LRESULT SkinnedMenuWnd::OnEraseBackground(HDC hdc)
{
	HBRUSH brush;

	brush = SkinnedMenuWnd_GetBackBrush(hMenu);
	if (NULL != brush)
	{
		RECT rect;
		if (FALSE != GetClientRect(hwnd, &rect) && 
			FALSE != FillRect(hdc, &rect, brush))
		{
			return 1;
		}
	}

	return __super::WindowProc(WM_ERASEBKGND, (WPARAM)hdc, 0L);
}

void SkinnedMenuWnd::OnPrint(HDC hdc, UINT options)
{
	if (0 != (PRF_CHECKVISIBLE & options))
	{
		DWORD windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (0 == (WS_VISIBLE & windowStyle)) 
		{
			return;
		}
	}

	if (0 != (PRF_NONCLIENT & options))
	{
		DrawBorder(hdc);
	}

	if (0 == ((PRF_ERASEBKGND | PRF_CLIENT) & options))
	{
		return;
	}

	POINT ptOrig;
	RECT rc, rcWindow;

	if (GetClientRect(hwnd, &rc) &&
		GetWindowRect(hwnd, &rcWindow))
	{
		MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&rc, 2);
	}
	else
	{
		SetRectEmpty(&rc);
		SetRectEmpty(&rcWindow);
	}

	INT  clipRegionCode;
	HRGN clipRegion = CreateRectRgn(0, 0, 0, 0);
	clipRegionCode = (NULL != clipRegion) ? GetClipRgn(hdc, clipRegion) : -1;

	OffsetViewportOrgEx(hdc, rc.left - rcWindow.left, rc.top - rcWindow.top, &ptOrig);
	if (-1 != clipRegionCode)
	{
		IntersectClipRect(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top);	
	}
	OffsetRect(&rc, -rc.left, -rc.top);

	if (0 != (PRF_ERASEBKGND & options))
	{
		MENUINFO mi = {0};
		mi.cbSize = sizeof(MENUINFO);
		mi.fMask = MIM_BACKGROUND;
		HBRUSH brushBk = NULL; 
		if (GetMenuInfo(hMenu, &mi) && mi.hbrBack)
			brushBk = mi.hbrBack;

		if (NULL == brushBk)
			brushBk = GetSysColorBrush(COLOR_WINDOW);

		FillRect(hdc, &rc, brushBk);
	}

	if (0 != (PRF_CLIENT & options))
	{
		menuFlags &= ~SMIF_BLOCKDRAW;
		SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)hdc, (LPARAM)(~(PRF_NONCLIENT | PRF_ERASEBKGND) & options));
		menuFlags |= SMIF_BLOCKDRAW;
	}

	if (-1 != clipRegionCode)
	{
		SelectClipRgn(hdc, (0 != clipRegionCode) ? clipRegion : NULL);
	}
	if (NULL != clipRegion)
		DeleteObject(clipRegion);
	
	SetViewportOrgEx(hdc, ptOrig.x, ptOrig.y, NULL);
	SetTimer(hwnd, MTID_EX_UNBLOCKDRAW, 250, NULL);
}

LRESULT SkinnedMenuWnd::OnMenuSelect(UINT selectedItem)
{
	if (((UINT)-1) != selectedItem)
		menuFlags &= ~SMIF_BLOCKDRAW;

	UINT updateScroll = 0;

	if (MHF_SCROLLUP == prevSelectedItem) 
		updateScroll |= MENU_BUTTON_SCROLLUP;
	else if (MHF_SCROLLDOWN == prevSelectedItem)
		updateScroll |= MENU_BUTTON_SCROLLDOWN;

	switch(selectedItem)
	{
		case MHF_SCROLLUP:
			updateScroll |= MENU_BUTTON_SCROLLUP;
			break;
		case MHF_SCROLLDOWN:
			updateScroll |= MENU_BUTTON_SCROLLDOWN; 
			break;
	}

	RECT rc;
	GetClientRect(hwnd, &rc);
	MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&rc, 2);
	rc.top += WASABI_API_APP->getScaleY(1);
	INT item = MenuItemFromPoint(hwnd, hMenu, *((POINT*)&rc));

	LRESULT result;
	BOOL fInvalidate = FALSE;

	if (0 !=  updateScroll)
	{
		DWORD windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

		result = __super::WindowProc(MN_SELECTITEM, selectedItem, 0L);

		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
		if (MenuItemFromPoint(hwnd, hMenu, *((POINT*)&rc)) != item)
		{
			updateScroll = MENU_BUTTON_SCROLLUP | MENU_BUTTON_SCROLLDOWN;
			fInvalidate = TRUE;
		}
	}
	else
	{
		result = __super::WindowProc(MN_SELECTITEM, selectedItem, 0L);
	}

	prevSelectedItem = selectedItem;

	if (0 != updateScroll)
	{
		DrawScrollButton(NULL, updateScroll);
		if (FALSE != fInvalidate)
		{
			InvalidateRect(hwnd, NULL, FALSE);
		}
	}

	return result;
}

LRESULT SkinnedMenuWnd::CallHookedWindowProc(UINT uItem, BOOL fByPosition, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFOW mii = {0};
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_SUBMENU;
	
	if (FALSE != GetMenuItemInfoW(hMenu, uItem, fByPosition, &mii) && 
		NULL != mii.hSubMenu)
	{
		SkinnedMenu sm;
		sm.InitializeHook(NULL, (menuExStyle & ~SMS_FORCEWIDTH), hmlil, 0, customProc, customParam);
		return __super::WindowProc(uMsg, wParam, lParam);
	}
	
	return __super::WindowProc(uMsg, wParam, lParam);
}

INT SkinnedMenuWnd::FindHiliteItem(HMENU hMenu)
{
	MENUITEMINFOW mii = {0};
	mii.cbSize = sizeof(MENUITEMINFOW);
	mii.fMask = MIIM_STATE;

	INT count = GetMenuItemCount(hMenu);
	for (INT i = 0; i < count; i++)
	{
		if (0 != GetMenuItemInfoW(hMenu, i, TRUE, &mii))
		{
			if (MFS_HILITE == ((MFS_HILITE | MFS_DISABLED | MFS_GRAYED) & mii.fState))
				return i;
		}
	}
	return -1;
}

LRESULT SkinnedMenuWnd::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_ERASEBKGND:
			return OnEraseBackground((HDC)wParam);

		case REFLECTED_DRAWITEM: 
			if (OnReflectedDrawItem((DRAWITEMSTRUCT*)((REFLECTPARAM*)lParam)->lParam))
			{
				((REFLECTPARAM*)lParam)->result = TRUE; 
				return TRUE;
			}
			break;

		case REFLECTED_MEASUREITEM:
			if (OnReflectedMeasureItem((MEASUREITEMSTRUCT*)((REFLECTPARAM*)lParam)->lParam))
			{
				((REFLECTPARAM*)lParam)->result = TRUE; 
				return TRUE;
			}
			break;

		case MN_SIZEWINDOW:
			{
				BOOL validateOwner = FALSE;

				if (NULL == hMenu) 
				{
					textCX = 0;
					shortcutCX = 0;
					HMENU menuToAttach = (HMENU)SendMessageW(hwnd, MN_GETHMENU, 0, 0L);

					if (FALSE != AttachMenu(menuToAttach))
						validateOwner = TRUE;
				}

				if (NULL != threadInfo)
				{
					threadInfo->SetActiveMeasureMenu(hMenu);
					if (FALSE != validateOwner &&
						FALSE == threadInfo->SetValidationHook(this))
					{
						validateOwner = FALSE;
					}
				}

				LRESULT result = __super::WindowProc(uMsg, wParam, lParam);

				if (NULL != threadInfo)
				{
					threadInfo->SetActiveMeasureMenu(NULL);
					if (FALSE != validateOwner)
						threadInfo->RemoveValidationHook(this);
				}
				return result;
			}
			break;

		case MN_SELECTITEM:
			return OnMenuSelect((UINT)wParam);

		case MN_LBUTTONDBLCLK:
		case MN_LBUTTONDOWN:
		case MN_LBUTTONUP:
			menuFlags &= ~SMIF_BLOCKDRAW;
			switch(wParam)
			{
				case MHF_SCROLLUP:
				case MHF_SCROLLDOWN:
					{
						LRESULT result = __super::WindowProc(uMsg, wParam, lParam);
						DrawScrollButton(NULL, MENU_BUTTON_SCROLLDOWN | MENU_BUTTON_SCROLLUP);
						return result;
					}
					break;
				default:
					if (wParam >= 0)
					{
						return CallHookedWindowProc((UINT)wParam, TRUE, uMsg, wParam, lParam);
					}
					break;
			}
			break;

		case WM_TIMER:
			switch(wParam)
			{
				case MTID_OPENSUBMENU:
					{						
						POINT pt;
						INT iItem;
						if (GetCursorPos(&pt) && 
							-1 != (iItem = MenuItemFromPoint(NULL, hMenu, pt)))
						{
							CallHookedWindowProc(iItem, TRUE, uMsg, wParam, lParam);
							return 0;
						}
					}
					break;
				case MHF_SCROLLUP:
				case MHF_SCROLLDOWN:
					__super::WindowProc(uMsg, wParam, lParam);
					DrawScrollButton(NULL, MENU_BUTTON_SCROLLDOWN | MENU_BUTTON_SCROLLUP);
					return 0;
				case MTID_EX_UNBLOCKDRAW:
					KillTimer(hwnd, wParam);
					menuFlags &= ~SMIF_BLOCKDRAW;
					return 0;
			}
			break;

		case WM_KEYDOWN:
			menuFlags &= ~SMIF_BLOCKDRAW;
			switch(wParam)
			{
				case VK_RETURN:
				case VK_RIGHT:
					{
						INT iItem = FindHiliteItem(hMenu);
						if (-1 != iItem)
							return CallHookedWindowProc(iItem, TRUE, uMsg, wParam, lParam);
					}
					break;

				case VK_UP:
					{
						RECT rc;
						GetClientRect(hwnd, &rc);
						MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&rc, 2);
						rc.top += 1;

						INT iItem = MenuItemFromPoint(hwnd, hMenu, *((POINT*)&rc));
						if (iItem >= 0)
						{
							MENUITEMINFOW mii = {0};
							mii.cbSize = sizeof(MENUITEMINFOW);
							mii.fMask = MIIM_STATE;

							if (GetMenuItemInfoW(hMenu, iItem, TRUE, &mii) && 
								0 != (MFS_HILITE & mii.fState))
							{
								DWORD windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
								if (0 != (WS_VISIBLE & windowStyle))
									SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
	
								__super::WindowProc(uMsg, wParam, lParam);
	
								if (0 != (WS_VISIBLE & windowStyle))
								{
									windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
									if (0 == (WS_VISIBLE & windowStyle))
									{
										windowStyle |= WS_VISIBLE;
										SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
									}

									INT iNew = MenuItemFromPoint(hwnd, hMenu, *((POINT*)&rc));
									if (iNew != iItem)
									{
										DrawScrollButton(NULL, MENU_BUTTON_SCROLLUP | MENU_BUTTON_SCROLLDOWN);
										InvalidateRect(hwnd, NULL, FALSE);
									}
									else
									{
										int iHilite = GetMenuItemCount(hMenu);
										while(0 < iHilite--)
										{
											if (FALSE != GetMenuItemInfoW(hMenu, iHilite, TRUE, &mii) && 
												0 != (MFS_HILITE & mii.fState))
											{
												break;
											}
										}

										if (FALSE != GetMenuItemRect(hwnd, hMenu, iItem, &rc))
										{
											MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rc, 2);
											InvalidateRect(hwnd, &rc, FALSE);
										}

										if (iHilite != iItem && 
											FALSE != GetMenuItemRect(hwnd, hMenu, iHilite, &rc))
										{
											MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rc, 2);
											InvalidateRect(hwnd, &rc, FALSE);
										}
									}
								}
								return 0;								
							}
						}
					}
					break;
				case VK_DOWN:
					{
						RECT rc;
						GetClientRect(hwnd, &rc);
						MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&rc, 2);
						rc.top = rc.bottom - 1;
						INT item = MenuItemFromPoint(hwnd, hMenu, *((POINT*)&rc));
						if (item >= 0)
						{
							MENUITEMINFOW mii = {0};
							mii.cbSize = sizeof(MENUITEMINFOW);
							mii.fMask = MIIM_STATE;

							if (GetMenuItemInfoW(hMenu, item, TRUE, &mii) && 
								MFS_HILITE == ((MFS_HILITE | MFS_DISABLED | MFS_GRAYED) & mii.fState))
							{
								DWORD windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
								SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

								__super::WindowProc(uMsg, wParam, lParam);

								SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
								INT iNew = MenuItemFromPoint(hwnd, hMenu, *((POINT*)&rc));
								if (iNew != item)
								{
									DrawScrollButton(NULL, MENU_BUTTON_SCROLLUP | MENU_BUTTON_SCROLLDOWN);
									InvalidateRect(hwnd, NULL, FALSE);
								}
								return 0;								
							}
						}
					}
					break;
			}
			break;
	}

	return __super::WindowProc(uMsg, wParam, lParam);
}