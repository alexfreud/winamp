#include "./loginTab.h"
#include "./common.h"
#include "./imageLoader.h"
#include "./loginGui.h"
#include "./graphics.h"

#include "../api.h"
#include "../resource.h"
#include "../../nu/windowsTheme.h"

#include <vssym32.h>
#include <vsstyle.h>

#include <malloc.h>
#include <math.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <strsafe.h>

#define MAX_TEXT_AVECHAR_WIDTH			12
#define MAX_HELP_AVECHAR_WIDTH			28
#define IMAGE_MARGIN_CY					4
#define IMAGE_MARGIN_CX					4

#define NLTDS_FOCUSED	0x00000001
#define NLTDS_DISABLED	0x00000002
#define NLTDS_LOCKED	0x00000004


typedef struct __LOGINTABITEM
{
	LPWSTR	text;
	UINT	iImage;
	UINT	iImageActive;
	UINT	iImageDisabled;
	LPARAM	param;
	LONG	textWidth;
} LOGINTABITEM;

typedef struct __ITEMSTATECOLORTABLE
{
	COLORREF	backTop;
	COLORREF	backBottom;
	COLORREF	backAlpha;
	COLORREF	text;
	INT			frameType;
} ITEMSTATECOLORTABLE;

typedef struct __ITEMCOLORTABLE
{
	ITEMSTATECOLORTABLE normal;
	ITEMSTATECOLORTABLE normalPressed;
	ITEMSTATECOLORTABLE normalHigh;
	ITEMSTATECOLORTABLE normalDisabled;
	ITEMSTATECOLORTABLE selected;
	ITEMSTATECOLORTABLE selectedPressed;
	ITEMSTATECOLORTABLE selectedHigh;
	ITEMSTATECOLORTABLE selectedDisabled;
} ITEMCOLORTABLE;

typedef struct __COLORTABLE
{
	COLORREF	backTop;
	COLORREF	backBottom;
	COLORREF	backLine;
	COLORREF	focus;
	COLORREF	focusDash;
	ITEMCOLORTABLE item;
} COLORTABLE;


typedef struct __LOGINTAB
{
	LOGINTABITEM	**items;
	INT				itemsCount;
	INT				*order;
	INT				iSelected;
	INT				iHighlighted;
	INT				iPressed;
	INT				iFocused;
	HIMAGELIST		imageList;
	UINT			drawStyle;
	
	COLORTABLE		colors;

	HFONT			fontText;

	LONG			textHeight;
	LONG			spacing;
	RECT			margins;
	LONG			textWidthMax;

	HBITMAP			chevronImage;
	INT				chevronWidth;
	HMENU			chevronMenu;

	LONG			chevronLeft;
	LONG			visibleRight;
	INT				lastVisible;

	HBITMAP			frameBitmap;
	INT				frameHeight;
	INT				frameWidth;

	HBITMAP			itemBitmap;

	HWND			hTooltip;
	BSTR			helpText;

} LOGINTAB;

typedef struct  __CALCITEMWIDTH
{
	HDC hdc;
	HFONT font;
	HWND hwnd;
	INT textWidthMax;
	INT imageWidth;
	INT	imageHeight;
	INT itemHeight;
	INT frameWidth;
	INT dialogPt;
	HDC ownedDC;
	HFONT ownedFont;
} CALCITEMWIDTH;

#define FRAMETYPE_NONE			0
#define FRAMETYPE_SELECTED		1
#define FRAMETYPE_ACTIVE		2
#define FRAMETYPE_DISABLED		0

typedef struct __PAINTITEMPARAM
{
	HWND hwndTab;
	HDC hdc;
	const RECT *prcPaint;
	const RECT *prcClient;
	HRGN clipRgn;
	HRGN eraseRgn;
	HDC hdcSrc;
	HDC hdcItem;
} PAINTITEMPARAM;

typedef struct __GETITEMRECTPARAM
{
	INT		index;
	RECT	*rect;
} GETITEMRECTPARAM;

typedef struct __HITTESTITEMPARAM
{
	POINT	pt;
	RECT	*rect;
} HITTESTITEMPARAM;

typedef struct __UPDATELAYOUTPARAM
{ 
	INT		itemCount; 
	RECT	visibleBox;
	BOOL	chevronVisible;
	LONG	chevronLeft;
} UPDATELAYOUTPARAM;

typedef struct __CHEVRONMENUPAINTPARAM
{
	INT itemWidth;
	INT	itemHeight;
	HDC hdcSrc;
	HDC hdcItem;
	RECT ownerRect;
	HWND hwndMenu;
} CHEVRONMENUPAINTPARAM;

#define GetTab(__hwnd) ((LOGINTAB*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))

static LRESULT WINAPI LoginTab_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef INT (CALLBACK *ITEMRECTCALLBAC)(LOGINTAB* /*tab*/, LOGINTABITEM* /*item*/, INT /*iItem*/, const RECT* /*prcItem*/, ULONG_PTR /*param*/);

BOOL LoginTab_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSW wc;
	if (FALSE != GetClassInfo(hInstance, NWC_LOGINTAB, &wc))
		return TRUE;
	
	ZeroMemory(&wc, sizeof(wc));

	wc.lpszClassName = NWC_LOGINTAB;
	wc.lpfnWndProc = LoginTab_WindowProc;
	wc.style = CS_PARENTDC; 
	wc.cbWndExtra =  sizeof(LOGINTAB*);
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	
	return ( 0 != RegisterClassW(&wc));
}

HWND LoginTab_CreateWindow(UINT styleEx, LPCWSTR pszTitle, UINT style, INT x, INT y, INT cx, INT cy, HWND hParent, INT_PTR controlId)
{
	if (FALSE == LoginTab_RegisterClass(WASABI_API_ORIG_HINST))
		return FALSE;

	HWND hwnd = CreateWindowEx(styleEx, NWC_LOGINTAB, pszTitle, WS_CHILD | style, 
					x, y, cx, cy, hParent, (HMENU)controlId, WASABI_API_ORIG_HINST, NULL);

	return hwnd;
}

static BOOL LoginTab_IsLocked(HWND hwnd)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	return (0 != (NLTS_LOCKED & windowStyle));
}
static BOOL LoginTab_InitCalcItemWidth(HWND hwnd, HDC hdc, CALCITEMWIDTH *pciw)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab || NULL == pciw) return FALSE;

	pciw->hdc = hdc;
	pciw->font = tab->fontText;
	pciw->hwnd = hwnd;
	pciw->textWidthMax = tab->textWidthMax;

	if (NULL == tab->imageList || FALSE == ImageList_GetIconSize(tab->imageList, &pciw->imageWidth, &pciw->imageHeight))
	{
		pciw->imageWidth = 0;
		pciw->imageHeight = 0;
	}
	else
	{
		pciw->imageWidth += 2 * IMAGE_MARGIN_CX;
		pciw->imageHeight += 2 * IMAGE_MARGIN_CY;
	}
	
	
	RECT rect;
	GetClientRect(hwnd, &rect);
	pciw->itemHeight = tab->frameHeight * 2 + tab->textHeight + pciw->imageHeight;
	pciw->frameWidth = tab->frameWidth;
	
	pciw->dialogPt = tab->spacing;
	pciw->ownedDC = NULL;
	pciw->ownedFont = NULL;

	return TRUE;
}

static BOOL LoginTab_DestroyCalcItemWidth(CALCITEMWIDTH *pciw)
{
	if (NULL == pciw) return FALSE;
	if (NULL != pciw->ownedDC)
	{
		SelectObject(pciw->ownedDC, pciw->ownedFont);
		ReleaseDC(pciw->hwnd, pciw->ownedDC);
	}
	return TRUE;
}
static INT LoginTab_CalculateItemWidth(CALCITEMWIDTH *pciw, LOGINTABITEM *item)
{
	if (NULL == pciw || NULL == item) return 0;

	if (-1 == item->textWidth)
	{
		if (NULL != item->text && L'\0' != *(item->text))
		{
			if (NULL == pciw->hdc)
			{
				pciw->ownedDC = GetDCEx(pciw->hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
				if (NULL == pciw->ownedDC) return 0;
				pciw->ownedFont = (HFONT)SelectObject(pciw->ownedDC, pciw->font);
				pciw->hdc = pciw->ownedDC;
			}

			SIZE textSize;
			if (FALSE == GetTextExtentPoint(pciw->hdc, item->text, lstrlen(item->text), &textSize))
				return 0;

			item->textWidth = textSize.cx;
			
		}
		else
			item->textWidth = 0;
	}

	INT width = (item->textWidth > pciw->imageWidth) ? item->textWidth : pciw->imageWidth;
	if (width > pciw->textWidthMax) width = pciw->textWidthMax;

	width += 2*pciw->frameWidth; // borders
		
	if (width < pciw->itemHeight)
	{	
		INT k = (pciw->itemHeight - width)/(2*pciw->dialogPt);
		if (k > 2) k = 2;
		width += 2*k*pciw->dialogPt;
	}

	return width;

}
static INT LoginTab_EnumerateItemRects(HWND hwnd, HDC hdc, ITEMRECTCALLBAC callback, ULONG_PTR param)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab || NULL == callback) return -1;

	RECT clientRect, tabRect;
	LONG chevronLeft, limitRight;
	GetClientRect(hwnd, &clientRect);

	chevronLeft = clientRect.right - tab->chevronWidth;

	clientRect.left += tab->margins.left;
	clientRect.top += tab->margins.top;
	clientRect.right -= tab->margins.right;
	clientRect.bottom -= tab->margins.bottom;

	limitRight = (chevronLeft < clientRect.right) ? chevronLeft : clientRect.right;

	CALCITEMWIDTH calcWidth;

	if (FALSE == LoginTab_InitCalcItemWidth(hwnd, hdc, &calcWidth))
		return -1;

	SetRect(&tabRect, clientRect.left, clientRect.top, clientRect.left, clientRect.bottom);

	INT result, index, lastItem;
	lastItem = tab->itemsCount - 1;
	result = -1;
	BOOL ignoreVisibleUpdate = FALSE;

	for (index = 0; index < tab->itemsCount; index++)
	{
		tabRect.left = tabRect.right;
		if (tabRect.left != clientRect.left)
			tabRect.left += tab->spacing;

		if (tabRect.left > limitRight)
		{
			tabRect.right = clientRect.right + 1;
			break;
		}
		INT iItem = tab->order[index];
		LOGINTABITEM *item = tab->items[iItem];
		INT width = LoginTab_CalculateItemWidth(&calcWidth, item);
		if (0 == width) break;

		tabRect.right = tabRect.left + width;
		if ((index == lastItem && tabRect.right > clientRect.right) || 
			(index < lastItem && tabRect.right > limitRight))
		{
				break;
		}

		result = callback(tab, item, iItem, &tabRect, param);
		if (-1 != result) 
		{
			ignoreVisibleUpdate = TRUE;
			break;
		}
	}

	if (FALSE == ignoreVisibleUpdate)
	{
		if ((index == lastItem && tabRect.right > clientRect.right) ||
			(index < lastItem && tabRect.right > limitRight))
		{
			tab->lastVisible = (index - 1);

			SetRect(&tabRect, chevronLeft, clientRect.top, 
					clientRect.right + tab->margins.right, clientRect.bottom);

			result = callback(tab, NULL, tab->itemsCount, &tabRect, param);
		}
		else
			tab->lastVisible = lastItem;
	}

	LoginTab_DestroyCalcItemWidth(&calcWidth);
	return result;
}

static void LoginTab_NotifySelectionChanged(HWND hwnd)
{
	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL == hParent) return;
	
	NMHDR nmhdr;
	nmhdr.code = NLTN_SELCHANGE;
	nmhdr.hwndFrom = hwnd;
	nmhdr.idFrom = GetWindowLongPtr(hwnd, GWLP_ID);
	SNDMSG(hParent, WM_NOTIFY, (WPARAM)nmhdr.idFrom, (LPARAM)&nmhdr);
}

static HBITMAP LoginTab_GetItemBitmap(LOGINTAB *tab, HDC hdc, INT cx, INT cy)
{
	if (cx < 1) cx = 1;
	if (cy < 1) cy = 1;

	BITMAP bm;
	if (NULL == tab->itemBitmap || 
		sizeof(BITMAP) != GetObject(tab->itemBitmap, sizeof(BITMAP), &bm) || 
		bm.bmWidth <= cx || abs(bm.bmHeight) < cy)
	{
		if (NULL != tab->itemBitmap)
			DeleteObject(tab->itemBitmap);
		
		cx++; // need +1px to compose selection fill
		tab->itemBitmap = CreateCompatibleBitmap(hdc, cx, cy);
	}

	return tab->itemBitmap;
}
static HBITMAP LoginTab_LoadChevronImage(HWND hwnd, INT *imageWidth, INT *imageHeight)
{
	INT width, height;
	HBITMAP hbmpDst, hbmpSrc; 
	hbmpSrc = ImageLoader_LoadBitmap(WASABI_API_ORIG_HINST, 
			MAKEINTRESOURCE(IDR_ARROW_IMAGE), FALSE, &width, &height);
	
	if (NULL == hbmpSrc)
		return NULL;

	if (height < 0) height = -height;

	INT frameHeight = height/2;
	INT frameWidth = width;
	
	BITMAPINFOHEADER bhi;
	ZeroMemory(&bhi, sizeof(bhi));
	bhi.biSize = sizeof(bhi);
	bhi.biCompression = BI_RGB;
	bhi.biBitCount = 32;
	bhi.biPlanes = 1;
	bhi.biWidth = frameWidth;
	bhi.biHeight = 4 * frameHeight;
	
	UINT *pixelData;
	hbmpDst = CreateDIBSection(NULL, (LPBITMAPINFO)&bhi, DIB_RGB_COLORS, (void**)&pixelData, NULL, 0);
	if (NULL == hbmpDst)
	{
		DeleteObject(hbmpSrc);
		return NULL;
	}
	
	BOOL resultOk = FALSE;

	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
	{
		HDC hdcSrc = CreateCompatibleDC(hdc);
		HDC hdcDst = CreateCompatibleDC(hdc);
		
		if (NULL != hdcSrc && NULL != hdcDst)
		{
			HBITMAP hbmpSrcOrig = (HBITMAP)SelectObject(hdcSrc, hbmpSrc);
			HBITMAP hbmpDstOrig = (HBITMAP)SelectObject(hdcDst, hbmpDst);
			
			RECT imageRect;
			SetRect(&imageRect, 0, 0, frameWidth, frameHeight);
			
			BOOL blitFailed = FALSE;
			// normal
			if (FALSE == blitFailed &&
				FALSE != BitBlt(hdcDst, 0, 0*frameHeight, frameWidth, frameHeight, hdcSrc, 0, 0, SRCCOPY))
			{
				Image_AdjustSaturationAlpha(hbmpDst, &imageRect, -150, -100);
			}
			else blitFailed = TRUE;
			
						
			// active
			if (FALSE == blitFailed && 
				FALSE == BitBlt(hdcDst, 0, 1*frameHeight, frameWidth, frameHeight, hdcSrc, 0, 0, SRCCOPY))
			{
				blitFailed = TRUE;
			}
			
			// disabled
			if (FALSE == blitFailed && 
				FALSE != BitBlt(hdcDst, 0, 2*frameHeight, frameWidth, frameHeight, hdcSrc, 0, 0, SRCCOPY))
			{
				OffsetRect(&imageRect, 0, 2*frameHeight);
				Image_AdjustSaturationAlpha(hbmpDst, &imageRect, -(150 + 600), -(100 + 600));
			}
			else blitFailed = TRUE;
			

			// pressed
			if (FALSE == blitFailed && 
				FALSE == BitBlt(hdcDst, 0, 3*frameHeight, frameWidth, frameHeight, hdcSrc, 0, frameHeight, SRCCOPY))
			{
				blitFailed = TRUE;
			}
			
			if (FALSE == blitFailed)
			{
				SetRect(&imageRect, 0, 0, bhi.biWidth, -bhi.biHeight);
				Image_Premultiply(hbmpDst, &imageRect);
				resultOk = TRUE;
			}

			SelectObject(hdcSrc, hbmpSrcOrig);
			SelectObject(hdcDst, hbmpDstOrig);
		}
		if (NULL != hdcSrc) DeleteDC(hdcSrc);
		if (NULL != hdcDst) DeleteDC(hdcDst);
		ReleaseDC(hwnd, hdc);
	}

	
	DeleteObject(hbmpSrc);

	if (FALSE == resultOk)
	{
		DeleteObject(hbmpDst);
		hbmpDst = NULL;
	}
	else 
	{
		if (NULL != imageWidth) *imageWidth = width;
		if (NULL != imageHeight) *imageHeight = height;
	}
	
	return hbmpDst;
}
static BOOL LoginTab_GradientFillVertRect(HDC hdc, const RECT *prcFill, COLORREF rgbTop, COLORREF rgbBottom)
{
	TRIVERTEX szVertex[2];
	szVertex[0].x = prcFill->left;
	szVertex[0].y = prcFill->top;
	szVertex[0].Red = GetRValue(rgbTop) << 8;
	szVertex[0].Green = GetGValue(rgbTop) << 8;
	szVertex[0].Blue = GetBValue(rgbTop) << 8;
	szVertex[0].Alpha = 0x0000;

	szVertex[1].x = prcFill->right;
	szVertex[1].y = prcFill->bottom;
	szVertex[1].Red = GetRValue(rgbBottom) << 8;
	szVertex[1].Green = GetGValue(rgbBottom) << 8;
	szVertex[1].Blue = GetBValue(rgbBottom) << 8;
	szVertex[1].Alpha = 0x0000;

	GRADIENT_RECT szMesh[1];
	szMesh[0].UpperLeft = 0;
	szMesh[0].LowerRight = 1;

	return GdiGradientFill(hdc, szVertex, ARRAYSIZE(szVertex), szMesh, 1, GRADIENT_FILL_RECT_V);
}

static void LoginTab_EraseBkGround(HDC hdc, LOGINTAB *tab, LONG clientHeight, const RECT *prcPaint)
{
	RECT rect;
	LONG middleY = clientHeight/2;

	COLORREF rgbOrig = SetBkColor(hdc, tab->colors.backTop);
	
	CopyRect(&rect, prcPaint);
	rect.bottom = middleY;
	if (rect.top < rect.bottom)
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
		
	SetRect(&rect, prcPaint->left, middleY, prcPaint->right, clientHeight - 1);
	
	if (FALSE == LoginTab_GradientFillVertRect(hdc, &rect,  tab->colors.backTop,  tab->colors.backBottom))
	{
		SetBkColor(hdc,  tab->colors.backBottom);
		if (prcPaint->bottom < rect.bottom)
			rect.bottom = prcPaint->bottom;
		if (rect.top < rect.bottom)
			ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
	}

	if (prcPaint->bottom == clientHeight)
	{
		SetBkColor(hdc, tab->colors.backLine);
		SetRect(&rect, prcPaint->left, clientHeight - 1, prcPaint->right, clientHeight);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
	}

	SetBkColor(hdc, rgbOrig);
}

static BOOL LoginTab_PaintItemFrame(HDC hdc, LOGINTAB *tab, INT frameType, const RECT *prcItem, const RECT *prcPaint, HDC hdcSrc)
{
	if (NULL == tab->frameBitmap)
		return FALSE;
	
	INT offsetY;
	switch(frameType)
	{
		case FRAMETYPE_SELECTED:	offsetY = 0; break;
		case FRAMETYPE_ACTIVE:		offsetY = 1 * (tab->frameHeight * 2 + 1); break;
		case FRAMETYPE_DISABLED:	offsetY = 2 * (tab->frameHeight * 2 + 1); break;
		default:					return FALSE;
	}
	
	SelectObject(hdcSrc, tab->frameBitmap);

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 255;
	bf.AlphaFormat = AC_SRC_ALPHA;

	INT lineLength;

	// left-top
	GdiAlphaBlend(hdc, prcItem->left, prcItem->top, tab->frameWidth, tab->frameHeight, 
		hdcSrc, 0, offsetY + 0, tab->frameWidth, tab->frameHeight, bf); 

	// right-top
	GdiAlphaBlend(hdc, prcItem->right - tab->frameWidth, prcItem->top, tab->frameWidth, tab->frameHeight, 
		hdcSrc, tab->frameWidth + 1, offsetY + 0, tab->frameWidth, tab->frameHeight, bf); 

	// right-bottom
	GdiAlphaBlend(hdc, prcItem->right - tab->frameWidth, prcItem->bottom - tab->frameHeight, tab->frameWidth, tab->frameHeight, 
		hdcSrc, tab->frameWidth + 1, offsetY + tab->frameHeight + 1, tab->frameWidth, tab->frameHeight, bf); 

	// left-bottom
	GdiAlphaBlend(hdc, prcItem->left, prcItem->bottom - tab->frameHeight, tab->frameWidth, tab->frameHeight, 
		hdcSrc, 0, offsetY + tab->frameHeight + 1, tab->frameWidth, tab->frameHeight, bf); 
	
	lineLength = (prcItem->right - prcItem->left) - tab->frameWidth * 2;
	// top 
	GdiAlphaBlend(hdc, prcItem->left + tab->frameWidth, prcItem->top, lineLength, tab->frameHeight, 
		hdcSrc, tab->frameWidth, offsetY + 0, 1, tab->frameHeight, bf);
	// bottom
	GdiAlphaBlend(hdc, prcItem->left + tab->frameWidth, prcItem->bottom - tab->frameHeight, lineLength, tab->frameHeight, 
		hdcSrc, tab->frameWidth, offsetY + tab->frameHeight + 1, 1, tab->frameHeight, bf);

	lineLength = (prcItem->bottom - prcItem->top) - tab->frameHeight * 2;
	// left 
	GdiAlphaBlend(hdc, prcItem->left, prcItem->top + tab->frameHeight, tab->frameWidth, lineLength, 
		hdcSrc, 0, offsetY + tab->frameHeight, tab->frameWidth, 1, bf);
	// right
	GdiAlphaBlend(hdc, prcItem->right - tab->frameWidth, prcItem->top + tab->frameHeight, tab->frameWidth, lineLength, 
		hdcSrc, tab->frameWidth + 1, offsetY + tab->frameHeight, tab->frameWidth, 1, bf);
	return TRUE;
}

static BOOL LoginTab_FillItem(HDC hdc, const RECT *prcItem, const RECT *prcPaint, INT alpha, COLORREF rgbTop, COLORREF rgbBottom, INT tempX)
{
	
	RECT rect;
	SetRect(&rect, tempX, prcItem->top, tempX + 1, prcItem->bottom);
	if (rgbTop == rgbBottom || FALSE == LoginTab_GradientFillVertRect(hdc, &rect, rgbTop, rgbBottom))
	{
		COLORREF rgbOrig = SetBkColor(hdc, rgbBottom);
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
		if (rgbOrig != rgbBottom) SetBkColor(hdc, rgbOrig);
	}

	INT height = prcPaint->bottom - prcPaint->top;

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = alpha;
	bf.AlphaFormat = 0x00;
		
	BOOL result = GdiAlphaBlend(hdc, prcPaint->left, prcPaint->top, 1, height, 
				hdc, tempX, prcPaint->top, 1, height, bf);

	if (FALSE != result)
	{
		INT stretchModeOrig = SetStretchBltMode(hdc, COLORONCOLOR);
	
		result = StretchBlt(hdc, prcPaint->left + 1, prcPaint->top, prcPaint->right - prcPaint->left - 1, height, 
					hdc, prcPaint->left, prcPaint->top, 1, height, SRCCOPY);

		if (COLORONCOLOR != stretchModeOrig)
			SetStretchBltMode(hdc, stretchModeOrig);
	}

	return result;
}
static void LoginTab_DrawRect(HDC hdc, const RECT *prc)
{
	RECT rect;
	SetRect(&rect, prc->left +1, prc->top, prc->right -1, prc->top + 1);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	SetRect(&rect, prc->left + 1, prc->bottom-1, prc->right -1, prc->bottom);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	SetRect(&rect, prc->left, prc->top + 1, prc->left + 1, prc->bottom - 1);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	SetRect(&rect, prc->right - 1, prc->top + 1, prc->right, prc->bottom - 1);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

}
static BOOL LoginTab_PaintChevron(HDC hdc, LOGINTAB *tab, const RECT *prcItem, const RECT *prcPaint, HDC hdcSrc)
{
	ITEMSTATECOLORTABLE *color;
	INT iItem = tab->itemsCount;

	if (0 != ((NLTDS_LOCKED | NLTDS_DISABLED) & tab->drawStyle))
		color = &tab->colors.item.normalDisabled;
	else
	{
		if (iItem == tab->iPressed)
			color = &tab->colors.item.normalPressed;
		else if (iItem == tab->iHighlighted)
			color = &tab->colors.item.normalHigh;
		else
			color = &tab->colors.item.normal;
	}
	
	RECT rect, itemRect;
	CopyRect(&itemRect, prcItem);

	if (iItem == tab->iFocused && 0 != (NLTDS_FOCUSED & tab->drawStyle))
	{
		CopyRect(&rect, prcItem);
		InflateRect(&rect, -(tab->frameWidth - 2), -(tab->frameWidth - 2));
		COLORREF rgbBkOrig = SetBkColor(hdc, tab->colors.focus);
		COLORREF rgbTextOrig = SetTextColor(hdc, tab->colors.focusDash);
		DrawFocusRect(hdc, &rect);
		//LoginTab_DrawRect(hdc, &rect);
		if (rgbBkOrig != tab->colors.focus) SetBkColor(hdc, rgbBkOrig);
		if (rgbTextOrig != tab->colors.focusDash) SetTextColor(hdc, rgbTextOrig);
	}
		
	INT frameType = color->frameType;
	if (iItem == tab->iFocused && 0 != (NLTDS_FOCUSED & tab->drawStyle)) 
		frameType = FRAMETYPE_ACTIVE;
		

	if (FRAMETYPE_NONE != frameType &&
		FALSE != LoginTab_PaintItemFrame(hdc, tab, frameType, prcItem, prcPaint, hdcSrc))
	{
		InflateRect(&itemRect, -(tab->frameWidth -1), -(tab->frameHeight -1));
	}
	
	if (0 != color->backAlpha && FALSE != IntersectRect(&rect, &itemRect, prcPaint))
	{
		LoginTab_FillItem(hdc, &itemRect, &rect, color->backAlpha, color->backTop, color->backBottom, prcPaint->right);
	}

	

	if (NULL != tab->chevronImage)
	{
		BITMAP bm;
		if (sizeof(bm) == GetObject(tab->chevronImage, sizeof(bm), &bm))
		{
			if (bm.bmHeight < 0) bm.bmHeight = -bm.bmHeight;
			bm.bmHeight = bm.bmHeight/4;
			
			
			INT cx = bm.bmWidth;
			INT cy = bm.bmHeight;

			INT offsetY;
			if (0 != ((NLTDS_LOCKED | NLTDS_DISABLED) & tab->drawStyle))
				offsetY = 2*bm.bmHeight;
			else if (iItem == tab->iPressed) 
				offsetY = 3*bm.bmHeight;
			else if (iItem == tab->iHighlighted || iItem == tab->iSelected)
				offsetY = 1*bm.bmHeight;
			else
				offsetY = 0;
						
			INT x = prcItem->left + ((prcItem->right - prcItem->left) - cx)/2;
			if (x < prcItem->left) x = prcItem->left;

			INT y = prcItem->top + ((prcItem->bottom - prcItem->top) - cy)/2;
			if (y < prcItem->top) y = prcItem->top;

			if ((x + cx) > prcItem->right) cx = prcItem->right - x;
			if ((y + cy) > prcItem->bottom) cy = prcItem->bottom - y;
			
			if (iItem == tab->iPressed) y++;
						
			SelectObject(hdcSrc, tab->chevronImage);

			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = 255;
			bf.AlphaFormat = AC_SRC_ALPHA;

			GdiAlphaBlend(hdc, x, y, cx, cy, hdcSrc, 0, offsetY, bm.bmWidth, bm.bmHeight, bf);
		}
	}

	return TRUE;
}
static void LoginTab_ResolveImageIndex(HWND hwnd, HIMAGELIST imageList, LOGINTABITEM *item, INT iItem, UINT requestMask)
{
	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL == hParent) return;

	NMLOGINTABIMAGE request;
	request.hdr.code = NLTN_GETITEMIMAGE;
	request.hdr.hwndFrom = hwnd;
	request.hdr.idFrom = GetWindowLongPtr(hwnd, GWLP_ID);
	request.iItem = iItem;
	request.param = item->param;
	request.imageList = imageList;
	request.maskRequest = requestMask;
	request.maskUpdate  = 0;
	request.iImage = item->iImage;
	request.iImageActive = item->iImageActive;
	request.iImageDisabled = item->iImageDisabled;
		
	SNDMSG(hParent, WM_NOTIFY, (WPARAM)request.hdr.idFrom, (LPARAM)&request);

	if (0 != request.maskUpdate)
	{
		if (0 != (NLTIF_IMAGE & request.maskUpdate))
			item->iImage = request.iImage;

		if (0 != (NLTIF_IMAGE_ACTIVE & request.maskUpdate))
			item->iImageActive = request.iImageActive;

		if (0 != (NLTIF_IMAGE_DISABLED & request.maskUpdate))
			item->iImageDisabled = request.iImageDisabled;
	}

}
static UINT LoginTab_GetImageIndex(HWND hwnd, HIMAGELIST imageList, LOGINTABITEM *item, INT iItem, UINT imageType)
{
	if (NULL == item) return NLTM_IMAGE_NONE;
	switch(imageType & NLTIF_IMAGE_MASK)
	{		
		case NLTIF_IMAGE_ACTIVE:
			if (NLTM_IMAGE_CALLBACK == item->iImageActive)
			{
				LoginTab_ResolveImageIndex(hwnd, imageList, item, iItem, NLTIF_IMAGE_ACTIVE);
				if (NLTM_IMAGE_CALLBACK == item->iImageActive)
					break;
			}
		
			if (NLTM_IMAGE_NONE != item->iImageActive)
				return item->iImageActive;
			
			break;
		case NLTIF_IMAGE_DISABLED:
			if (NLTM_IMAGE_CALLBACK == item->iImageDisabled)
			{
				LoginTab_ResolveImageIndex(hwnd, imageList, item, iItem, NLTIF_IMAGE_DISABLED);
				if (NLTM_IMAGE_CALLBACK == item->iImageDisabled)
					break;
			}

			if (NLTM_IMAGE_NONE != item->iImageDisabled)
				return item->iImageDisabled;
			
			break;
	}
	
	if (NLTM_IMAGE_CALLBACK == item->iImage)
		LoginTab_ResolveImageIndex(hwnd, imageList, item, iItem, NLTIF_IMAGE);

	return item->iImage;
}

static BOOL LoginTab_PaintItem(HWND hwndTab, HDC hdc, LOGINTAB *tab, LOGINTABITEM *item, INT iItem, const RECT *prcItem, const RECT *prcPaint, HDC hdcSrc)
{
	
	ITEMSTATECOLORTABLE *color;
	UINT imageType;

	if (iItem == tab->iSelected)
	{
		if (0 != (NLTDS_DISABLED & tab->drawStyle))
		{
			color = &tab->colors.item.selectedDisabled;
			imageType = NLTIF_IMAGE_DISABLED;
		}
		else 
		{
			if (0 != (NLTDS_LOCKED & tab->drawStyle))
			{
				color = &tab->colors.item.selected;
			}
			else
			{
				if (iItem == tab->iPressed)
					color = &tab->colors.item.selectedPressed;
				else if (iItem == tab->iHighlighted)
					color = &tab->colors.item.selectedHigh;
				else
					color = &tab->colors.item.selected;
			}

			imageType = NLTIF_IMAGE_ACTIVE;
		}
	}
	else
	{
		if (0 != ((NLTDS_DISABLED | NLTDS_LOCKED) & tab->drawStyle))
		{
			color = &tab->colors.item.normalDisabled;
			imageType = NLTIF_IMAGE_DISABLED;
		}
		else if (iItem == tab->iPressed)
		{
			color = &tab->colors.item.normalPressed;
			imageType = NLTIF_IMAGE_ACTIVE;
		}
		else if (iItem == tab->iHighlighted)
		{
			color = &tab->colors.item.normalHigh;
			imageType = NLTIF_IMAGE_ACTIVE;
		}
		else
		{
			color = &tab->colors.item.normal;
			imageType = NLTIF_IMAGE;
		}
	}

	RECT rect, itemRect;
	CopyRect(&itemRect, prcItem);

	
		
	if (FRAMETYPE_NONE != color->frameType && 
		FALSE != LoginTab_PaintItemFrame(hdc, tab, color->frameType, prcItem, prcPaint, hdcSrc))
	{
		InflateRect(&itemRect, -(tab->frameWidth -1), -(tab->frameHeight -1));
	}
	
	if (0 != color->backAlpha && FALSE != IntersectRect(&rect, &itemRect, prcPaint))
	{
		LoginTab_FillItem(hdc, &itemRect, &rect, color->backAlpha, color->backTop, color->backBottom, prcPaint->right);
	}

	if (iItem == tab->iFocused && 0 != (NLTDS_FOCUSED & tab->drawStyle))
	{
		CopyRect(&rect, prcItem);
		InflateRect(&rect, -(tab->frameWidth - 2), -(tab->frameWidth - 2));
		COLORREF rgbBkOrig = SetBkColor(hdc, tab->colors.focus);
		COLORREF rgbTextOrig = SetTextColor(hdc, tab->colors.focusDash);
		DrawFocusRect(hdc, &rect);
		//LoginTab_DrawRect(hdc, &rect);
		if (rgbBkOrig != tab->colors.focus) SetBkColor(hdc, rgbBkOrig);
		if (rgbTextOrig != tab->colors.focusDash) SetTextColor(hdc, rgbTextOrig);
	}
	
	if (NULL != tab->imageList)
	{
		UINT iImage = LoginTab_GetImageIndex(hwndTab, tab->imageList, item, iItem, imageType);
		if (NLTM_IMAGE_NONE != iImage)
		{
			IMAGELISTDRAWPARAMS dp;
			dp.cbSize =   56/*sizeof(IMAGELISTDRAWPARAMS) - sizeof(DWORD) * 3*/;
			dp.himl = tab->imageList;
			dp.i = iImage;
			dp.hdcDst = hdc;

			ImageList_GetIconSize(tab->imageList, &dp.cx, &dp.cy);
			dp.x = prcItem->left + ((prcItem->right - prcItem->left) - dp.cx)/2;
			if (dp.x < (prcItem->left + tab->frameWidth)) dp.x = prcItem->left + tab->frameWidth;
			if ((dp.x + dp.cx) > (prcItem->right - tab->frameWidth)) 
			{
				dp.cx = prcItem->right - tab->frameWidth - dp.x;
				if (dp.cx < 0) dp.cx = 0;
			}

			dp.y = prcItem->top + tab->frameHeight + IMAGE_MARGIN_CY;
			if ((dp.y + dp.cy) > (prcItem->bottom - tab->frameHeight)) 
			{
				dp.cy = prcItem->bottom  - tab->frameHeight- dp.y;
				if (dp.cy < 0) dp.cy = 0;
			}
			
			dp.xBitmap = 0;
			dp.yBitmap = 0;
			dp.rgbBk = CLR_NONE;
			dp.rgbFg = CLR_NONE;
			dp.fStyle  = ILD_NORMAL;
			dp.dwRop = SRCCOPY;
			dp.fState = ILS_NORMAL /*| ILS_SATURATE*/ /*| ILS_ALPHA*/;
			dp.Frame = 255;
			dp.crEffect = 0;

			if (dp.cx > 0 && dp.cy > 0)
				ImageList_DrawIndirect(&dp);
		}

	}
	if (NULL != item->text && L'\0' != *item->text)
	{
		LONG left = prcItem->left + ((prcItem->right - prcItem->left) - item->textWidth) / 2;
		if (left < (prcItem->left + tab->frameWidth)) left = prcItem->left + tab->frameWidth;
		
		LONG top = prcItem->bottom - tab->textHeight - tab->frameHeight + 1;
		
		SetRect(&rect, left, top, left + item->textWidth, top + tab->textHeight);
		if (rect.right > (prcItem->right - tab->frameWidth)) rect.right = prcItem->right - tab->frameWidth;
		
		if (rect.bottom > prcPaint->bottom) rect.bottom = prcPaint->bottom;
		if (rect.top < prcPaint->top) rect.top = prcPaint->top;
		if (rect.right > prcPaint->right) rect.right = prcPaint->right;
		if (rect.left < prcPaint->left) rect.left = prcPaint->left;
		
		if (rect.left < rect.right && rect.top < rect.bottom)
		{
			SetTextColor(hdc, color->text);
			INT cchText = lstrlen(item->text);
			ExtTextOut(hdc, left, top, ETO_CLIPPED, &rect, item->text, cchText, NULL);
		}
	}

	return TRUE;
}

static INT CALLBACK LoginTab_PaintItemCallback(LOGINTAB *tab, LOGINTABITEM *item, INT iItem, const RECT *prcItem, ULONG_PTR param)
{
	PAINTITEMPARAM *pip = (PAINTITEMPARAM*)param;
	if (NULL == pip) return -2;

	RECT paintRect;
	if (FALSE != IntersectRect(&paintRect, pip->prcPaint, prcItem))
	{

		SetRectRgn(pip->clipRgn, paintRect.left, paintRect.top, paintRect.right, paintRect.bottom);
		CombineRgn(pip->eraseRgn, pip->eraseRgn, pip->clipRgn, RGN_DIFF);

		HBITMAP hbmp = LoginTab_GetItemBitmap(tab, pip->hdc, 
							prcItem->right - prcItem->left, 
							prcItem->bottom - prcItem->top);

		if (NULL != hbmp)
		{
			SelectObject(pip->hdcItem, hbmp);
			SetViewportOrgEx(pip->hdcItem, -paintRect.left, -paintRect.top, NULL);
		}

		LoginTab_EraseBkGround(pip->hdcItem, tab, pip->prcClient->bottom - pip->prcClient->top, &paintRect);

		if (iItem == tab->itemsCount)
			LoginTab_PaintChevron(pip->hdcItem, tab, prcItem, &paintRect, pip->hdcSrc);
		else
			LoginTab_PaintItem(pip->hwndTab, pip->hdcItem, tab, item, iItem, prcItem, &paintRect, pip->hdcSrc);

		BitBlt(pip->hdc, paintRect.left, paintRect.top, paintRect.right - paintRect.left, paintRect.bottom - paintRect.top, 
				pip->hdcItem, paintRect.left, paintRect.top, SRCCOPY);
	}
	return -1;
}

static UINT LoginTab_GetDrawStyles(HWND hwnd)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	UINT drawStyle = 0;

	if (0 != (WS_DISABLED & windowStyle))
		drawStyle |= NLTDS_DISABLED;
	else if (hwnd == GetFocus())
	{
		UINT uiState = (UINT)SendMessage(hwnd, WM_QUERYUISTATE, 0, 0L);
		if (0 == (UISF_HIDEFOCUS & uiState))
			drawStyle |= NLTDS_FOCUSED;
	}

	if (0 != (NLTS_LOCKED & windowStyle))
		drawStyle |= NLTDS_LOCKED;

	return drawStyle;
}

static void LoginTab_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	
	tab->drawStyle = LoginTab_GetDrawStyles(hwnd);
	
	HBITMAP bitmapSrcOrig, bitmapItemOrig;
	HFONT fontItemOrig;

	PAINTITEMPARAM param;
	param.hwndTab = hwnd;
	param.hdc = hdc;
	param.prcPaint = prcPaint;
	param.prcClient = &clientRect;
	param.clipRgn = CreateRectRgn(0,0,0,0);
	param.eraseRgn = CreateRectRgnIndirect(&clientRect);
	param.hdcSrc = CreateCompatibleDC(hdc);
	param.hdcItem = CreateCompatibleDC(hdc);

	
	if (NULL != param.hdcSrc)
		bitmapSrcOrig = (HBITMAP)GetCurrentObject(param.hdcSrc, OBJ_BITMAP);

	if (NULL != param.hdcItem)
	{
		bitmapItemOrig = (HBITMAP)GetCurrentObject(param.hdcItem, OBJ_BITMAP);
		SetBkMode(param.hdcItem, TRANSPARENT);
		fontItemOrig = (HFONT)SelectObject(param.hdcItem, tab->fontText);
	}

	LoginTab_EnumerateItemRects(hwnd, param.hdcItem, LoginTab_PaintItemCallback, (ULONG_PTR)&param);

	if (FALSE != fErase)
	{
		SelectClipRgn(hdc, param.eraseRgn);	
		LoginTab_EraseBkGround(hdc, tab, clientRect.bottom - clientRect.top, prcPaint);
		//if (SUCCEEDED(UxTheme_LoadLibrary()) && FALSE != UxIsAppThemed())
		//{
		//	// CONTROLPANEL, CPANEL_NAVIGATIONPANE, 0
		//	//
		//	UXTHEME hTheme = UxOpenThemeData(hwnd, L"MENU");
		//	if (NULL != hTheme)
		//	{				
		//		clientRect.right++;
		//		UxDrawThemeBackground(hTheme, hdc, MENU_BARBACKGROUND, MB_ACTIVE, &clientRect, prcPaint);
		//		UxCloseThemeData(hTheme);
		//	}
		//}
	}

	if (NULL != param.hdcSrc)
	{
		SelectObject(param.hdcSrc, bitmapSrcOrig);
		DeleteDC(param.hdcSrc);
	}

	if (NULL != param.hdcItem)
	{
		SelectObject(param.hdcItem, bitmapItemOrig);
		SelectObject(param.hdcItem, fontItemOrig);
		DeleteDC(param.hdcItem);
	}

	DeleteObject(param.clipRgn);
	DeleteObject(param.eraseRgn);
}

static INT CALLBACK LoginTab_GetItemRectCallback(LOGINTAB *tab, LOGINTABITEM *item, INT iItem, const RECT *prcItem, ULONG_PTR param)
{
	GETITEMRECTPARAM *gip = (GETITEMRECTPARAM*)param;
	if (NULL == gip) return -2;

	if (iItem == gip->index)
	{
		CopyRect(gip->rect, prcItem);
		return iItem;
	}

	return -1;
}

static BOOL LoginTab_GetItemRect(HWND hwnd, INT iItem, RECT *itemRect)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab || NULL == itemRect || iItem < 0 || iItem > tab->itemsCount)
		return FALSE;

	GETITEMRECTPARAM param;
	param.index = iItem;
	param.rect = itemRect;
	
	INT result = LoginTab_EnumerateItemRects(hwnd, NULL, LoginTab_GetItemRectCallback, (ULONG_PTR)&param);
	
	return (result == iItem);
}

static INT CALLBACK LoginTab_HitTestCallback(LOGINTAB *tab, LOGINTABITEM *item, INT iItem, const RECT *prcItem, ULONG_PTR param)
{
	HITTESTITEMPARAM *htp = (HITTESTITEMPARAM*)param;
	if (NULL == htp) return -2;

	if (FALSE != PtInRect(prcItem,  htp->pt))
	{
		if (NULL != htp->rect)
			CopyRect(htp->rect, prcItem);
		return iItem;
	}

	return -1;
}

static INT LoginTab_HitTest(HWND hwnd, INT x, INT y, RECT *itemRect)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return -1;

	HITTESTITEMPARAM param;
	param.pt.x = x;
	param.pt.y = y;
	param.rect = itemRect;

	return LoginTab_EnumerateItemRects(hwnd, NULL, LoginTab_HitTestCallback, (ULONG_PTR)&param);
}

static INT CALLBACK LoginTab_UpdateLayoutCallback(LOGINTAB *tab, LOGINTABITEM *item, INT iItem, const RECT *prcItem, ULONG_PTR param)
{
	UPDATELAYOUTPARAM *ulp = (UPDATELAYOUTPARAM*)param;
	if (NULL == ulp) return -2;

	if (iItem != tab->itemsCount)
	{
		if (0 == ulp->itemCount)
			CopyRect(&ulp->visibleBox, prcItem);
		else
			ulp->visibleBox.right = prcItem->right;

		ulp->itemCount++;
	}
	else
	{
		ulp->chevronLeft = prcItem->left;
		
		if (ulp->visibleBox.right > ulp->chevronLeft)
			ulp->visibleBox.right = ulp->chevronLeft;
		
		ulp->chevronVisible = TRUE;
	}

	return -1;
}

static void LoginTab_UpdateLayout(HWND hwnd, BOOL fRedraw)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	RECT clientRect, rect;
	GetClientRect(hwnd, &clientRect);

	UPDATELAYOUTPARAM param;
	ZeroMemory(&param, sizeof(param));
	param.chevronLeft = clientRect.right;

	INT *orderCopy	= NULL;
	if (tab->itemsCount > 0)
	{
		orderCopy = (INT*)calloc(tab->itemsCount, sizeof(INT));
		if (NULL == orderCopy) return;
		CopyMemory(orderCopy, tab->order, tab->itemsCount * sizeof(INT)); 
	}


//	for (INT i = 0; i < tab->itemsCount; i++)
//		tab->order[i] = i; // reset order

	LoginTab_EnumerateItemRects(hwnd, NULL, LoginTab_UpdateLayoutCallback, (ULONG_PTR)&param);

	INT selectPos = -1;
	if (-1  != tab->iSelected)
	{
		for (INT i = 0; i < tab->itemsCount; i++)
		{
			if (tab->order[i] == tab->iSelected)
			{
				selectPos = i;
				break;
			}
		}
	}

	if (tab->lastVisible < selectPos && tab->lastVisible >= 0 && selectPos >= 0)
	{
		CALCITEMWIDTH calcWidth;
		if (FALSE != LoginTab_InitCalcItemWidth(hwnd, NULL, &calcWidth))
		{
			INT selectWidth = LoginTab_CalculateItemWidth(&calcWidth, tab->items[tab->iSelected]);
			INT limit = param.chevronLeft - selectWidth;
			INT right = param.visibleBox.right + tab->spacing;
			INT pos = tab->lastVisible + 1;
			
			while(right > limit && pos-- > 0)
			{
				right -= LoginTab_CalculateItemWidth(&calcWidth, tab->items[tab->order[pos]]);
				if (pos > 0)
					right -= tab->spacing;
			}

			if (pos < selectPos)
				MoveMemory(tab->order + (pos + 1), tab->order + pos, (selectPos - pos) * sizeof(INT));

			tab->order[pos] = tab->iSelected;
			tab->lastVisible = pos;

			right += selectWidth;
			//if (param.visibleBox.right > right)
				param.visibleBox.right = right;

			LoginTab_DestroyCalcItemWidth(&calcWidth);
		}

	}

	INT invalidLeft = clientRect.right;
	if(NULL != orderCopy)
	{
		for (INT i = 0; i < tab->itemsCount; i++)
		{
			if (tab->order[i] != orderCopy[i])
			{
				if (FALSE != LoginTab_GetItemRect(hwnd, tab->order[i], &rect))
					invalidLeft = rect.left;
				break;
			}
		}
		free(orderCopy);
	}

	if (tab->chevronLeft != param.chevronLeft)
	{
		SetRect(&rect,  param.chevronLeft, clientRect.top, clientRect.right, clientRect.bottom);
		if (rect.left > tab->chevronLeft) 
			rect.left = tab->chevronLeft;

		InvalidateRect(hwnd, &rect, FALSE);
		tab->chevronLeft = param.chevronLeft;
	}

	if (tab->visibleRight != param.visibleBox.right || invalidLeft != clientRect.right)
	{
		CopyRect(&rect, &param.visibleBox);
		rect.left = min(param.visibleBox.right, tab->visibleRight);
		if (invalidLeft < rect.left) rect.left = invalidLeft;
		rect.right = max(param.visibleBox.right, tab->visibleRight);

		InvalidateRect(hwnd, &rect, FALSE);
		tab->visibleRight = param.visibleBox.right;
	}

}
static void LoginTab_SetItemFocus(HWND hwnd, INT iFocus)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	INT focusPos = -1;
	if (iFocus >= tab->itemsCount)
		focusPos = iFocus;
	else if (-1 != iFocus)
	{
		for (INT i = 0; i < tab->itemsCount; i++)
		{
			if (tab->order[i] == iFocus)
			{
				focusPos = i;
				break;
			}
		}
	}

	if (focusPos > tab->lastVisible)
	{
		if (tab->lastVisible != (tab->itemsCount -1))
		{
			iFocus = tab->itemsCount;
		}
		else
		{
			iFocus = tab->order[tab->lastVisible];
		}
	}
		
	if (iFocus < 0)
		iFocus = (tab->itemsCount  > 0) ? 0 : -1;

	if (iFocus != tab->iFocused)
	{
		INT iFocused = tab->iFocused;
		INT iSelected = tab->iSelected;

		tab->iFocused = iFocus;
		if (iFocus < tab->itemsCount)
			tab->iSelected = iFocus;

		RECT rect;
		if (-1 != tab->iFocused && FALSE != LoginTab_GetItemRect(hwnd, tab->iFocused, &rect))
			InvalidateRect(hwnd, &rect, FALSE);	
	
		if (-1 != iFocused && iFocused != tab->iFocused &&
			FALSE != LoginTab_GetItemRect(hwnd, iFocused, &rect))
		{
			InvalidateRect(hwnd, &rect, FALSE);	
		}
		
		if (-1 != iSelected && iSelected != tab->iSelected && iSelected != iFocused &&
			FALSE != LoginTab_GetItemRect(hwnd, iSelected, &rect))
		{
			InvalidateRect(hwnd, &rect, FALSE);	
		}

		if (iSelected != tab->iSelected)
		{
			LoginTab_NotifySelectionChanged(hwnd);
		}
	}
}
static BOOL LoginTab_ShowHiddenTabs(HWND hwnd, const RECT *ownerRect)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return FALSE;

	HMENU hMenu = CreatePopupMenu();
	if (NULL == hMenu) return FALSE;

	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE | MIIM_FTYPE;
	mi.dwStyle = /*MNS_MODELESS | */ MNS_NOCHECK;
	SetMenuInfo(hMenu, &mi);

	UINT insertedCount = 0;
	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE | MIIM_FTYPE | MIIM_DATA;
	mii.fState = MFS_UNHILITE | MFS_ENABLED;
	
	CHEVRONMENUPAINTPARAM menuPaint;
	ZeroMemory(&menuPaint, sizeof(menuPaint));

	CALCITEMWIDTH calcItem;
	LoginTab_InitCalcItemWidth(hwnd, NULL, &calcItem);
	
	menuPaint.itemHeight = calcItem.itemHeight;
	if (NULL != ownerRect)
		CopyRect(&menuPaint.ownerRect, ownerRect);
	
	INT width = tab->itemsCount - tab->lastVisible - 1;
	if (width < 2) width = 1;
	else if (width < 9) width = 2;
	else width = (INT)sqrt((float)(width));

	for(INT offset = 0; offset < width; offset++)
	{
		mii.fType = MFT_OWNERDRAW | MFT_MENUBREAK;
		for(INT i = tab->lastVisible + 1 + offset; i < tab->itemsCount; i += width)
		{
			LOGINTABITEM *item = tab->items[tab->order[i]];

			INT itemWidth = LoginTab_CalculateItemWidth(&calcItem, item);
			if (menuPaint.itemWidth < itemWidth) menuPaint.itemWidth = itemWidth;

			mii.wID = tab->order[i] + 1;
			mii.dwTypeData = item->text;
			mii.dwItemData = (ULONG_PTR)item;
			
			if (FALSE != InsertMenuItem(hMenu, insertedCount, TRUE, &mii)) 
				insertedCount++;

			mii.fType = MFT_OWNERDRAW;
		}
	}

	LoginTab_DestroyCalcItemWidth(&calcItem);

	if (NULL != hMenu && insertedCount > 0)
	{
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);

		POINT menuOrig;
		menuOrig.x = windowRect.right;
		menuOrig.y = windowRect.bottom - 1;

		UINT menuStyle = TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_VERPOSANIMATION | TPM_NONOTIFY | TPM_RETURNCMD;


		TRACKMOUSEEVENT tm;
		tm.cbSize = sizeof(tm);
		tm.dwFlags = TME_CANCEL | TME_LEAVE;
		tm.hwndTrack = hwnd;
		TrackMouseEvent(&tm);

		tab->chevronMenu = hMenu;

		HBITMAP bitmapSrcOrig, bitmapItemOrig;
		HFONT fontItemOrig;

		HBRUSH hbrBack = NULL;

		HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			menuPaint.hdcSrc = CreateCompatibleDC(hdc);
			menuPaint.hdcItem = CreateCompatibleDC(hdc);
			

			if (NULL != menuPaint.hdcSrc)
			{
				bitmapSrcOrig = (HBITMAP)GetCurrentObject(menuPaint.hdcSrc, OBJ_BITMAP);

				INT menuHeight = (insertedCount /width + insertedCount%width) * menuPaint.itemHeight + 2 * 4;
				HBITMAP hbmpPattern = CreateCompatibleBitmap(hdc, 1, menuHeight);
				if (NULL != hbmpPattern)
				{
					HBITMAP bmpOrig = (HBITMAP)SelectObject(menuPaint.hdcSrc, hbmpPattern);
					RECT rect;
					SetRect(&rect, 0, 0, 1, menuHeight);
					LoginTab_EraseBkGround(menuPaint.hdcSrc, tab, menuHeight, &rect);
					SelectObject(menuPaint.hdcSrc, bmpOrig);
					hbrBack = CreatePatternBrush(hbmpPattern);
					DeleteObject(hbmpPattern);
				}
			}

			if (NULL != menuPaint.hdcItem)
			{
				bitmapItemOrig = (HBITMAP)GetCurrentObject(menuPaint.hdcItem, OBJ_BITMAP);
				SetBkMode(menuPaint.hdcItem, TRANSPARENT);
				fontItemOrig = (HFONT)SelectObject(menuPaint.hdcItem, tab->fontText);
			}

			

			ReleaseDC(hwnd, hdc);
		}

		
		MENUINFO mi;
		mi.cbSize = sizeof(mi);
		mi.fMask = MIM_MENUDATA | MIM_BACKGROUND;
		mi.dwMenuData = (ULONG_PTR)&menuPaint;
		mi.hbrBack = hbrBack;
		SetMenuInfo(hMenu, &mi);
		
		TPMPARAMS tpm;
		tpm.cbSize =sizeof(tpm);
		GetWindowRect(hwnd, &tpm.rcExclude);
		tpm.rcExclude.bottom -= 2;
		tpm.rcExclude.left = -20000;
		tpm.rcExclude.right = 20000;
		INT commandId = TrackPopupMenuEx(hMenu, menuStyle, menuOrig.x, menuOrig.y, hwnd, &tpm);
		commandId--;

		if ( hbrBack != NULL )
			DeleteObject( hbrBack );

		if (NULL != menuPaint.hdcSrc)
		{
			SelectObject(menuPaint.hdcSrc, bitmapSrcOrig);
			DeleteDC(menuPaint.hdcSrc);
		}

		if (NULL != menuPaint.hdcItem)
		{
			SelectObject(menuPaint.hdcItem, bitmapItemOrig);
			SelectObject(menuPaint.hdcItem, fontItemOrig);
			DeleteDC(menuPaint.hdcItem);
		}

		tab->chevronMenu = NULL;
		
		if (commandId >= 0 && commandId < tab->itemsCount && tab->iSelected != commandId)
		{
			LoginTab_SetCurSel(hwnd, commandId);
			LoginTab_NotifySelectionChanged(hwnd);
		}
	}	

	if (NULL != hMenu)
		DestroyMenu(hMenu);

	return TRUE;
}

static void LoginTab_TrackMouseLeave(HWND hwnd)
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


static void LoginTab_UpdateColors(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	tab->colors.backTop	= GetSysColor(COLOR_WINDOW);

	WORD h, l, s, lBottom, lLine;
	INT k;
	ColorRGBToHLS(tab->colors.backTop, &h, &l, &s);
	k = MulDiv(240, 50, 1000);
	lBottom = l + ((l > 0) ? -k : k);
	if (lBottom > 240) lBottom = 240;

	k = MulDiv(240, 75, 1000);
	lLine = l + ((l > 0) ? -k : k);
	if (lLine > 240) lLine = 240;
	
	tab->colors.backBottom = ColorHLSToRGB(h, lBottom, s);
	tab->colors.backLine = ColorHLSToRGB(h, lLine, s);

	tab->colors.focus		= RGB(255, 255, 255);
	tab->colors.focusDash	= RGB(0, 0, 0);

	COLORREF rgbTextActive = GetSysColor(COLOR_WINDOWTEXT);
	COLORREF rgbText = Color_Blend(rgbTextActive, tab->colors.backBottom, 210);


	tab->colors.item.normal.backAlpha	= 0;
	tab->colors.item.normal.backTop		= 0;
	tab->colors.item.normal.backBottom	= 0;
	tab->colors.item.normal.frameType	= FRAMETYPE_NONE;
	tab->colors.item.normal.text		= rgbText;

	tab->colors.item.normalHigh.backAlpha		= tab->colors.item.normal.backAlpha;
	tab->colors.item.normalHigh.backTop			= tab->colors.item.normal.backTop;
	tab->colors.item.normalHigh.backBottom		= tab->colors.item.normal.backBottom;
	tab->colors.item.normalHigh.frameType		= tab->colors.item.normal.frameType;
	tab->colors.item.normalHigh.text			= rgbTextActive;

	tab->colors.item.normalPressed.backAlpha	= tab->colors.item.normal.backAlpha;
	tab->colors.item.normalPressed.backTop		= tab->colors.item.normal.backTop;
	tab->colors.item.normalPressed.backBottom	= tab->colors.item.normal.backBottom;
	tab->colors.item.normalPressed.frameType	= tab->colors.item.normal.frameType;
	tab->colors.item.normalPressed.text			= rgbTextActive;

	tab->colors.item.normalDisabled.backAlpha		= tab->colors.item.normal.backAlpha;
	tab->colors.item.normalDisabled.backTop			= tab->colors.item.normal.backTop;
	tab->colors.item.normalDisabled.backBottom		= tab->colors.item.normal.backBottom;
	tab->colors.item.normalDisabled.frameType		= tab->colors.item.normal.frameType;
	tab->colors.item.normalDisabled.text			= Color_Blend(GetSysColor(COLOR_GRAYTEXT), tab->colors.backBottom, 160);
	
	tab->colors.item.selected.backAlpha			= 100;
	tab->colors.item.selected.backTop			= Color_Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_WINDOW), 100);
	tab->colors.item.selected.backBottom		= Color_Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_WINDOW), 140);

	tab->colors.item.selected.frameType			= FRAMETYPE_SELECTED;
	tab->colors.item.selected.text				= rgbTextActive;

	tab->colors.item.selectedHigh.backAlpha		= tab->colors.item.selected.backAlpha;
	tab->colors.item.selectedHigh.backTop		= tab->colors.item.selected.backTop;
	tab->colors.item.selectedHigh.backBottom	= tab->colors.item.selected.backBottom;
	tab->colors.item.selectedHigh.frameType		= tab->colors.item.selected.frameType;
	tab->colors.item.selectedHigh.text			= tab->colors.item.selected.text;

	tab->colors.item.selectedPressed.backAlpha	= tab->colors.item.selected.backAlpha;
	tab->colors.item.selectedPressed.backTop	= tab->colors.item.selected.backTop;
	tab->colors.item.selectedPressed.backBottom	= tab->colors.item.selected.backBottom;
	tab->colors.item.selectedPressed.frameType	= tab->colors.item.selected.frameType;
	tab->colors.item.selectedPressed.text		= tab->colors.item.selected.text;

	tab->colors.item.selectedDisabled.backAlpha		= tab->colors.item.selected.backAlpha;
	tab->colors.item.selectedDisabled.backTop		= Color_Blend(GetSysColor(COLOR_3DLIGHT), GetSysColor(COLOR_WINDOW), 100);
	tab->colors.item.selectedDisabled.backBottom	= Color_Blend(GetSysColor(COLOR_3DLIGHT), GetSysColor(COLOR_WINDOW), 140);
	tab->colors.item.selectedDisabled.frameType		= FRAMETYPE_DISABLED;
	tab->colors.item.selectedDisabled.text			= tab->colors.item.normalDisabled.text;

	if (NULL != tab->frameBitmap)
		DeleteObject(tab->frameBitmap);
	
	BITMAPINFOHEADER frameHeader;
	BYTE *framePixels;
	tab->frameBitmap = ImageLoader_LoadBitmapEx(WASABI_API_ORIG_HINST, 
			MAKEINTRESOURCE(IDR_SELECTIONFRAME_IMAGE), FALSE, &frameHeader, (void**)&framePixels);
	if (NULL == tab->frameBitmap)
	{
		tab->frameWidth  = 0;
		tab->frameHeight = 0;
	}
	else
	{
		if (frameHeader.biHeight < 0) 
			frameHeader.biHeight = -frameHeader.biHeight;

		tab->frameWidth = (frameHeader.biWidth - 1)/2;
		tab->frameHeight = (frameHeader.biHeight/2 - 1)/2;

		COLORREF rgbTop =  Color_Blend(GetSysColor(COLOR_HIGHLIGHT), GetSysColor(COLOR_WINDOW), 200);
		COLORREF rgbBottom = GetSysColor(COLOR_WINDOW);

		Image_ColorizeEx(framePixels, frameHeader.biWidth, frameHeader.biHeight, 0, 0, 
				frameHeader.biWidth, frameHeader.biHeight, frameHeader.biBitCount, TRUE, rgbBottom, rgbTop);
	}

	if (NULL != tab->chevronImage)
	{
		DeleteObject(tab->chevronImage);
		tab->chevronImage = NULL;
	}

	tab->chevronImage = LoginTab_LoadChevronImage(hwnd, &tab->chevronWidth, NULL);
	if (NULL == tab->chevronImage)
	{
		tab->chevronWidth  = 0;
	}
	else
	{	
		INT k = tab->frameWidth - 2;
		if (k < 0) k = 0;
		tab->chevronWidth += 2 * k;
	}

}

static void LoginTab_UpdateFonts(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;


	tab->fontText = NULL;
	LoginGuiObject *loginGui;
	if (SUCCEEDED(LoginGuiObject::QueryInstance(&loginGui)))
	{
		tab->fontText = loginGui->GetTextFont();
		loginGui->Release();
	}
		
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
	{
		HFONT fontOrig = (HFONT)SelectObject(hdc, tab->fontText);
		
		TEXTMETRIC tm;
		if (FALSE != GetTextMetrics(hdc, &tm))
		{
			tab->textHeight = tm.tmHeight;
	
			LONG baseunitY = tm.tmHeight;
			LONG baseunitX = LoginBox_GetAveCharWidth(hdc);

			tab->textWidthMax = baseunitX * MAX_TEXT_AVECHAR_WIDTH + tm.tmOverhang;
			tab->margins.left = MulDiv(1, baseunitX, 4);
			tab->margins.right = MulDiv(1, baseunitX, 4);
			tab->margins.top = MulDiv(1, baseunitY, 8);
			tab->margins.bottom = MulDiv(1, baseunitY, 8);
			tab->spacing = MulDiv(1, baseunitX, 4);

			for (INT i = 0; i < tab->itemsCount; i++)
				tab->items[i]->textWidth = -1;
		}
	

		SelectObject(hdc, fontOrig);
		ReleaseDC(hwnd, hdc);
	}
}
static void LoginTab_UpdateMouseInfo(HWND hwnd)
{
	POINT pt;
	RECT rect;
	GetCursorPos(&pt);
	MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);

	GetClientRect(hwnd, &rect);
	if (FALSE == PtInRect(&rect, pt))
	{
		SendMessage(hwnd, WM_MOUSELEAVE, 0, 0L);
	}
	else
	{
		UINT vKey = 0;
		if (0 != (0x8000 & GetAsyncKeyState(VK_CONTROL)))
			vKey |= MK_CONTROL;
		if (0 != (0x8000 & GetAsyncKeyState(VK_LBUTTON)))
			vKey |= MK_LBUTTON;
		if (0 != (0x8000 & GetAsyncKeyState(VK_RBUTTON)))
			vKey |= MK_RBUTTON;
		if (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)))
			vKey |= MK_SHIFT;
		if (0 != (0x8000 & GetAsyncKeyState(VK_XBUTTON1)))
			vKey |= MK_XBUTTON1;
		if (0 != (0x8000 & GetAsyncKeyState(VK_XBUTTON2)))
			vKey |= MK_XBUTTON2;

		SendMessage(hwnd, WM_MOUSEMOVE, vKey, MAKELPARAM(pt.x, pt.y));
	}
}
static HWND LoginTab_CreateTooltip(HWND hwnd)
{
	HWND hTooltip = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOPARENTNOTIFY | WS_EX_TRANSPARENT | WS_EX_LAYERED, 
							TOOLTIPS_CLASS, NULL, WS_CLIPSIBLINGS | WS_POPUP | TTS_NOPREFIX /*| TTS_ALWAYSTIP*/,
							0, 0, 0, 0, hwnd, NULL, NULL, NULL);

	if (NULL == hTooltip)
		return NULL;

	SendMessage(hTooltip, CCM_SETVERSION, 6, 0L);
	SetWindowPos(hTooltip, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	SendMessage(hTooltip, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELPARAM(1000, 0)); 

	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hwnd;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage(hTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);


	INT helpWidthMax = 260;

	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);

	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(vi);
	if (FALSE == GetVersionEx(&vi))
		ZeroMemory(&vi, sizeof(vi));
	
	if (vi.dwMajorVersion  < 6)
		ncm.cbSize -= sizeof(ncm.iPaddedBorderWidth);

	RECT marginRect;
	SetRect(&marginRect, 3, 1, 3, 1);
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0))
	{
		HFONT font = CreateFontIndirect(&ncm.lfStatusFont);
		if (NULL != font)
		{
			HDC hdc = GetDCEx(hTooltip, NULL, DCX_CACHE);
			if (NULL != hdc)
			{
				HFONT fontOrig = (HFONT)SelectObject(hdc, font);
				TEXTMETRIC tm;
				if (FALSE != GetTextMetrics(hdc, &tm))
				{
					INT baseunitX = LoginBox_GetAveCharWidth(hdc);
					if (NULL != baseunitX)
					{
						helpWidthMax = baseunitX * MAX_HELP_AVECHAR_WIDTH + tm.tmOverhang;
						marginRect.left = MulDiv(2, baseunitX, 4);
						marginRect.right = marginRect.left;
						marginRect.top = MulDiv(1, tm.tmHeight, 8);
						marginRect.bottom = MulDiv(1, tm.tmHeight, 8);
					}
				}
				SelectObject(hdc, fontOrig);
				ReleaseDC(hTooltip, hdc);
			}
			DeleteObject(font);
		}
	}
			
	SendMessage(hTooltip, TTM_SETMAXTIPWIDTH, 0, helpWidthMax);
	SendMessage(hTooltip, TTM_SETMARGIN, 0, (LPARAM)&marginRect);
	
	return hTooltip;
}

static void LoginTab_RelayTooltipMouseMsg(HWND hwnd, UINT uMsg, UINT vKey, POINTS pts)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab || NULL == tab->hTooltip || -1 == tab->iHighlighted)
		return;

	MSG msg;
	msg.hwnd = hwnd;
	msg.message = uMsg;
	msg.wParam = (WPARAM)vKey;
	msg.lParam = MAKELPARAM(pts.x, pts.y);

	SendMessage(tab->hTooltip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
}

static void LoginTab_UpdateTooltip(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.hwnd = hwnd;
	ti.uId = 0;
		
	if (FALSE == SendMessage(tab->hTooltip, TTM_GETTOOLINFO, 0, (LPARAM)&ti))
		return;
			
	INT iItem = tab->iHighlighted;
	RECT rect;
	if (-1 == iItem || FALSE == LoginTab_GetItemRect(hwnd, iItem, &rect))
	{		
		SendMessage(tab->hTooltip, TTM_ACTIVATE, FALSE, 0L);
		SysFreeString(tab->helpText);
		tab->helpText = NULL;

		return;
	}

	if (ti.lParam != (LPARAM)iItem || FALSE == EqualRect(&ti.rect, &rect))
	{
		SendMessage(tab->hTooltip, TTM_ACTIVATE, FALSE, 0L);
	
		CopyRect(&ti.rect, &rect);
		if (ti.lParam != iItem)
		{
			LPCWSTR pszTitle = NULL;
			if (iItem >= 0 && iItem < tab->itemsCount)
				pszTitle = tab->items[iItem]->text;
			SendMessage(tab->hTooltip, TTM_SETTITLE, (WPARAM)TTI_NONE, (LPARAM)pszTitle);

			ti.lParam = (LPARAM)iItem;
			ti.lpszText = LPSTR_TEXTCALLBACK;
	
			SysFreeString(tab->helpText);
			tab->helpText = NULL;
		}
		SendMessage(tab->hTooltip, TTM_SETTOOLINFO, 0, (LPARAM)&ti);
		
	}

	SendMessage(tab->hTooltip, TTM_ACTIVATE, TRUE, 0L);
}

static void LoginTab_FreeItem(LOGINTABITEM *item)
{
	if (NULL == item) return;
	
	LoginBox_FreeString(item->text);
	free(item);
}

static BOOL LoginTab_SetItemInternal(LOGINTABITEM *dst, const NLTITEM *src)
{
	if (NULL == dst || NULL == src)
		return FALSE;

	BOOL succeeded = TRUE;
	
	if (0 != (NLTIF_TEXT & src->mask))
	{
		dst->textWidth = -1;
		LoginBox_FreeString(dst->text);
		if (NULL != src->pszText)
		{
			dst->text = LoginBox_CopyString(src->pszText);
			if (NULL == dst->text) succeeded = FALSE;
		}
		else
		{
			dst->text = NULL;
		}
	}

	if (0 != (NLTIF_PARAM & src->mask))
		dst->param = src->param;

	if (0 != (NLTIF_IMAGE & src->mask))
		dst->iImage = src->iImage;

	if (0 != (NLTIF_IMAGE_ACTIVE & src->mask))
		dst->iImageActive = src->iImageActive;

	if (0 != (NLTIF_IMAGE_DISABLED & src->mask))
		dst->iImageDisabled = src->iImageDisabled;

	return succeeded;
}

static LOGINTABITEM *LoginTab_CreateItem(const NLTITEM *pItem)
{
	LOGINTABITEM *item = (LOGINTABITEM*)calloc(1, sizeof(LOGINTABITEM));
	if (NULL == item) return NULL;

	item->textWidth = -1;
	item->iImage = NLTM_IMAGE_NONE;
	item->iImageActive = NLTM_IMAGE_NONE;
	item->iImageDisabled = NLTM_IMAGE_NONE;

	if (NULL != pItem && FALSE == LoginTab_SetItemInternal(item, pItem))
	{
		LoginTab_FreeItem(item);
		item = NULL;
	}
	return item;
}

static INT LoginTab_DeleteAllItemsReal(HWND hwnd, LOGINTABITEM **itemsList, INT itemsCount)
{
	if (NULL == itemsList || itemsCount < 1)
		return 0;

	NMLOGINTAB nmp;
	nmp.hdr.hwndFrom = hwnd;
	nmp.hdr.idFrom = GetWindowLongPtr(hwnd, GWLP_ID);
	HWND hParent = GetAncestor(hwnd, GA_PARENT);

	BOOL fNotifyItem = FALSE;

	if (NULL != hParent)
	{
		nmp.hdr.code = NLTN_DELETEALLITEMS;
		nmp.iItem = -1;
		fNotifyItem = (FALSE == (BOOL)SNDMSG(hParent, WM_NOTIFY, (WPARAM)nmp.hdr.idFrom, (LPARAM)&nmp));
		if (FALSE != fNotifyItem)
			nmp.hdr.code = NLTN_DELETEITEM;
	}

	INT deleted = 0;
	for(; itemsCount > 0; itemsCount--)
	{
		nmp.iItem = itemsCount - 1;
		if (FALSE != fNotifyItem)
			SNDMSG(hParent, WM_NOTIFY, (WPARAM)nmp.hdr.idFrom, (LPARAM)&nmp);
		
		LoginTab_FreeItem(itemsList[nmp.iItem]);
		deleted++;
	}
	
	return deleted;
}

static LRESULT LoginTab_OnCreate(HWND hwnd, CREATESTRUCT* pcs)
{
	LOGINTAB *tab = (LOGINTAB*)calloc(1, sizeof(LOGINTAB));
	if (NULL != tab)
	{
		SetLastError(ERROR_SUCCESS);
		if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)tab) && ERROR_SUCCESS != GetLastError())
		{
			free(tab);
			tab = NULL;
		}
	}

	if (NULL == tab)
		return -1;
	
	tab->iPressed = -1;
	tab->iSelected = -1;
	tab->iHighlighted = -1;

	tab->hTooltip = LoginTab_CreateTooltip(hwnd);

	LoginTab_UpdateColors(hwnd);
	LoginTab_UpdateFonts(hwnd);
		
	return 0;
}

static void LoginTab_OnDestroy(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);
	if (NULL == tab) return;
	
	if (NULL != tab->items)
	{
		tab->itemsCount -= LoginTab_DeleteAllItemsReal(hwnd, tab->items, tab->itemsCount);
		free(tab->items);
	}

	if (NULL != tab->order)
		free(tab->order);

	if (NULL != tab->frameBitmap)
		DeleteObject(tab->frameBitmap);

	if (NULL != tab->itemBitmap)
		DeleteObject(tab->itemBitmap);

	if (NULL != tab->chevronImage)
		DeleteObject(tab->chevronImage);

	if (NULL != tab->hTooltip)
		DestroyWindow(tab->hTooltip);

	SysFreeString(tab->helpText);

	free(tab);
}

static void LoginTab_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags))
		return;

	LoginTab_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & pwp->flags));
}


static void LoginTab_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			LoginTab_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void LoginTab_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{	
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
		LoginTab_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}


static void LoginTab_OnMouseMove(HWND hwnd, UINT vKey, POINTS pts)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	if (FALSE != LoginTab_IsLocked(hwnd))
		return;

	RECT rect;

	INT iHighlighted = tab->iHighlighted;
	
	tab->iHighlighted = (-1 == tab->iPressed) ? 
		LoginTab_HitTest(hwnd, pts.x, pts.y, &rect) : -1;
	

	
	if (iHighlighted != tab->iHighlighted)
		LoginTab_UpdateTooltip(hwnd);
	
	LoginTab_RelayTooltipMouseMsg(hwnd, WM_MOUSEMOVE, vKey, pts);
		

	if (iHighlighted != tab->iHighlighted)
	{		
		InvalidateRect(hwnd, &rect, FALSE);
	
		if (-1 != iHighlighted && FALSE != LoginTab_GetItemRect(hwnd, iHighlighted, &rect))
			InvalidateRect(hwnd, &rect, FALSE);	
	}
	
	if (-1 != tab->iHighlighted)
		LoginTab_TrackMouseLeave(hwnd);

	
}

static void LoginTab_OnMouseLeave(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	INT iPressed = tab->iPressed;
	INT iHighlighted = tab->iHighlighted;

	tab->iPressed = -1;
	tab->iHighlighted = -1;

	RECT rect;
	if (-1 != iHighlighted && FALSE != LoginTab_GetItemRect(hwnd, iHighlighted, &rect))
		InvalidateRect(hwnd, &rect, FALSE);	

	if (-1 != iPressed && iPressed != iHighlighted && FALSE != LoginTab_GetItemRect(hwnd, iPressed, &rect))
		InvalidateRect(hwnd, &rect, FALSE);	

}

static void LoginTab_OnLButtonDown(HWND hwnd, UINT vKey, POINTS pts)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	if (FALSE != LoginTab_IsLocked(hwnd))
		return;

	LoginTab_RelayTooltipMouseMsg(hwnd, WM_LBUTTONDOWN, vKey, pts);
	
	RECT rect, rect2;
	INT iPressed, iHighlighted;
	INT iItem = LoginTab_HitTest(hwnd, pts.x, pts.y, &rect);
	
	iPressed = tab->iPressed;
	iHighlighted = tab->iHighlighted;
	
	tab->iPressed = iItem;
	tab->iHighlighted = -1;

	if (iPressed != iItem && -1 != iPressed && 
		FALSE != LoginTab_GetItemRect(hwnd, iPressed, &rect2))
	{
		InvalidateRect(hwnd, &rect2, FALSE);	
	}

	if (iHighlighted != iItem && -1 != iHighlighted && iHighlighted != iPressed && 
		FALSE != LoginTab_GetItemRect(hwnd, iHighlighted, &rect2))
	{
		InvalidateRect(hwnd, &rect2, FALSE);	
	}

	if (-1 != iItem && iPressed != iItem)
	{
		if (iItem == tab->itemsCount && tab->iFocused != iItem)
		{
			INT iFocused = tab->iFocused;
			tab->iFocused = iItem;

			if (-1 != iFocused && FALSE != LoginTab_GetItemRect(hwnd, iFocused, &rect2))
				InvalidateRect(hwnd, &rect2, FALSE);
		}
		InvalidateRect(hwnd, &rect, FALSE);
	
		if (iItem == tab->itemsCount)
		{			
		
			LoginTab_ShowHiddenTabs(hwnd, &rect);
			
			if (0 == (0x8000 & GetAsyncKeyState((FALSE == GetSystemMetrics(SM_SWAPBUTTON)) ? VK_LBUTTON : VK_RBUTTON)))
				tab->iPressed = -1;
			tab->iFocused = tab->itemsCount;
			InvalidateRect(hwnd, &rect, FALSE);
			LoginTab_UpdateMouseInfo(hwnd);

		}
		else if (hwnd != GetCapture())
			SetCapture(hwnd);
	}
}

static void LoginTab_OnLButtonUp(HWND hwnd, UINT vKey, POINTS pts)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	if (FALSE != LoginTab_IsLocked(hwnd))
		return;

	LoginTab_RelayTooltipMouseMsg(hwnd, WM_LBUTTONUP, vKey, pts);

	RECT rect, rect2;

	INT iPressed = tab->iPressed;
	INT iHighlighted = tab->iHighlighted;
	INT iSelected = tab->iSelected;
	INT iFocused = tab->iFocused;
	
	INT iItem = LoginTab_HitTest(hwnd, pts.x, pts.y, &rect);
	
		
	tab->iPressed = -1;
	tab->iHighlighted = iItem;

	if (iItem == iPressed && -1 != iItem)
	{
		if (iItem < tab->itemsCount)
			tab->iSelected = iItem;
		tab->iFocused = iItem;
	}

	if (-1 != iHighlighted && iHighlighted != tab->iHighlighted &&
		iHighlighted != iPressed && iHighlighted != iFocused && iHighlighted != iSelected && 
		FALSE != LoginTab_GetItemRect(hwnd, iHighlighted, &rect2))
	{
		InvalidateRect(hwnd, &rect2, FALSE);	
	}

	
	if (-1 != iPressed && iPressed != tab->iPressed &&
		(iPressed != iFocused || tab->iFocused == iFocused) && 
		(iPressed != iSelected  || tab->iSelected == iSelected) && 
		FALSE != LoginTab_GetItemRect(hwnd, iPressed, &rect2))
	{
		InvalidateRect(hwnd, &rect2, FALSE);	
	}

	
	if (-1 != iSelected && iSelected != tab->iSelected &&
		iSelected  != iFocused && 
		FALSE != LoginTab_GetItemRect(hwnd, iSelected, &rect2))
	{
		InvalidateRect(hwnd, &rect2, FALSE);	
	}

	if (-1 != iFocused && iFocused != tab->iFocused &&
		FALSE != LoginTab_GetItemRect(hwnd, iFocused, &rect2))
	{
		InvalidateRect(hwnd, &rect2, FALSE);	
	}

	

	if (-1 != iItem)
		InvalidateRect(hwnd, &rect, FALSE);
	
	if (hwnd == GetCapture())
		ReleaseCapture();

	if (-1 != tab->iSelected && tab->iSelected != iSelected)
		LoginTab_NotifySelectionChanged(hwnd);
}

static void LoginTab_OnRButtonDown(HWND hwnd, UINT vKey, POINTS pts)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	if (FALSE != LoginTab_IsLocked(hwnd))
		return;

	LoginTab_RelayTooltipMouseMsg(hwnd, WM_RBUTTONDOWN, vKey, pts);

	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL != hParent)
	{
		NMLOGINTABCLICK ntc;
		ntc.hdr.code = NM_RCLICK;
		ntc.hdr.hwndFrom = hwnd;
		ntc.hdr.idFrom = GetWindowLongPtr(hwnd, GWLP_ID);
		ntc.pt.x = pts.x;
		ntc.pt.y = pts.y;
		SNDMSG(hParent, WM_NOTIFY, (WPARAM)ntc.hdr.idFrom, (LPARAM)&ntc);
	}
}

static void LoginTab_OnRButtonUp(HWND hwnd, UINT vKey, POINTS pts)
{
	if (FALSE != LoginTab_IsLocked(hwnd))
		return;

	LoginTab_RelayTooltipMouseMsg(hwnd, WM_RBUTTONUP, vKey, pts);
}

static void LoginTab_OnMButtonDown(HWND hwnd, UINT vKey, POINTS pts)
{
	if (FALSE != LoginTab_IsLocked(hwnd))
		return;

	LoginTab_RelayTooltipMouseMsg(hwnd, WM_MBUTTONDOWN, vKey, pts);
}

static void LoginTab_OnMButtonUp(HWND hwnd, UINT vKey, POINTS pts)
{
	if (FALSE != LoginTab_IsLocked(hwnd))
		return;

	LoginTab_RelayTooltipMouseMsg(hwnd, WM_MBUTTONUP, vKey, pts);
}

static void LoginTab_OnCaptureChanged(HWND hwnd, HWND hCapture)
{
	if (hwnd != hCapture)
		LoginTab_TrackMouseLeave(hwnd);
}

static void LoginTab_OnSetFocus(HWND hwnd, HWND hFocus)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	INT iFocus = tab->iSelected;
	tab->iFocused = -1;
	LoginTab_SetItemFocus(hwnd, iFocus);
}

static void LoginTab_OnKillFocus(HWND hwnd, HWND hFocus)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	RECT rect;
	if (-1 != tab->iFocused && FALSE != LoginTab_GetItemRect(hwnd, tab->iFocused, &rect))
	{
		InvalidateRect(hwnd, &rect, FALSE);	
	}
}

static void LoginTab_OnEnable(HWND hwnd, BOOL fEnabled)
{
	InvalidateRect(hwnd, NULL, FALSE);
}

static void LoginTab_OnKeyDown(HWND hwnd, INT vKey, UINT flags)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	if (FALSE != LoginTab_IsLocked(hwnd))
		return;
	
	INT focusPos = -1;
	if (tab->iFocused >= tab->itemsCount)
	{
		focusPos = tab->iFocused;
	}
	else if (-1 != tab->iFocused)
	{
		for (INT i = 0; i < tab->itemsCount; i++)
		{
			if (tab->order[i] == tab->iFocused)
			{
				focusPos = i;
				break;
			}
		}
	}

	switch(vKey)
	{
		case VK_LEFT:	focusPos = (focusPos > tab->lastVisible) ? tab->lastVisible : (focusPos - 1); break;
		case VK_RIGHT:	focusPos++; break;
		case VK_PRIOR:
		case VK_HOME:	focusPos = 0; break;
		case VK_END:	focusPos = tab->itemsCount -1; break;
		case VK_NEXT:	focusPos = tab->lastVisible; break;	

		case VK_SPACE:
		case VK_RETURN:
			if (tab->iFocused == tab->itemsCount)
			{
				tab->iPressed = tab->iFocused;
				RECT rect;
				if (FALSE == LoginTab_GetItemRect(hwnd, tab->iPressed, &rect))
					SetRectEmpty(&rect);
				
				LoginTab_ShowHiddenTabs(hwnd, &rect);
				tab->iFocused = tab->itemsCount;
				tab->iPressed = -1;
				InvalidateRect(hwnd, &rect, FALSE);
				LoginTab_UpdateMouseInfo(hwnd);
				return;
			}
			break;
	}

	INT iFocus;
	if (focusPos >= tab->itemsCount)
		iFocus = tab->itemsCount;
	else if (focusPos < 0)
		iFocus = 0;
	else
		iFocus = tab->order[focusPos];

	LoginTab_SetItemFocus(hwnd, iFocus);
}


static LRESULT LoginTab_OnGetDlgCode(HWND hwnd, INT vKey, MSG *pMsg)
{
	switch(vKey)
	{
		case VK_TAB:	return 0;
	}
	return DLGC_WANTALLKEYS;
}

static LRESULT LoginTab_OnMenuChar(HWND hwnd, INT vkCode, INT menuType, HMENU hMenu)
{
	switch(vkCode)
	{
		case VK_SPACE:
		case VK_RETURN:
			for (INT i = GetMenuItemCount(hMenu) - 1; i >= 0; i--)
			{
				UINT r = GetMenuState(hMenu, i, MF_BYPOSITION);
				if (-1 != r && 0 != (MF_HILITE & LOWORD(r))) 
					return MAKELRESULT(i, MNC_EXECUTE);	
			}
			return MAKELRESULT(0, MNC_SELECT);
	}
	return MAKELRESULT(0, MNC_IGNORE);	
}

static void LoginTab_OnMenuSelect(HWND hwnd, INT iItem, UINT flags, HMENU hMenu)
{
	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_MENUDATA | MIM_BACKGROUND;

	if (FALSE != GetMenuInfo(hMenu, &mi) && NULL != mi.dwMenuData)
	{
		CHEVRONMENUPAINTPARAM *pmp= (CHEVRONMENUPAINTPARAM*)mi.dwMenuData;
		if (NULL != pmp->hwndMenu)
		{
			UINT stateOrig = (UINT)SendMessage(pmp->hwndMenu, WM_QUERYUISTATE, 0, 0L);
				
			SendMessage(pmp->hwndMenu, WM_CHANGEUISTATE, 
				MAKEWPARAM(UISF_HIDEACCEL | UISF_HIDEFOCUS, UIS_INITIALIZE), 0L);

			UINT stateCurrent = (UINT)SendMessage(pmp->hwndMenu, WM_QUERYUISTATE, 0, 0L);
			if ((UISF_HIDEFOCUS & stateOrig) != (UISF_HIDEFOCUS & stateCurrent))
			{
				INT menuCount = GetMenuItemCount(hMenu);
				while(menuCount--)
				{
					if (iItem == GetMenuItemID(hMenu, menuCount))
					{
						RECT rect;
						if (FALSE != GetMenuItemRect(NULL, hMenu, menuCount, &rect))
						{					
							MapWindowPoints(HWND_DESKTOP, pmp->hwndMenu, (POINT*)&rect, 2);
							InvalidateRect(pmp->hwndMenu, &rect, FALSE);
						}
						break;
					}
				}
			}
		}
	}
}


static void LoginTab_GetTootipDispInfo(HWND hwnd, NMTTDISPINFO *pdisp)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return;

	if(NULL == tab->helpText)
	{
		INT iItem = (INT)pdisp->lParam;	
		if (iItem >= 0 && iItem < tab->itemsCount)
		{
			HWND hParent = GetAncestor(hwnd, GA_PARENT);
			if (NULL != hParent)
			{
				NMLOGINTABHELP help;
				help.hdr.code = NLTN_GETITEMHELP;
				help.hdr.hwndFrom = hwnd;
				help.hdr.idFrom = GetWindowLongPtr(hwnd, GWLP_ID);
				help.iItem = iItem;
				help.param = tab->items[iItem]->param;
				help.bstrHelp = NULL;
				SNDMSG(hParent, WM_NOTIFY, (WPARAM)help.hdr.idFrom, (LPARAM)&help);
				tab->helpText = help.bstrHelp;
			}
		}
		else if (iItem == tab->itemsCount)
		{
			WCHAR szBuffer[256] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_LOGINTAB_MOREPROVIDERS, szBuffer, ARRAYSIZE(szBuffer));
			tab->helpText = SysAllocString(szBuffer);
		}

		if (NULL == tab->helpText)
			tab->helpText = SysAllocString(L" ");
	}

	if (NULL != tab->helpText && L'\0' != tab->helpText)
	{
		pdisp->lpszText = (LPWSTR)tab->helpText;
		pdisp->uFlags = TTF_DI_SETITEM;
	}
}

static BOOL LoginTab_OnShow(HWND hwnd, HWND hTooltip)
{
	TOOLINFO ti;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.hwnd = hwnd;
	ti.uId = 0;

	if (FALSE == SendMessage(hTooltip, TTM_GETTOOLINFO, 0, (LPARAM)&ti))
		return FALSE;

	MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&ti.rect, 2);

	RECT windowRect, tooltipRect;
	GetWindowRect(hwnd, &windowRect);
	GetWindowRect(hTooltip, &tooltipRect);

	ti.rect.right = ti.rect.left + (tooltipRect.right - tooltipRect.left);
	ti.rect.top = windowRect.bottom;
	ti.rect.bottom = ti.rect.top + (tooltipRect.bottom - tooltipRect.top);

	HMONITOR hMonitor = MonitorFromRect(&ti.rect, MONITOR_DEFAULTTONEAREST);
	if (NULL != hMonitor)
	{
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		if (FALSE != GetMonitorInfo(hMonitor, &mi))
		{
			INT offsetX = 0;
			INT offsetY = 0;

			if (ti.rect.right > mi.rcWork.right) 
				offsetX += (mi.rcWork.right - ti.rect.right);
		
			if (ti.rect.bottom > mi.rcWork.bottom) 
			{
				offsetY += (mi.rcWork.bottom - ti.rect.bottom);
				if ((ti.rect.top + offsetY) < windowRect.bottom  && (ti.rect.bottom + offsetY) > windowRect.top)
					offsetY = (windowRect.top - (ti.rect.bottom - ti.rect.top)) - ti.rect.top;
			}
			
			if ((ti.rect.left + offsetX) < mi.rcWork.left) 
				offsetX += (mi.rcWork.left - (ti.rect.left + offsetX));
			if ((ti.rect.top + offsetY) < mi.rcWork.top) 
				offsetY += (mi.rcWork.top - (ti.rect.top + offsetY));
			

			
			
			OffsetRect(&ti.rect, offsetX, offsetY);
		}
	}

	return SetWindowPos(hTooltip, NULL, ti.rect.left, ti.rect.top, 0, 0, 
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);	
}

static LRESULT LoginTab_OnTooltipNotify(HWND hwnd, NMHDR *pnmh)
{
	switch(pnmh->code)
	{
		case TTN_GETDISPINFO:
			LoginTab_GetTootipDispInfo(hwnd, (NMTTDISPINFO*)pnmh);
			break;
		case TTN_SHOW:
			return LoginTab_OnShow(hwnd, pnmh->hwndFrom);

	}
	return 0;
}

static LRESULT LoginTab_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return 0;

	if (tab->hTooltip == pnmh->hwndFrom && NULL != tab->hTooltip)
		return LoginTab_OnTooltipNotify(hwnd, pnmh);

	return 0;
}
static INT LoginTab_OnGetIdealHeight(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return 0;

	INT height = 0;

	INT iconCX, iconCY;
	if (NULL != tab->imageList && FALSE != ImageList_GetIconSize(tab->imageList, &iconCX, &iconCY))
	{
		height += (iconCY + 2 * IMAGE_MARGIN_CY);
	}
	
	height += tab->textHeight;

	height += tab->margins.top + tab->margins.bottom;
	height += 2 * tab->frameHeight;
	
	return height;
}

static INT LoginTab_OnGetIdealWidth(HWND hwnd, INT itemsCount)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return 0;
	
	INT width = 0;

	if (itemsCount < 0) itemsCount = 0;
	if (itemsCount > tab->itemsCount) itemsCount = tab->itemsCount;

	if (itemsCount == 0)
	{
		width = tab->margins.left + tab->margins.right;
		return width;
	}
	itemsCount--;
	if (itemsCount <= tab->lastVisible)
	{
		RECT rect;
		if (FALSE != LoginTab_GetItemRect(hwnd, itemsCount, &rect))
			width = rect.right;
	}
	else
	{
		RECT rect;
		GetWindowRect(hwnd, &rect);
		LONG origWidth = rect.right - rect.left;
		LONG origHeight = rect.bottom - rect.top;
		SetWindowPos(hwnd, NULL, 0, 0, 40000, origHeight, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSENDCHANGING);

		if (FALSE != LoginTab_GetItemRect(hwnd, itemsCount, &rect))
			width = rect.right;
		
		SetWindowPos(hwnd, NULL, 0, 0, origWidth, origHeight, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSENDCHANGING);
	}

	if (0 != width)
	{
		width += (itemsCount < (tab->itemsCount - 1) && tab->chevronWidth > tab->margins.right) ? 
			tab->chevronWidth : tab->margins.right;
	}
	return width;
}
static INT LoginTab_OnInsertItem(HWND hwnd, INT iItem, NLTITEM *pItem)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab || NULL == pItem) return -1;
	if (iItem < 0) iItem = 0;
	

	INT itemsMax = (0 != tab->items) ? (INT)(_msize(tab->items)/sizeof(LOGINTABITEM*)) : 0;
	if (tab->itemsCount >= itemsMax)
	{
		INT k = tab->itemsCount + 1 - itemsMax;
		if (k < 8) k = 8;
		itemsMax += k;
		void *data = realloc(tab->items, itemsMax * sizeof(LOGINTABITEM*));
		if (NULL == data) return -1;
		tab->items = (LOGINTABITEM**)data;
		data = realloc(tab->order, itemsMax * sizeof(INT));
		if (NULL == data) return -1;
		tab->order = (INT*)data;
	}

	LOGINTABITEM *item = LoginTab_CreateItem(pItem);
	if (NULL == item) return -1;

	if (iItem >= tab->itemsCount)
	{
		iItem = tab->itemsCount;
		tab->items[iItem] = item;
		tab->order[iItem] = iItem;
	}
	else
	{		
		MoveMemory((tab->items + iItem + 1), (tab->items + iItem), sizeof(LOGINTABITEM*) * (tab->itemsCount - iItem));
		tab->items[iItem] = item;

		MoveMemory((tab->order + iItem + 1), (tab->order + iItem), sizeof(INT) * (tab->itemsCount - iItem));
		tab->order[iItem] = iItem;
		
		for (INT i = 0; i <= tab->itemsCount; i++)
		{
			if (iItem != i && tab->order[i] >= iItem)
				++(tab->order[i]);
		}
	}

	tab->itemsCount++;

	
	return iItem;
}

static BOOL LoginTab_OnSetItem(HWND hwnd, INT iItem, NLTITEM *pItem)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab || NULL == pItem) return FALSE;
	if (iItem < 0 || iItem >= tab->itemsCount) return FALSE;

	BOOL result =  LoginTab_SetItemInternal(tab->items[iItem], pItem);

	RECT rect;
	GetClientRect(hwnd, &rect);
	LONG clientRight = rect.right;

	if (FALSE != LoginTab_GetItemRect(hwnd, iItem, &rect))
	{
		rect.right = clientRight;
		InvalidateRect(hwnd, &rect, FALSE);
	}
	else if (NULL != tab->chevronMenu)
	{
		INT menuCount = GetMenuItemCount(tab->chevronMenu);
		while(menuCount--)
		{
			if (iItem == (GetMenuItemID(tab->chevronMenu, menuCount) - 1))
			{
				if (FALSE != GetMenuItemRect(NULL, tab->chevronMenu, menuCount, &rect))
				{					
					POINT ptTest;
					ptTest.x = rect.left + (rect.right - rect.left)/2;
					ptTest.y = rect.top + (rect.bottom - rect.top)/2;
					HWND hwndMenu = WindowFromPoint(ptTest);
					if (NULL != hwndMenu)
					{
						MapWindowPoints(HWND_DESKTOP, hwndMenu, (POINT*)&rect, 2);
						InvalidateRect(hwndMenu, &rect, FALSE);
					}
				}
				break;
			}
		}
		
	}

	return result;
}

static BOOL LoginTab_OnGetItem(HWND hwnd, INT iItem, NLTITEM *pItem)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab || NULL == pItem) return FALSE;
	if (iItem < 0 || iItem >= tab->itemsCount) return FALSE;

	LOGINTABITEM *item = tab->items[iItem];

	BOOL succeeded = TRUE;

	if (0 != (NLTIF_TEXT & pItem->mask))
	{
		if (NULL == pItem->pszText || 
			FAILED(StringCchCopyEx(pItem->pszText, pItem->cchTextMax, item->text, NULL, NULL, STRSAFE_IGNORE_NULLS)))
		{
			succeeded = FALSE;
		}
	}

	if (0 != (NLTIF_PARAM & pItem->mask))
		pItem->param = item->param;

	if (0 != (NLTIF_IMAGE & pItem->mask))
		pItem->iImage = item->iImage;

	if (0 != (NLTIF_IMAGE_ACTIVE & pItem->mask))
		pItem->iImageActive = item->iImageActive;
	
	if (0 != (NLTIF_IMAGE_DISABLED & pItem->mask))
		pItem->iImageDisabled = item->iImageDisabled;

	return succeeded;
}

static BOOL LoginTab_OnDeleteItem(HWND hwnd, INT iItem)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return FALSE;

	if (iItem < 0 || iItem >= tab->itemsCount)
		return FALSE;

	HWND hParent = GetAncestor(hwnd, GA_PARENT);
	if (NULL != hParent)
	{
		NMLOGINTAB nmp;
		nmp.hdr.code = NLTN_DELETEITEM;
		nmp.hdr.hwndFrom = hwnd;
		nmp.hdr.idFrom = GetWindowLongPtr(hwnd, GWLP_ID);
		nmp.iItem = iItem;
		SNDMSG(hParent, WM_NOTIFY, (WPARAM)nmp.hdr.idFrom, (LPARAM)&nmp);
	}
	
	LOGINTABITEM *item  = tab->items[iItem];

	INT shiftLen = tab->itemsCount - iItem - 1;
	if (shiftLen > 0)
	{
		
		MoveMemory((tab->items + iItem), (tab->items + (iItem + 1)), sizeof(LOGINTABITEM*)*shiftLen);

		INT iOrder = tab->itemsCount - 1;
		while(iOrder--)
		{
			if (iItem == tab->order[iOrder])
			{
				MoveMemory((tab->order + iOrder), (tab->order + (iOrder + 1)), 
							sizeof(INT)*(tab->itemsCount - iOrder - 1));
				break;
			}
		}

	}

	LoginTab_FreeItem(item);
	tab->itemsCount--;

	return TRUE;
}


static BOOL LoginTab_OnDeleteAllItems(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return FALSE;

	tab->itemsCount -= LoginTab_DeleteAllItemsReal(hwnd, tab->items, tab->itemsCount);
	return TRUE;
}		

static INT LoginTab_OnGetItemCount(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	return (NULL != tab) ? tab->itemsCount : -1;
}

static INT LoginTab_OnGetCurSel(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	return (NULL != tab) ? tab->iSelected : -1;
}		

static INT LoginTab_OnSetCurSel(HWND hwnd, INT iItem)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return -1;

	if (iItem < 0 || iItem >= tab->itemsCount)
		return -1;

	if (iItem == tab->iSelected)
		return tab->iSelected;

	INT iSelected = tab->iSelected;
	INT iFocused = tab->iFocused;

	tab->iSelected = iItem;
	
	if (tab->iFocused != tab->itemsCount)
		tab->iFocused = iItem;

	RECT rect;
	if (-1 != iSelected &&
		FALSE != LoginTab_GetItemRect(hwnd, iSelected, &rect))
	{
		InvalidateRect(hwnd, &rect, FALSE);
	}
	
	if (iFocused != tab->iFocused && -1 != iFocused && iFocused != iSelected &&
		FALSE != LoginTab_GetItemRect(hwnd, iFocused, &rect))
	{
		InvalidateRect(hwnd, &rect, FALSE);
	}

	if (-1 != tab->iSelected && 
		FALSE != LoginTab_GetItemRect(hwnd, tab->iSelected, &rect))
	{
		InvalidateRect(hwnd, &rect, FALSE);
	}

	for(INT i = (tab->lastVisible + 1); i < tab->itemsCount; i++)
	{
		if (tab->order[i] == tab->iSelected)
		{
			LoginTab_UpdateLayout(hwnd, TRUE);
			break;
		}
	}

	return iSelected;
}

		
static HIMAGELIST LoginTab_OnSetImageList(HWND hwnd, HIMAGELIST himl)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return NULL;

	HIMAGELIST old = tab->imageList;
	tab->imageList = himl;

	LoginTab_UpdateLayout(hwnd, TRUE);
	return old;
}

static HIMAGELIST LoginTab_OnGetImageList(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	return (NULL != tab) ? tab->imageList : NULL;
}

static BOOL LoginTab_OnResetOrder(HWND hwnd)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return FALSE;

	for (INT i = 0; i < tab->itemsCount; i++)
		tab->order[i] = i;
	
	LoginTab_UpdateLayout(hwnd, FALSE);
	InvalidateRect(hwnd, NULL, FALSE);
	return TRUE;
}

static void LoginTab_OnLockSelection(HWND hwnd, BOOL fLock)
{
	UINT windowStyle = GetWindowStyle(hwnd);
	if ((FALSE != fLock) != (0 != (NLTS_LOCKED & windowStyle)))
	{
		if (FALSE != fLock) 
			windowStyle |= NLTS_LOCKED;
		else
			windowStyle &= ~NLTS_LOCKED;
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
		InvalidateRect(hwnd, NULL, FALSE);
	}
}
static BOOL LoginTab_OnMeasureChevronItem(HWND hwnd, MEASUREITEMSTRUCT* pmis)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return FALSE;

	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_MENUDATA;

	if (FALSE != GetMenuInfo(tab->chevronMenu, &mi) && NULL != mi.dwMenuData)
	{
		CHEVRONMENUPAINTPARAM *pmp= (CHEVRONMENUPAINTPARAM*)mi.dwMenuData;
		pmis->itemWidth = pmp->itemWidth;
		pmis->itemHeight = pmp->itemHeight;

		return TRUE;
	}
	return FALSE;
}


static BOOL LoginTab_OnDrawChevronItem(HWND hwnd, DRAWITEMSTRUCT* pdis)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return FALSE;

	
	MENUINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_MENUDATA | MIM_BACKGROUND;

	if (FALSE == GetMenuInfo(tab->chevronMenu, &mi) || NULL == mi.dwMenuData)
		return FALSE;
	
	CHEVRONMENUPAINTPARAM *pmp= (CHEVRONMENUPAINTPARAM*)mi.dwMenuData;
	
	if (NULL == pmp->hwndMenu)
	{
		pmp->hwndMenu = WindowFromDC(pdis->hDC);
		if (NULL != pmp->hwndMenu)
		{
			SendMessage(pmp->hwndMenu, WM_CHANGEUISTATE, 
				MAKEWPARAM(UISF_HIDEACCEL | UISF_HIDEFOCUS, UIS_INITIALIZE), 0L);
		}
	}

	tab->drawStyle = LoginTab_GetDrawStyles(hwnd);
	
	if (NULL != pmp->hwndMenu)
	{	
		UINT uiState = (UINT)SendMessage(pmp->hwndMenu, WM_QUERYUISTATE, 0, 0L);
		if (0 == (UISF_HIDEFOCUS & uiState))
			tab->drawStyle |= NLTDS_FOCUSED;
		else
			tab->drawStyle &= ~NLTDS_FOCUSED;
	}


	HBITMAP hbmp = LoginTab_GetItemBitmap(tab, pdis->hDC, 
						pdis->rcItem.right - pdis->rcItem.left, 
						pdis->rcItem.bottom - pdis->rcItem.top);

	HBITMAP hbmpOrig = NULL;
	POINT viewportOrig;
	if (NULL != hbmp)
	{
		hbmpOrig = (HBITMAP)SelectObject(pmp->hdcItem, hbmp);
		SetViewportOrgEx(pmp->hdcItem, -pdis->rcItem.left, -pdis->rcItem.top, &viewportOrig);
	}

	if (NULL == mi.hbrBack)
			mi.hbrBack = GetSysColorBrush(COLOR_MENU);

	INT itemWidth = pdis->rcItem.right - pdis->rcItem.left;
	INT itemHeight = pdis->rcItem.bottom - pdis->rcItem.top;

	POINT brushOrgEx;
	SetBrushOrgEx(pmp->hdcItem, 0, -pdis->rcItem.top, &brushOrgEx);
	FillRect(pmp->hdcItem, &pdis->rcItem, mi.hbrBack);
	SetBrushOrgEx(pmp->hdcItem, brushOrgEx.x, brushOrgEx.y, NULL);

	LOGINTABITEM *item = (LOGINTABITEM*)pdis->itemData;
	INT iItem = pdis->itemID - 1;


	INT iHighlighted = tab->iHighlighted;
	
	if (0 != (ODS_SELECTED & pdis->itemState))
	{
		tab->iHighlighted = iItem;
		if (tab->iFocused != iItem && -1 != tab->iFocused)
		{
			RECT rect;
			if (FALSE != LoginTab_GetItemRect(hwnd, tab->iFocused, &rect))
				InvalidateRect(hwnd, &rect, FALSE);
		}
		tab->iFocused = iItem;
	}
	else
		tab->iFocused = -1;
	
	LoginTab_PaintItem(hwnd, pmp->hdcItem, tab, item, iItem, &pdis->rcItem, &pdis->rcItem, pmp->hdcSrc);

	tab->iHighlighted = iHighlighted;

	BitBlt(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, itemWidth, itemHeight, 
			pmp->hdcItem, pdis->rcItem.left, pdis->rcItem.top, SRCCOPY);
	
	if (NULL != hbmp)
	{
		SelectObject(pmp->hdcItem, hbmpOrig);
		SetViewportOrgEx(pmp->hdcItem, viewportOrig.x, viewportOrig.y, NULL);
	}

	return TRUE;
}
static BOOL LoginTab_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT* pmis)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return FALSE;

	switch(pmis->CtlType)
	{
		case ODT_MENU:
			if (NULL != tab->chevronMenu)
				return LoginTab_OnMeasureChevronItem(hwnd, pmis);
			break;
	}
	
	return FALSE;
}

static BOOL LoginTab_OnDrawItem(HWND hwnd, DRAWITEMSTRUCT* pdis)
{
	LOGINTAB *tab = GetTab(hwnd);
	if (NULL == tab) return FALSE;

	switch(pdis->CtlType)
	{
		case ODT_MENU:
			if (NULL != tab->chevronMenu)
				return LoginTab_OnDrawChevronItem(hwnd, pdis);
			break;
	}
	
	return FALSE;
}

void LoginTab_OnThemeChanged(HWND hwnd)
{
	LoginTab_UpdateColors(hwnd);
	OutputDebugStringA("Theme changed received\r\n");
}

void LoginTab_OnSysColorChanged(HWND hwnd)
{
	LoginTab_UpdateColors(hwnd);
	OutputDebugStringA("Color changed received\r\n");
}


static LRESULT WINAPI LoginTab_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return LoginTab_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			LoginTab_OnDestroy(hwnd); return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_PAINT:				LoginTab_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		LoginTab_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_WINDOWPOSCHANGED:	LoginTab_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SIZE:				return 0;
		case WM_MOUSEMOVE:			LoginTab_OnMouseMove(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONDOWN:		LoginTab_OnLButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONUP:			LoginTab_OnLButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_RBUTTONDOWN:		LoginTab_OnRButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_RBUTTONUP:			LoginTab_OnRButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_MBUTTONDOWN:		LoginTab_OnMButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_MBUTTONUP:			LoginTab_OnMButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_MOUSELEAVE:			LoginTab_OnMouseLeave(hwnd); return 0;
		case WM_CAPTURECHANGED:		LoginTab_OnCaptureChanged(hwnd, (HWND)lParam); return 0;
		case WM_SETFOCUS:			LoginTab_OnSetFocus(hwnd, (HWND)wParam); return 0;
		case WM_KILLFOCUS:			LoginTab_OnKillFocus(hwnd, (HWND)wParam); return 0;
		case WM_ENABLE:				LoginTab_OnEnable(hwnd, (BOOL)wParam); return 0;
		case WM_KEYDOWN:			LoginTab_OnKeyDown(hwnd, (INT)wParam, (UINT)lParam); return 0;
		case WM_GETDLGCODE:			return LoginTab_OnGetDlgCode(hwnd, (INT)wParam, (MSG*)lParam);
		case WM_MENUCHAR:			return LoginTab_OnMenuChar(hwnd, LOWORD(wParam), HIWORD(wParam), (HMENU)lParam);
		case WM_MENUSELECT:			LoginTab_OnMenuSelect(hwnd, LOWORD(wParam), HIWORD(wParam), (HMENU)lParam); return 0;
		case WM_MEASUREITEM:		return LoginTab_OnMeasureItem(hwnd, (MEASUREITEMSTRUCT*)lParam);
		case WM_DRAWITEM:			return LoginTab_OnDrawItem(hwnd, (DRAWITEMSTRUCT*)lParam);
		case WM_NOTIFY:				return LoginTab_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam);
		case WM_THEMECHANGED:		LoginTab_OnThemeChanged(hwnd); return TRUE;
		case WM_SYSCOLORCHANGE:		LoginTab_OnSysColorChanged(hwnd); return TRUE;
		
		
		case NLTM_GETIDEALHEIGHT:	return LoginTab_OnGetIdealHeight(hwnd);
		case NLTM_INSERTITEM:		return LoginTab_OnInsertItem(hwnd, (INT)wParam, (NLTITEM*)lParam);
		case NLTM_SETITEM:			return LoginTab_OnSetItem(hwnd, (INT)wParam, (NLTITEM*)lParam);
		case NLTM_GETITEM:			return LoginTab_OnGetItem(hwnd, (INT)wParam, (NLTITEM*)lParam);
		case NLTM_DELETEITEM:		return LoginTab_OnDeleteItem(hwnd, (INT)wParam);
		case NLTM_DELETEALLITEMS:	return LoginTab_OnDeleteAllItems(hwnd);
		case NLTM_GETITEMCOUNT:		return LoginTab_OnGetItemCount(hwnd);
		case NLTM_GETCURSEL:		return LoginTab_OnGetCurSel(hwnd);
		case NLTM_SETCURSEL:		return LoginTab_OnSetCurSel(hwnd, (INT)wParam);
		case NLTM_SETIMAGELIST:		return (LRESULT)LoginTab_OnSetImageList(hwnd, (HIMAGELIST)lParam);
		case NLTM_GETIMAGELIST:		return (LRESULT)LoginTab_OnGetImageList(hwnd);
		case NLTM_RESETORDER:		LoginTab_OnResetOrder(hwnd); return 0;
		case NLTM_LOCKSELECTION:	LoginTab_OnLockSelection(hwnd, (BOOL)wParam); return 0;
		case NLTM_GETIDEALWIDTH:	return LoginTab_OnGetIdealWidth(hwnd, (INT)wParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}