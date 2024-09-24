#include "main.h"
#include "./curtain.h"
#include "./graphics.h"
#include "./resource.h"

#include "../winamp/wa_dlg.h"
#include "../Plugins/General/gen_ml/ml_ipc_0313.h"

#include "./ifc_imageloader.h"
#include "./ifc_skinhelper.h"

#include <strsafe.h>
#include <windows.h>

#define WIDGET_MINWIDTH_UNITS	60 
#define WIDGET_MAXWIDTH_UNITS	200

#define WIDGET_FRAMECX			1
#define WIDGET_FRAMECY			1
#define WIDGET_SPACECX_UNITS	8
#define WIDGET_SPACECY_UNITS	8
#define WIDGET_CONTROLSPACE_UNITS	6

#define PROGRESS_FRAMECOUNT		24
#define ANIMATETIMER_ID			64
#define ANIMATETIMER_INTERVAL	1000 / PROGRESS_FRAMECOUNT

typedef struct __CURTAIN
{
	LPWSTR		pszTitle;
	LPWSTR		pszOperation;
	HFONT		textFont;
	RECT		widgetRect;
	HBRUSH		backBrush;
	HBRUSH		widgetBrush;
	HBRUSH		frameBrush;
	HBITMAP		progressBitmap;
	INT			frameNumber;
	SIZE		titleSize;
	SIZE		operationSize;
	SIZE		imageSize;
	COLORREF	rgbBk;
	COLORREF	rgbFg;
} CURTAIN;

#define GetCurtain(__hwnd) ((CURTAIN*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))

static LRESULT CALLBACK Curtain_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL Curtain_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc = {0};
	if (GetClassInfo(hInstance, NWC_ONLINEMEDIACURTAIN, &wc)) return TRUE;

	ZeroMemory(&wc, sizeof(WNDCLASS));

	wc.hInstance = hInstance;
	wc.lpszClassName = NWC_ONLINEMEDIACURTAIN;
	wc.lpfnWndProc = Curtain_WindowProc;
	wc.style = CS_DBLCLKS;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.cbWndExtra = sizeof(CURTAIN*);
	
	return ( 0 != RegisterClassW(&wc));
}

#define RA_LEFT		0x0001
#define RA_RIGHT	0x0002
#define RA_HCENTER	0x0003
#define RA_TOP		0x0010
#define RA_BOTTOM	0x0020
#define RA_VCENTER	0x0030
#define RA_FITHORZ	0x0100
#define RA_FITVERT	0x0200

static COLORREF Curtain_GetColor(INT skinColor, INT sysColor)
{
	COLORREF rgb;
	ifc_skinhelper *skinHelper = NULL;
	HRESULT hr = Plugin_GetSkinHelper(&skinHelper);
	if (SUCCEEDED(hr) && skinHelper != NULL)
	{
		hr = skinHelper->GetColor(skinColor, &rgb);
		skinHelper->Release();
	}

	if (FAILED(hr)) 
		rgb = GetSysColor(sysColor);

	return rgb;
}

static void Curtain_RectAlign(RECT *prcTarget, const RECT *prcBounds, UINT flags)
{
	if (0 != (0x0F & flags)) // horz
	{
		LONG targetWidth = prcTarget->right - prcTarget->left;
		LONG boundsWidth = prcBounds->right - prcBounds->left;

		if (0 != (RA_FITHORZ & flags) && targetWidth > boundsWidth)
			targetWidth = boundsWidth;

		if (targetWidth == boundsWidth)
		{
			prcTarget->left = prcBounds->left;
			prcTarget->right = prcBounds->right;
		}
		else
		{
			switch(0x0F & flags)
			{
				case RA_HCENTER:	prcTarget->left = prcBounds->left + (boundsWidth - targetWidth)/2; break;
				case RA_LEFT:		prcTarget->left = prcBounds->left; break;
				case RA_RIGHT:		prcTarget->left = prcBounds->right - targetWidth; break;
			}
			prcTarget->right = prcTarget->left + targetWidth; 
		}
	}

	if (0 != (0xF0 & flags)) // horz
	{
		LONG targetHeight = prcTarget->bottom - prcTarget->top;
		LONG boundsHeight = prcBounds->bottom - prcBounds->top;

		if (0 != (RA_FITVERT & flags) && targetHeight > boundsHeight)
			targetHeight = boundsHeight;

		if (targetHeight == boundsHeight)
		{
			prcTarget->top = prcBounds->top;
			prcTarget->bottom = prcBounds->bottom;
		}
		else
		{
			switch(0xF0 & flags)
			{
				case RA_VCENTER:	prcTarget->top = prcBounds->top + (boundsHeight - targetHeight)/2; break;
				case RA_TOP:		prcTarget->top = prcBounds->top; break;
				case RA_BOTTOM:		prcTarget->top = prcBounds->bottom - targetHeight; break;
			}
			prcTarget->bottom = prcTarget->top + targetHeight; 
		}
	}
}

static BOOL Curtain_SetPluginString(LPWSTR *ppDest, LPCWSTR pszSource)
{
	if (NULL != *ppDest)
		Plugin_FreeString(*ppDest);

	if(NULL == pszSource)
	{
		*ppDest = NULL;
	}
	else
	{
		WCHAR szBuffer[1024] = {0};
		if (IS_INTRESOURCE(pszSource))
		{
			Plugin_LoadString((INT)(INT_PTR)pszSource, szBuffer, ARRAYSIZE(szBuffer));
			pszSource = szBuffer;
		}
		*ppDest = Plugin_CopyString(pszSource);
	}

	return TRUE;
}

static BOOL Curtain_GetTextSize(LPCWSTR pszText, HWND hwnd, SIZE *textSize)
{
	if (NULL == hwnd || NULL == textSize)
		return FALSE;

	HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);

	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (NULL == hdc) return FALSE;
	HFONT originalFont = (HFONT)SelectObject(hdc, font);

	INT cchText = (NULL != pszText) ? lstrlen(pszText) : 0;
	BOOL result = FALSE;

	if ( 0 == cchText)
	{
		TEXTMETRIC tm = {0};
		if (GetTextMetrics(hdc, &tm))
		{
			textSize->cy = tm.tmHeight;
			textSize->cx = 0;
			result = TRUE;
		}
	}
	else
	{		
		if (GetTextExtentPoint32(hdc, pszText, cchText, textSize))
			result = TRUE;
	}

	SelectObject(hdc, originalFont);
	ReleaseDC(hwnd, hdc);

	return result;
}

static BOOL Curtain_MapDialogRectHdc(HDC hdc, RECT *prc)
{
	TEXTMETRIC tm;
	if (NULL == prc || !GetTextMetrics(hdc, &tm))
		return FALSE;

	prc->left   = MulDiv(prc->left,   tm.tmAveCharWidth, 4);
	prc->right  = MulDiv(prc->right,  tm.tmAveCharWidth, 4);
	prc->top    = MulDiv(prc->top,    tm.tmHeight, 8);
	prc->bottom = MulDiv(prc->bottom, tm.tmHeight, 8);

	return TRUE;
}

static BOOL Curtain_MapDialogRect(HWND hwnd, RECT *prc)
{	
	HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);

	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (NULL == hdc) return FALSE;
	HFONT originalFont = (HFONT)SelectObject(hdc, font);

	BOOL result = Curtain_MapDialogRectHdc(hdc, prc);

	SelectObject(hdc, originalFont);
	ReleaseDC(hwnd, hdc);
	return result;
}

static BOOL Curtain_GetImageSize(HBITMAP bitmap, INT frameCount, SIZE *imageSize)
{
	if (NULL == bitmap || 0 == frameCount || NULL == imageSize)
		return FALSE;

	BITMAP bm;
	if (sizeof(BITMAP) != GetObject(bitmap, sizeof(BITMAP), &bm))
		return FALSE;

	imageSize->cx = bm.bmWidth;
	imageSize->cy = bm.bmHeight / frameCount;

	return TRUE;
}

HBITMAP Curtain_LoadImage(HWND hwnd, INT frameCount, COLORREF rgbBk, COLORREF rgbFg)
{	
	HBITMAP frameBitmap = NULL;
	BITMAPINFOHEADER header = {0};
	BYTE *pixelData = NULL;
	ifc_omimageloader *loader = NULL;

	if (SUCCEEDED(Plugin_QueryImageLoader(Plugin_GetInstance(), MAKEINTRESOURCE(IDR_CURTAINPROGRESS_IMAGE), FALSE, &loader)) && loader != NULL)
	{
		loader->LoadBitmapEx(&frameBitmap, &header, (void**)&pixelData);
		loader->Release();
	}

	if (NULL == frameBitmap) 
		return NULL;

	if (header.biHeight < 0) header.biHeight = - header.biHeight;

	Image_Colorize(pixelData, header.biWidth, header.biHeight, header.biBitCount, rgbBk, rgbFg, TRUE);

	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);

	HBITMAP bitmap = Image_AnimateRotation(hdc, frameBitmap, frameCount, rgbBk, FALSE);

	ReleaseDC(hwnd, hdc);
	DeleteObject(frameBitmap);

	return bitmap;
}

static void Curtain_UpdateLayout(HWND hwnd, BOOL fRedraw)
{
	CURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain) return;

	RECT clientRect;
	if (!GetClientRect(hwnd, &clientRect))
		return;

	InflateRect(&clientRect, -8, -8);

	if (!Curtain_GetImageSize(curtain->progressBitmap, PROGRESS_FRAMECOUNT, &curtain->imageSize))
		ZeroMemory(&curtain->imageSize, sizeof(SIZE));

	if (!Curtain_GetTextSize(curtain->pszTitle, hwnd, &curtain->titleSize))
		ZeroMemory(&curtain->titleSize, sizeof(SIZE));

	if (!Curtain_GetTextSize(curtain->pszOperation, hwnd, &curtain->operationSize))
		ZeroMemory(&curtain->operationSize, sizeof(SIZE));

	RECT spaceRect;
	SetRect(&spaceRect, WIDGET_SPACECX_UNITS, WIDGET_SPACECY_UNITS, 0, WIDGET_CONTROLSPACE_UNITS);
	Curtain_MapDialogRect(hwnd, &spaceRect);
	INT spaceCY = spaceRect.top;
	INT intervalCY = spaceRect.bottom;

	INT widgetHeight = 2 * spaceCY + curtain->titleSize.cy;
	if (widgetHeight > (clientRect.bottom - clientRect.top))
	{
		widgetHeight = 0;
		ZeroMemory(&curtain->titleSize, sizeof(SIZE));
	}

	if (0 != widgetHeight && curtain->operationSize.cy > 0 &&
		(widgetHeight + curtain->operationSize.cy + intervalCY) < (clientRect.bottom - clientRect.top))
		widgetHeight += curtain->operationSize.cy + intervalCY;
	else
		ZeroMemory(&curtain->operationSize, sizeof(SIZE));

	if (0 != widgetHeight && curtain->imageSize.cy > 0 &&
		(widgetHeight + curtain->imageSize.cy + intervalCY) < (clientRect.bottom - clientRect.top))
		widgetHeight += curtain->imageSize.cy + intervalCY;
	else
		ZeroMemory(&curtain->imageSize, sizeof(SIZE));

	RECT limitRect, prevRect;
	CopyRect(&prevRect, &curtain->widgetRect);

	SetRect(&limitRect, WIDGET_MINWIDTH_UNITS, 0, WIDGET_MAXWIDTH_UNITS, 0);
	Curtain_MapDialogRect(hwnd, &limitRect);
	if ( 0 != widgetHeight && (clientRect.right - clientRect.left) >= limitRect.left)
	{
		curtain->widgetRect.left = clientRect.left;
		curtain->widgetRect.top = clientRect.top;

		curtain->widgetRect.right = ((clientRect.right - clientRect.left) >= limitRect.right) ? 
										(curtain->widgetRect.left + limitRect.right) : 
										(clientRect.right - clientRect.left); 

		curtain->widgetRect.bottom  = clientRect.top + widgetHeight;
		Curtain_RectAlign(&curtain->widgetRect, &clientRect, RA_HCENTER | RA_VCENTER);
	}
	else
	{
		SetRectEmpty(&curtain->widgetRect);
		ZeroMemory(&curtain->titleSize, sizeof(SIZE));
		ZeroMemory(&curtain->operationSize, sizeof(SIZE));
		ZeroMemory(&curtain->imageSize, sizeof(SIZE));
	}

	if (FALSE != fRedraw && FALSE == EqualRect(&curtain->widgetRect, &prevRect))
	{
		InvalidateRect(hwnd, &prevRect, TRUE);
		InvalidateRect(hwnd, &curtain->widgetRect, FALSE);
	}
}

static HRGN Curtain_GetFrameRgn(RECT *prcWidget)
{
	HRGN regionFrame, regionPart;

	regionFrame = CreateRectRgn(prcWidget->left, prcWidget->top, prcWidget->right, prcWidget->top + WIDGET_FRAMECY);

	regionPart = CreateRectRgn(prcWidget->left, prcWidget->bottom - WIDGET_FRAMECY, prcWidget->right, prcWidget->bottom);
	CombineRgn(regionFrame, regionFrame, regionPart, RGN_OR);
	SetRectRgn(regionPart, prcWidget->left, prcWidget->top + WIDGET_FRAMECY, 	prcWidget->left + WIDGET_FRAMECX, prcWidget->bottom - WIDGET_FRAMECY); 
	CombineRgn(regionFrame, regionFrame, regionPart, RGN_OR);
	SetRectRgn(regionPart, prcWidget->right - WIDGET_FRAMECX, prcWidget->top + WIDGET_FRAMECY, prcWidget->right, prcWidget->bottom - WIDGET_FRAMECY); 
	CombineRgn(regionFrame, regionFrame, regionPart, RGN_OR);

	DeleteObject(regionPart);
	return regionFrame;
}

static void Curtain_PaintWidgetBack(CURTAIN *curtain, HDC hdc, HRGN regionPaint)
{
	HRGN regionFrame = Curtain_GetFrameRgn(&curtain->widgetRect);

	if (NULL == curtain->frameBrush)
		curtain->frameBrush = CreateSolidBrush(Curtain_GetColor(WADLG_HILITE, COLOR_3DHILIGHT));

	if (FillRgn(hdc, regionFrame, curtain->frameBrush))
		CombineRgn(regionPaint, regionPaint, regionFrame, RGN_DIFF);

	DeleteObject(regionFrame);

	if (NULL == curtain->widgetBrush)
		curtain->widgetBrush = CreateSolidBrush(Curtain_GetColor(WADLG_WNDBG, COLOR_WINDOW));

	FillRgn(hdc, regionPaint, curtain->widgetBrush);
}

static BOOL Curtain_PaintImage(HBITMAP bitmap, INT frame, const RECT *bitmapRect, HDC hdc, const RECT *paintRect)
{
	RECT blitRect;
	if (!IntersectRect(&blitRect, bitmapRect, paintRect))
		return TRUE;

	BOOL success = FALSE;
	HDC hdcSrc = CreateCompatibleDC(hdc);
	if (NULL != hdcSrc)
	{
		HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcSrc, bitmap);

		success = BitBlt(hdc, blitRect.left, blitRect.top, 
					blitRect.right - blitRect.left, blitRect.bottom - blitRect.top, 
					hdcSrc, 
					blitRect.left - bitmapRect->left, 
					(bitmapRect->bottom - bitmapRect->top)*frame + blitRect.top - bitmapRect->top, 
					SRCCOPY);

		SelectObject(hdcSrc, hbmpOld);
		DeleteDC(hdcSrc);
	}

	return success;
}

static BOOL Curtain_PaintWidget(CURTAIN *curtain, HDC hdc, RECT *prcPaint, BOOL fErase)
{
	HRGN regionPaint = CreateRectRgnIndirect(prcPaint);
	HRGN regionPart = CreateRectRgn(0,0,0,0);

	COLORREF originalBk = SetBkColor(hdc, curtain->rgbBk);
	COLORREF originalFg = SetTextColor(hdc, curtain->rgbFg);

	RECT clientRect;
	CopyRect(&clientRect, &curtain->widgetRect);

	HFONT originalFont = (HFONT)SelectObject(hdc, (NULL != curtain->textFont) ? 
												curtain->textFont :
												(HFONT)GetStockObject(DEFAULT_GUI_FONT));

	RECT spaceRect;
	SetRect(&spaceRect, WIDGET_SPACECX_UNITS, WIDGET_SPACECY_UNITS, 0, 0);
	Curtain_MapDialogRectHdc(hdc, &spaceRect);

	InflateRect(&clientRect, -spaceRect.left, -spaceRect.top);

	if (NULL != curtain->progressBitmap && 0 != curtain->imageSize.cx && 0 != curtain->imageSize.cy)
	{
		RECT imageRect;
		SetRect(&imageRect, 0, 0, curtain->imageSize.cx, curtain->imageSize.cy);
		Curtain_RectAlign(&imageRect, &clientRect, RA_HCENTER | RA_VCENTER);

		if (Curtain_PaintImage(curtain->progressBitmap, curtain->frameNumber, &imageRect, hdc,prcPaint))
		{
			SetRectRgn(regionPart, imageRect.left, imageRect.top, imageRect.right, imageRect.bottom);
			CombineRgn(regionPaint, regionPaint, regionPart, RGN_DIFF);
		}
	}

	if (NULL != curtain->pszTitle && 0 != curtain->titleSize.cx && 0 != curtain->titleSize.cy)
	{
		RECT textRect, paintRect;;
		SetRect(&textRect, 0, 0, curtain->titleSize.cx, curtain->titleSize.cy);
		Curtain_RectAlign(&textRect, &clientRect, RA_HCENTER | RA_TOP | RA_FITHORZ);
		if (IntersectRect(&paintRect, &textRect, prcPaint))
		{
			INT cchText = lstrlen(curtain->pszTitle);
			if (ExtTextOut(hdc, textRect.left, textRect.top, ETO_CLIPPED | ETO_OPAQUE, &paintRect, curtain->pszTitle, cchText, NULL))
			{
				SetRectRgn(regionPart, paintRect.left, paintRect.top, paintRect.right, paintRect.bottom);
				CombineRgn(regionPaint, regionPaint, regionPart, RGN_DIFF);
			}
		}
	}

	if (NULL != curtain->pszOperation && 0 != curtain->operationSize.cx && 0 != curtain->operationSize.cy)
	{
		RECT textRect, paintRect;
		SetRect(&textRect, 0, 0, curtain->operationSize.cx, curtain->operationSize.cy);
		Curtain_RectAlign(&textRect, &clientRect, RA_HCENTER | RA_BOTTOM | RA_FITHORZ);
		if (IntersectRect(&paintRect, &textRect, prcPaint))
		{
			INT cchText = lstrlen(curtain->pszOperation);
			if (ExtTextOut(hdc, textRect.left, textRect.top, ETO_CLIPPED | ETO_OPAQUE, &paintRect, curtain->pszOperation, cchText, NULL))
			{
				SetRectRgn(regionPart, paintRect.left, paintRect.top, paintRect.right, paintRect.bottom);
				CombineRgn(regionPaint, regionPaint, regionPart, RGN_DIFF);
			}
		}
	}

	SelectObject(hdc, originalFont);

	if (originalBk != curtain->rgbBk) SetBkColor(hdc, originalBk);
	if (originalFg != curtain->rgbFg) SetTextColor(hdc, originalFg);

	if (NULL != fErase)
		Curtain_PaintWidgetBack(curtain, hdc, regionPaint);

	DeleteObject(regionPaint);
	DeleteObject(regionPart);
	return TRUE;
}

static void Curtain_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	CURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain) return;

	HRGN regionPaint = CreateRectRgnIndirect(prcPaint);

	if (!IsRectEmpty(&curtain->widgetRect))
	{
		RECT widgetPaintRect;
		if (IntersectRect(&widgetPaintRect, &curtain->widgetRect, prcPaint) &&
			FALSE != Curtain_PaintWidget(curtain, hdc, &widgetPaintRect, fErase))
		{
			HRGN regionWidget = CreateRectRgnIndirect(&widgetPaintRect);
			CombineRgn(regionPaint, regionPaint, regionWidget, RGN_DIFF);
			DeleteObject(regionWidget);
		}
	}

	if (FALSE != fErase)
	{
		if (NULL == curtain->backBrush)
		{
			curtain->backBrush = CreateSolidBrush(Curtain_GetColor(WADLG_ITEMBG, COLOR_WINDOW));
		}
		FillRgn(hdc, regionPaint, curtain->backBrush);
	}

	DeleteObject(regionPaint);
}

static void CALLBACK Curtain_AnimateTimerElpased(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	CURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain)
	{
		KillTimer(hwnd, idEvent);
		return;
	}

	if (NULL != curtain->progressBitmap && 0 != curtain->imageSize.cx && 0 != curtain->imageSize.cy)
	{
		curtain->frameNumber++;
		if (curtain->frameNumber >= PROGRESS_FRAMECOUNT)
			curtain->frameNumber = 0;

		RECT imageRect;
		SetRect(&imageRect, 0, 0, curtain->imageSize.cx, curtain->imageSize.cy);
		Curtain_RectAlign(&imageRect, &curtain->widgetRect, RA_HCENTER | RA_VCENTER);
		InvalidateRect(hwnd, &imageRect, FALSE);
	}
}

static LRESULT Curtain_OnCreate(HWND hwnd, CREATESTRUCT *pcs)
{
	CURTAIN *curtain = (CURTAIN*)calloc(1, sizeof(CURTAIN));
	if (NULL != curtain)
	{
		SetLastError(ERROR_SUCCESS);
		if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)curtain) && ERROR_SUCCESS != GetLastError())
		{
			free(curtain);
			curtain = NULL;
		}
	}

	if (NULL == curtain)
	{
		DestroyWindow(hwnd);
		return -1;
	}

	SetTimer(hwnd, ANIMATETIMER_ID, ANIMATETIMER_INTERVAL, Curtain_AnimateTimerElpased);
	return 0;
}

static void Curtain_OnDestroy(HWND hwnd)
{
	CURTAIN *curtain = GetCurtain(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);

	if (NULL != curtain)
	{
		if (NULL != curtain->pszTitle)
			Plugin_FreeString(curtain->pszTitle);

		if (NULL != curtain->pszOperation)
			Plugin_FreeString(curtain->pszOperation);

		if (NULL != curtain->backBrush)
			DeleteObject(curtain->backBrush);
		if (NULL != curtain->widgetBrush)
			DeleteObject(curtain->widgetBrush);
		if (NULL != curtain->frameBrush)
			DeleteObject(curtain->frameBrush);
		if (NULL != curtain->progressBitmap)
			DeleteObject(curtain->progressBitmap);

		free(curtain);
	}
}

static void Curtain_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps = {0};
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right)
			Curtain_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
		EndPaint(hwnd, &ps);
	}
}

static void Curtain_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{
	RECT clientRect;
	if (GetClientRect(hwnd, &clientRect))
	Curtain_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}

static void Curtain_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags))
		return;

	Curtain_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & pwp->flags));
}

static void Curtain_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
}

static BOOL Curtain_OnSetText(HWND hwnd, LPCWSTR pszText)
{
	CURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain) return FALSE;

	Curtain_SetPluginString(&curtain->pszTitle, pszText);

	InvalidateRect(hwnd, &curtain->widgetRect, FALSE);
	return TRUE;
}

static INT Curtain_OnGetText(HWND hwnd, LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer || cchBufferMax)
		return 0;

	pszBuffer[0] = L'\0';

	CURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain || NULL == curtain->pszTitle) return FALSE;

	INT cchTitle = lstrlenW(curtain->pszTitle);
	if (cchTitle > 0)
	{
		if (cchTitle >= cchBufferMax)
			cchTitle = (cchBufferMax - 1);
		StringCchCopyN(pszBuffer, cchBufferMax, curtain->pszTitle, cchTitle);
	}
	return cchTitle;
}

static INT Curtain_OnGetTextLength(HWND hwnd)
{
	CURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain || NULL == curtain->pszTitle) return FALSE;
	return lstrlenW(curtain->pszTitle);
}

static BOOL Curtain_OnSetOperationText(HWND hwnd, LPCWSTR pszText)
{
	CURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain) return FALSE;

	Curtain_SetPluginString(&curtain->pszOperation, pszText);

	InvalidateRect(hwnd, &curtain->widgetRect, FALSE);
	return TRUE;
}

static void Curtain_OnUpdateSkin(HWND hwnd, BOOL fRedraw)
{
	CURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain) return;

	ifc_skinhelper *skinHelper = NULL;
	if (FAILED(Plugin_GetSkinHelper(&skinHelper)))
		skinHelper = NULL;

	curtain->textFont = (NULL != skinHelper) ? skinHelper->GetFont() : NULL;

	if (NULL != curtain->backBrush)
	{
		DeleteObject(curtain->backBrush);
		curtain->backBrush = NULL;
	}
	if (NULL != curtain->widgetBrush)
	{
		DeleteObject(curtain->widgetBrush);
		curtain->widgetBrush = NULL;
	}
	if (NULL != curtain->frameBrush)
	{
		DeleteObject(curtain->frameBrush);
		curtain->frameBrush = NULL;
	}
	if (NULL != curtain->progressBitmap)
		DeleteObject(curtain->progressBitmap);

	if (NULL == skinHelper || FAILED(skinHelper->GetColor(WADLG_WNDBG, &curtain->rgbBk)))
		curtain->rgbBk = GetSysColor(COLOR_WINDOW);

	if (NULL == skinHelper || FAILED(skinHelper->GetColor(WADLG_WNDFG, &curtain->rgbFg)))
		curtain->rgbFg = GetSysColor(COLOR_WINDOWTEXT);

	if (NULL != skinHelper)
		skinHelper->Release();

	curtain->progressBitmap = Curtain_LoadImage(hwnd, PROGRESS_FRAMECOUNT, curtain->rgbBk, curtain->rgbFg);
	Curtain_UpdateLayout(hwnd, fRedraw);
}

static LRESULT Curtain_OnGetFont(HWND hwnd)
{
	CURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain) return NULL;
	return (LRESULT)curtain->textFont;
}

static void Curtain_OnSetFont(HWND hwnd, HFONT hFont, BOOL fRedraw)
{
	CURTAIN *curtain = GetCurtain(hwnd);
	if (NULL == curtain) return;

	curtain->textFont = hFont;
	if (FALSE != fRedraw)
		InvalidateRect(hwnd, &curtain->widgetRect, FALSE);
}

static LRESULT CALLBACK Curtain_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:				return Curtain_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			Curtain_OnDestroy(hwnd); break;
		case WM_PAINT:				Curtain_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:		Curtain_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_WINDOWPOSCHANGED:	Curtain_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SETTEXT:			return Curtain_OnSetText(hwnd, (LPCWSTR)lParam);
		case WM_GETTEXT:			return Curtain_OnGetText(hwnd, (LPWSTR)lParam, (INT)wParam);
		case WM_GETTEXTLENGTH:		return Curtain_OnGetTextLength(hwnd);
		case WM_GETFONT:			return Curtain_OnGetFont(hwnd);
		case WM_SETFONT:			Curtain_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam));
		case WM_COMMAND:			Curtain_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;

		case CWM_SETOPERATIONTEXT:	return Curtain_OnSetOperationText(hwnd, (LPCWSTR)lParam);
		case CWM_UPDATESKIN:		Curtain_OnUpdateSkin(hwnd, (BOOL)lParam); return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}