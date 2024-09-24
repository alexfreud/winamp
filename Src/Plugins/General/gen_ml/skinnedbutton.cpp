#include "main.h"
#include "./skinnedbutton.h"
#include "../winamp/wa_dlg.h"
#include "./skinning.h"
#include "./stockobjects.h"
#include "./colors.h"
#include "./ml_imagefilter.h"
#include "./ml_imagelist.h"
#include "./resource.h"


#include <math.h>
#include <strsafe.h>

#define MARGIN_TOP		2
#define MARGIN_BOTTOM	2
#define MARGIN_LEFT		6
#define MARGIN_RIGHT	6
#define IMAGE_SPACE		3
#define SPLITAREA_WIDTH	19
#define SPLITAREA_SPACE	-5
#define SPLITAREA_SPACE_ALT	-3

#define IMAGEFILTER_BLEND			0x00
#define IMAGEFILTER_BLENDPLUSCOLOR	0x01

// internaly used button styles
#define SWBS_SPLITSETMANUAL	0x10000000 
#define SWBS_SPLITHOVER		0x20000000	
#define SWBS_BUTTONHOVER	0x40000000	

// additional colors
typedef struct _EXTENDEDCOLORTABLE
{
	BOOL	 updateRequired;
	COLORREF rgbTextBk;
	BOOL	 bTextBkUnified;
	COLORREF rgbTextBkDown;
	BOOL	 bTextBkDownUnified;
	COLORREF rgbTextDisabled;
	COLORREF rgbTextDisabled2; // disabled based on WNDBG
	COLORREF rgbText15;
	COLORREF rgbText15Down;
	COLORREF rgbText75;
	COLORREF rgbText75Down;
	COLORREF rgbToolBkHigh;
	COLORREF rgbToolBkPressed;

} EXTENDEDCOLORTABLE;

typedef struct _IMAGEFILTERPARAM
{
	BUTTONPAINTSTRUCT	*pbps;
	RECT				*prcImage;
} IMAGEFILTERPARAM;

extern HMLIMGFLTRMNGR hmlifMngr;	// default gen_ml fitler manager

static EXTENDEDCOLORTABLE extendedColors = { TRUE, 0, };
static HWND hoverHWND = NULL;
static BOOL hoverSplit = FALSE;
static HMLIMGLST hmlilSplitter = NULL;
static IMAGEFILTERPARAM imagefilterParam;
static HWND lButtonDownHWND = NULL;

static HMLIMGLST CreateSplitterImageList()
{
	HMLIMGLST hmlil = MLImageListI_Create(6, 4, MLILC_COLOR32, 2, 1, 3, hmlifMngr);
	if (hmlil) 
	{
		MLIMAGESOURCE_I mlis = {0};
		mlis.type = SRC_TYPE_PNG_I;
		mlis.hInst = plugin.hDllInstance;
		mlis.bpp = 32;
		mlis.flags = ISF_FORCE_BPP_I;
		mlis.lpszName = MAKEINTRESOURCEW(IDB_SPLITARROW);
		MLImageListI_Add(hmlil, &mlis, MLIF_BUTTONBLENDPLUSCOLOR_UID, 0);
		mlis.lpszName = MAKEINTRESOURCEW(IDB_SPLITARROW_PRESSED);
		MLImageListI_Add(hmlil, &mlis, MLIF_BUTTONBLENDPLUSCOLOR_UID, 0);
	}
	return hmlil;
}

static void DrawButtonImage(HMLIMGLST hmlil, INT hmlilIndex, RECT *prcImage, BUTTONPAINTSTRUCT *pbps)
{
	INT offset;

	HIMAGELIST himl = MLImageListI_GetRealList(hmlil);
	imagefilterParam.pbps = pbps;
	imagefilterParam.prcImage = prcImage;

	if (BST_PUSHED & pbps->buttonState) 
		offset = 10000;
	else if (WS_DISABLED & pbps->windowStyle) 
		offset = 20000;
	else if (SWBS_BUTTONHOVER & pbps->skinnedStyle) 
		offset = 30000;
	else 
		offset = 0;

	BOOL bUN; 
	if (SWBS_TOOLBAR & pbps->skinnedStyle)
	{
		bUN = TRUE; 
		offset += 40000;
		if (offset == 40000 && (WS_BORDER & pbps->windowStyle)) 
		{
			offset = 40000 + 30000;
		}
	}
	else
	{
		bUN = ((BST_PUSHED & pbps->buttonState) ? 
				extendedColors.bTextBkDownUnified : 
				extendedColors.bTextBkUnified);
	}

	COLORREF colorBk, colorFg;
	colorFg = (COLORREF)(INT_PTR)WADlg_getBitmap();
	colorBk = (COLORREF)MAKELONG(prcImage->bottom - prcImage->top, ((bUN) ? prcImage->top : 0) + offset);

	INT index = MLImageListI_GetRealIndex(hmlil, hmlilIndex, colorBk, colorFg);

	if (-1 != index) 
		ImageList_Draw(himl, index, pbps->hdc, prcImage->left, prcImage->top, ILD_NORMAL);
}

typedef struct _DIBDATA
{
	BYTE		*pData;
	INT		bpp;
	INT		cx;
	INT		cy;
	INT		step;
	LONG	pitch;
	BOOL	bFree;
} DIBDATA;

static BYTE* GetButtonDib(DIBDATA *pdd, BOOL bPressed)
{
	BITMAPINFOHEADER bi;
	HBITMAP hbmp;

	if (!pdd) return NULL;
	ZeroMemory(pdd, sizeof(DIBDATA));

	hbmp = WADlg_getBitmap();
	if (!hbmp)  return NULL;

	HDC hdc = (HDC)MlStockObjects_Get(CACHED_DC);

	ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	if (GetDIBits(hdc, hbmp, 0, 0, NULL, (BITMAPINFO*)&bi, DIB_RGB_COLORS))
	{
		bi.biCompression = BI_RGB;

		pdd->bFree = TRUE;
		pdd->step = (bi.biBitCount>>3);
		pdd->pitch = bi.biWidth * pdd->step;
		while (pdd->pitch%4) pdd->pitch++;

		pdd->pData = (BYTE*)malloc(7 * pdd->pitch);
		pdd->cx = bi.biWidth;

		INT o = (bPressed) ? 15 : 0;
		pdd->cy = GetDIBits(hdc, hbmp, 
							((bi.biHeight > 0) ? (bi.biHeight - (11 + o)) : (4 + o)), 7,
							pdd->pData, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
		if (!pdd->cy)
		{
			free(pdd->pData);
			pdd->pData = NULL;
		}
	}
	return pdd->pData;
}

COLORREF SkinnedButton::GetButtonBkColor(BOOL bPressed, BOOL *pbUnified)
{
	DIBDATA dib;
	BOOL bUnified = TRUE;
	ULONG r(0), g(0), b(0);
	int cy;

	if (GetButtonDib(&dib, bPressed))
	{						
		cy = dib.cy;
		BYTE *line = dib.pData + (dib.step * 4);
		DWORD p = *(DWORD*)line;

		for (; cy-- != 0; line += dib.pitch)
		{	
			r += line[2];
			g += line[1];
			b += line[0];
			if (bUnified && p != *(DWORD*)line) bUnified = FALSE;
		}
		if (dib.bFree) free(dib.pData);
	}

	if (pbUnified) *pbUnified = bUnified;
	cy = (dib.cy ? dib.cy : 1);
	return RGB(r/cy, g/cy, b/cy);
}

static void UpdateExtendedColorTable()
{
	COLORREF fg, bk;
	float r;

	extendedColors.updateRequired = FALSE;
	extendedColors.rgbTextBk = SkinnedButton::GetButtonBkColor(FALSE, &extendedColors.bTextBkUnified);
	extendedColors.rgbTextBkDown = SkinnedButton::GetButtonBkColor(TRUE, &extendedColors.bTextBkDownUnified);
	extendedColors.rgbTextDisabled = WADlg_getColor(WADLG_BUTTONFG);

	r = 77;
	do
	{
		r -= 1;
		fg = BlendColors(extendedColors.rgbTextDisabled, extendedColors.rgbTextBk, (COLORREF)r);
	} while (GetColorDistance(fg, extendedColors.rgbTextBk) > 70);
	extendedColors.rgbTextDisabled = fg;

	extendedColors.rgbTextDisabled2 = WADlg_getColor(WADLG_WNDFG);
	bk = WADlg_getColor(WADLG_WNDBG);

	r = 77;
	do
	{
		r -= 1;
		fg = BlendColors(extendedColors.rgbTextDisabled2, bk, (COLORREF)r);
	} while (GetColorDistance(fg, bk) > 70);
	extendedColors.rgbTextDisabled2 = fg;

	fg = WADlg_getColor(WADLG_BUTTONFG);
	extendedColors.rgbText15 = BlendColors(fg, extendedColors.rgbTextBk, 40);
	extendedColors.rgbText75 = BlendColors(fg, extendedColors.rgbTextBk, 191);
	extendedColors.rgbText15Down = BlendColors(fg, extendedColors.rgbTextBkDown, 40);
	extendedColors.rgbText75Down = BlendColors(fg, extendedColors.rgbTextBkDown, 191);

	fg = WADlg_getColor(WADLG_HILITE);
	extendedColors.rgbToolBkHigh = BlendColors(fg, bk, 64);
	extendedColors.rgbToolBkPressed = BlendColors(fg, bk, 127);
}

SkinnedButton::SkinnedButton(void) : SkinnedWnd(FALSE)
{
	ZeroMemory(&imagelist, sizeof(MLBUTTONIMAGELIST));
}

SkinnedButton::~SkinnedButton(void)
{
}

BOOL SkinnedButton::Attach(HWND hwndButton)
{
	if(!SkinnedWnd::Attach(hwndButton)) return FALSE;
	SetType(SKINNEDWND_TYPE_BUTTON);
	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	return TRUE;
}

void SkinnedButton::OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw)
{
	extendedColors.updateRequired = TRUE;
	__super::OnSkinChanged(bNotifyChildren, bRedraw);
}

static void PushButton_DrawBackground(BUTTONPAINTSTRUCT *pbps)
{
	if (SWBS_TOOLBAR & pbps->skinnedStyle) 
	{
		SetBkColor(pbps->hdc, pbps->rgbTextBk);
		ExtTextOutW(pbps->hdc, 0, 0, ETO_OPAQUE, &pbps->rcPaint, NULL, 0, NULL);
		if (WS_BORDER & pbps->windowStyle) FrameRect(pbps->hdc, &pbps->rcClient, (HBRUSH)MlStockObjects_Get(HILITE_BRUSH));
	}
	else 
	{
		HBITMAP hbmp = WADlg_getBitmap();
		if (hbmp)
		{
			INT stretchModeOld;
			INT yoffs = (BST_PUSHED & pbps->buttonState) ? 15 : 0;
			HDC hdcSrc = (HDC)MlStockObjects_Get(CACHED_DC);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcSrc, hbmp);
			SetBrushOrgEx(pbps->hdc, pbps->rcClient.left, pbps->rcClient.top, NULL);
			stretchModeOld = SetStretchBltMode(pbps->hdc, COLORONCOLOR);

			if (pbps->rcPaint.top <=4)
			{
				if (pbps->rcPaint.left <= 4) 
					BitBlt(pbps->hdc, pbps->rcPaint.left, pbps->rcPaint.top, 4 - pbps->rcPaint.left, 4 - pbps->rcPaint.top, 
							hdcSrc, pbps->rcPaint.left, pbps->rcPaint.top + yoffs, SRCCOPY); // top left
				if (pbps->rcPaint.right > 4 && pbps->rcPaint.left < pbps->rcClient.right - 4) 
					StretchBlt(pbps->hdc, 4, pbps->rcClient.top, pbps->rcClient.right -4 -4, 4, 
								hdcSrc, 4, 0 + yoffs, 47-4-4, 4, SRCCOPY); // top center

				if (pbps->rcPaint.right >= pbps->rcClient.right - 4) 
					BitBlt(pbps->hdc, pbps->rcClient.right - 4, pbps->rcPaint.top, 4, 4 - pbps->rcPaint.top, 
            				hdcSrc, 47 - 4, pbps->rcPaint.top + yoffs, SRCCOPY); // top right
			}
			if (pbps->rcPaint.left <= 4) 
				StretchBlt(pbps->hdc, 0, 4, 4, pbps->rcClient.bottom - 4 -4, hdcSrc, 0, 4 + yoffs, 4, 15-4-4, SRCCOPY); // left edge
			if (pbps->rcPaint.right >= pbps->rcClient.right - 4) 
				StretchBlt(pbps->hdc, pbps->rcClient.right - 4, pbps->rcClient.top + 4, 4, pbps->rcClient.bottom - 4 - 4, hdcSrc, 47-4,4+yoffs, 4, 15-4-4,SRCCOPY); // right edge

			// center
			if ((pbps->rcPaint.right > 4 && pbps->rcPaint.left < pbps->rcClient.right - 4) &&
				(pbps->rcPaint.bottom > 4 && pbps->rcPaint.top < pbps->rcClient.bottom- 4))
				StretchBlt(pbps->hdc, 4, 4, pbps->rcClient.right-4-4, pbps->rcClient.bottom -4 -4, 
							hdcSrc, 4,4+yoffs,47-4-4,15-4-4,SRCCOPY);

			if (pbps->rcPaint.bottom >= pbps->rcClient.bottom - 4)
			{
				if (pbps->rcPaint.left <= 4) 
					BitBlt(pbps->hdc, 0, pbps->rcClient.bottom-4,4,4, hdcSrc, 0, 15-4+yoffs, SRCCOPY); // bottom left
				if (pbps->rcPaint.right > 4 && pbps->rcPaint.left < pbps->rcClient.right - 4) 
					StretchBlt(pbps->hdc, 4, pbps->rcClient.bottom-4, pbps->rcClient.right-4-4,4, hdcSrc, 4,15-4+yoffs,47-4-4,4,SRCCOPY); // bottom center
				if (pbps->rcPaint.right >= pbps->rcClient.right - 4) 
					BitBlt(pbps->hdc, pbps->rcClient.right - 4, pbps->rcClient.bottom - 4, 4, 4, hdcSrc, 47-4, 15-4+yoffs, SRCCOPY); // bottom right
			}

			SelectObject(hdcSrc, hbmpOld);
			if (COLORONCOLOR != stretchModeOld) SetStretchBltMode(pbps->hdc, stretchModeOld);
		}
	}
}

static void PushButton_CalulateRects(BUTTONPAINTSTRUCT *pbps, RECT *prcText, RECT *prcImage, INT imageWidth, INT imageHeight)
{
	SetRect(prcText, 0, 0, 0, 0);
	SetRect(prcImage, 0, 0, 0, 0);

	if (!pbps->hImage &&  !pbps->cchText) return;

	if (0 != (0x0F & pbps->textFormat))
	{
		INT l, t;

		if (pbps->cchText)
		{
			if (FALSE == DrawTextW(pbps->hdc, pbps->szText, pbps->cchText, prcText, 
				(~0x0F & pbps->textFormat) | DT_CALCRECT))
			{
				SetRectEmpty(prcText);
			}

			if (imageWidth) prcText->right += (imageWidth + IMAGE_SPACE);
		}		
		else SetRect(prcText, 0, 0, imageWidth, imageHeight);

		if (DT_BOTTOM & pbps->textFormat) t = pbps->rcClient.bottom - prcText->bottom - MARGIN_BOTTOM;
		else if (DT_VCENTER & pbps->textFormat) t = pbps->rcClient.top + ((pbps->rcClient.bottom - pbps->rcClient.top) - prcText->bottom)/2;
		else t = pbps->rcClient.top + MARGIN_TOP;

		if (DT_RIGHT & pbps->textFormat) 
		{
			l = pbps->rcClient.right - prcText->right;
			if (SWBS_DROPDOWNBUTTON & pbps->skinnedStyle)
				l -= (SPLITAREA_WIDTH + SPLITAREA_SPACE);
			if (SWBS_SPLITBUTTON & pbps->skinnedStyle)
				l -= (SPLITAREA_WIDTH + SPLITAREA_SPACE_ALT);
			else
				l -= MARGIN_RIGHT;
		}

		else if (DT_CENTER & pbps->textFormat) 
		{
			int w = pbps->rcClient.right - pbps->rcClient.left;
			if (SWBS_DROPDOWNBUTTON & pbps->skinnedStyle)
			{
				w -= (SPLITAREA_WIDTH + SPLITAREA_SPACE);
			}
			else if (SWBS_SPLITBUTTON & pbps->skinnedStyle)
			{
				w -= (SPLITAREA_WIDTH + SPLITAREA_SPACE_ALT);
			}
			l = pbps->rcClient.left + (w - prcText->right)/2;
		}
		else 
			l = pbps->rcClient.left + MARGIN_LEFT;

		if (BST_PUSHED & pbps->buttonState && (0 == (BS_PUSHLIKE & pbps->windowStyle)))
		{
			if (SWBS_DROPDOWNBUTTON & pbps->skinnedStyle) t++; 
			else 
			{
				l++;
				if (SWBS_TOOLBAR & pbps->skinnedStyle) t++;
			}
		}
		OffsetRect(prcText, l, t);
	}
	else
	{
		SetRect(prcText, MARGIN_LEFT , MARGIN_TOP,
				pbps->rcClient.right - MARGIN_RIGHT, pbps->rcClient.bottom - MARGIN_BOTTOM);
		
		if (0 != (BST_PUSHED & pbps->buttonState))
			prcText +=1;
	}

	// Fit Text Rect
	if (prcText->top < (pbps->rcClient.top + MARGIN_TOP))
	{
		LONG h = prcText->bottom - prcText->top;
		prcText->top = pbps->rcClient.top + MARGIN_TOP;
		if ((BST_PUSHED & pbps->buttonState) && (SWBS_DROPDOWNBUTTON & pbps->skinnedStyle)) prcText->top++;
		prcText->bottom = prcText->top + h;
	}
	if (prcText->bottom > (pbps->rcClient.bottom - MARGIN_BOTTOM)) prcText->bottom = pbps->rcClient.bottom - MARGIN_BOTTOM;

	if (prcText->left < (pbps->rcClient.left + MARGIN_LEFT))
	{
		LONG w = prcText->right - prcText->left;
		prcText->left = pbps->rcClient.left + MARGIN_LEFT;
		if ((BST_PUSHED & pbps->buttonState) && 0 == (SWBS_DROPDOWNBUTTON & pbps->skinnedStyle)) prcText->left++;
		prcText->right = prcText->left + w - ((BST_PUSHED & pbps->buttonState) ? 1 : 0);
	}

	int lim = pbps->rcClient.right;
	if (SWBS_SPLITBUTTON & pbps->skinnedStyle) 
	{
		lim -= (SPLITAREA_WIDTH + SPLITAREA_SPACE_ALT);
		if (BST_PUSHED & pbps->buttonState) lim++;
	}
	else if (SWBS_DROPDOWNBUTTON & pbps->skinnedStyle)
		lim -= (SPLITAREA_WIDTH + SPLITAREA_SPACE);
	else
		lim -= MARGIN_RIGHT;

	if (prcText->right > lim) prcText->right = lim;

	if (imageHeight && imageWidth)
	{
		INT top;
		if (prcText->left + imageWidth > prcText->right) imageWidth = prcText->right - prcText->left;

		top = prcText->top;
		if (prcText->top + imageHeight > prcText->bottom)
		{
			if ((DT_BOTTOM | DT_VCENTER) & pbps->textFormat)
			{
				if (DT_BOTTOM & pbps->textFormat) top = prcText->bottom -imageHeight;
				else top += ((prcText->bottom - prcText->top) - imageHeight) / 2;
				if (top < pbps->rcClient.top + MARGIN_TOP) top = pbps->rcClient.top + MARGIN_TOP;
			}
			if (top + imageHeight > (pbps->rcClient.bottom - MARGIN_BOTTOM)) imageHeight = pbps->rcClient.bottom - top - MARGIN_BOTTOM;
		}

		if ((DT_VCENTER & pbps->textFormat) && (prcText->bottom - top) > imageHeight)
			top += ((prcText->bottom - top) - imageHeight)/2;

		SetRect(prcImage, prcText->left, top, prcText->left + imageWidth, top + imageHeight);
		prcText->left += (imageWidth + IMAGE_SPACE);
	}
}

void SkinnedButton::DrawPushButton(BUTTONPAINTSTRUCT *pbps)
{	
	RECT rcText, rcImage;
	SetRect(&rcText, 0, 0, 0, 0);
	SetRect(&rcImage, 0, 0, 0, 0);	
	if (pbps->hImage)
	{
		if (-1 == pbps->imageIndex)
		{
			BITMAP bi;
			if (GetObjectW(pbps->hImage, sizeof(BITMAP), &bi)) { rcImage.right = bi.bmWidth; rcImage.bottom = bi.bmHeight; }
		}
		else
		{
			MLImageListI_GetImageSize((HMLIMGLST)pbps->hImage, (INT*)&rcImage.right,  (INT*)&rcImage.bottom);
		}
	}

	PushButton_CalulateRects(pbps, &rcText, &rcImage, rcImage.right, rcImage.bottom);
	PushButton_DrawBackground(pbps);
	IntersectClipRect(pbps->hdc, pbps->rcClient.left + MARGIN_LEFT, pbps->rcClient.top + MARGIN_TOP, 
							pbps->rcClient.right - MARGIN_RIGHT, pbps->rcClient.bottom - MARGIN_BOTTOM);

	if (rcImage.left != rcImage.right)
	{				
		if (-1 == pbps->imageIndex)
		{
			HDC hdcSrc = (HDC)MlStockObjects_Get(CACHED_DC);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcSrc, pbps->hImage);
			BitBlt(pbps->hdc, rcImage.left, rcImage.top, rcImage.right - rcImage.left, rcImage.bottom - rcImage.top, hdcSrc, 0, 0, SRCCOPY);
			SelectObject(hdcSrc, hbmpOld);
		}
		else DrawButtonImage((HMLIMGLST)pbps->hImage, pbps->imageIndex, &rcImage, pbps);
	}

	if (0 != pbps->cchText)  // draw text
	{	
		INT bkModeOld = SetBkMode(pbps->hdc, TRANSPARENT);
   		DrawTextW(pbps->hdc, pbps->szText, pbps->cchText, &rcText, (~0x0F & pbps->textFormat) | DT_WORD_ELLIPSIS | DT_NOCLIP);
		if (TRANSPARENT != bkModeOld) SetBkMode(pbps->hdc, bkModeOld);
	}

	if ((BST_FOCUS & pbps->buttonState) ||(SWBS_SPLITBUTTON & pbps->skinnedStyle))
	{
		HRGN rgn;
		rgn = CreateRectRgnIndirect(&pbps->rcClient);
		SelectClipRgn(pbps->hdc, rgn);
		DeleteObject(rgn);
	}

	if ((SWBS_SPLITBUTTON | SWBS_DROPDOWNBUTTON) & pbps->skinnedStyle)
	{
		if (NULL == hmlilSplitter) hmlilSplitter = CreateSplitterImageList();
		if (hmlilSplitter)
		{
			RECT rs;
			INT left = pbps->rcClient.right - SPLITAREA_WIDTH;
			if ((BST_PUSHED & pbps->buttonState) && 0 == (SWBS_DROPDOWNBUTTON & pbps->skinnedStyle)) left++;

			SetRect(&rs, 0, 0, 0, 0);
			MLImageListI_GetImageSize(hmlilSplitter, (INT*)&rs.right, (INT*)&rs.bottom);

			OffsetRect(&rs, left + (SPLITAREA_WIDTH - rs.right)/2 - (SPLITAREA_WIDTH - rs.right)%2, 
				pbps->rcClient.top + (pbps->rcClient.bottom - pbps->rcClient.top - rs.bottom)/2 + 1 + 
				((SWBS_SPLITPRESSED & pbps->skinnedStyle) ? 1 : 0));
			if (rs.left > (pbps->rcClient.left + 4) && rs.top > (pbps->rcClient.top  + 2))
				DrawButtonImage(hmlilSplitter, ((SWBS_SPLITPRESSED | SWBS_SPLITHOVER) & pbps->skinnedStyle) ? 1 : 0,
								&rs, pbps);

			if (SWBS_SPLITBUTTON  & pbps->skinnedStyle)
			{
				HPEN pen = (HPEN)GetStockObject(DC_PEN);
				HPEN penOld = (HPEN)SelectObject(pbps->hdc, pen);

				SetDCPenColor(pbps->hdc, (BST_PUSHED & pbps->buttonState) ? extendedColors.rgbText75Down : 
				((WS_DISABLED & pbps->windowStyle)? extendedColors.rgbText15 : extendedColors.rgbText75));
				MoveToEx(pbps->hdc, left, pbps->rcClient.top + (MARGIN_TOP + 1), NULL);
				LineTo(pbps->hdc, left, pbps->rcClient.bottom - (MARGIN_BOTTOM  + 1));

				left++;
				SetDCPenColor(pbps->hdc, (BST_PUSHED & pbps->buttonState) ? extendedColors.rgbText15Down : extendedColors.rgbText15);
				MoveToEx(pbps->hdc, left, pbps->rcClient.top + (MARGIN_TOP + 1), NULL);
				LineTo(pbps->hdc, left, pbps->rcClient.bottom - (MARGIN_BOTTOM  + 1));

				if (SWBS_SPLITPRESSED & pbps->skinnedStyle)
				{
					MoveToEx(pbps->hdc, left, pbps->rcClient.top + (MARGIN_TOP + 1), NULL);
					LineTo(pbps->hdc, pbps->rcClient.right - 2, pbps->rcClient.top + (MARGIN_TOP + 1));
				}
				SelectObject(pbps->hdc, penOld);
			}
		}
	}

	if (BST_FOCUS & pbps->buttonState) 
	{
		if (SWBS_TOOLBAR & pbps->skinnedStyle) 
		{
			FrameRect(pbps->hdc, &pbps->rcClient, (HBRUSH)MlStockObjects_Get(HILITE_BRUSH));
		}
		else
		{
			COLORREF fg, bk;
			RECT rf; 

			SetRect(&rf, pbps->rcClient.left + 2, pbps->rcClient.top + 2, pbps->rcClient.right - 2, pbps->rcClient.bottom - 2);
			if (SWBS_SPLITBUTTON & pbps->skinnedStyle) rf.right = pbps->rcClient.right - SPLITAREA_WIDTH - 1;
			fg = SetTextColor(pbps->hdc, GetSysColor(COLOR_WINDOWTEXT));
			bk = SetBkColor(pbps->hdc, GetSysColor(COLOR_WINDOW));
			DrawFocusRect(pbps->hdc, &rf);
			SetTextColor(pbps->hdc, fg);
			SetBkColor(pbps->hdc, bk);	
		}
	}
}

void SkinnedButton::DrawGroupBox(BUTTONPAINTSTRUCT *pbps)
{
}

void SkinnedButton::DrawCheckBox(BUTTONPAINTSTRUCT *pbps)
{
	ExtTextOutW(pbps->hdc, 0, 0, ETO_OPAQUE, &pbps->rcPaint, L"", 0, NULL);
	if (pbps->cchText) DrawTextW(pbps->hdc, pbps->szText, pbps->cchText, &pbps->rcClient, pbps->textFormat); 
}

void SkinnedButton::DrawRadioButton(BUTTONPAINTSTRUCT *pbps)
{
	ExtTextOutW(pbps->hdc, 0, 0, ETO_OPAQUE, &pbps->rcPaint, L"", 0, NULL);
	if (pbps->cchText) DrawTextW(pbps->hdc, pbps->szText, pbps->cchText, &pbps->rcClient, pbps->textFormat); 
}

void SkinnedButton::DrawButton(BUTTONPAINTSTRUCT *pbps)
{
	HFONT hfo(NULL);
	COLORREF rgbTextOld, rgbTextBkOld; 

	if (pbps->hFont) hfo = (HFONT)SelectObject(pbps->hdc, pbps->hFont);
	rgbTextOld = SetTextColor(pbps->hdc, pbps->rgbText);
	rgbTextBkOld = SetBkColor(pbps->hdc, pbps->rgbTextBk);

	INT type;
	if (BS_PUSHLIKE & pbps->windowStyle) type = BS_PUSHBUTTON;
	else type = (BS_TYPEMASK & pbps->windowStyle);

	switch(type)
	{
		case BS_3STATE:
		case BS_AUTO3STATE:
		case BS_AUTOCHECKBOX:
		case BS_CHECKBOX:	
			DrawCheckBox(pbps);
			break;
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
			DrawRadioButton(pbps);
			break;
		case BS_GROUPBOX:
			DrawGroupBox(pbps);
			break;
		case BS_PUSHBUTTON:
		case BS_DEFPUSHBUTTON:
			DrawPushButton(pbps);
			break;
	}

	SetTextColor(pbps->hdc, rgbTextOld);
	SetBkColor(pbps->hdc, rgbTextBkOld);
	if (NULL != hfo) SelectObject(pbps->hdc, hfo);
}

void SkinnedButton::PrePaint(HWND hwnd, BUTTONPAINTSTRUCT *pbps, UINT skinStyle, UINT uiState)
{	
	GetClientRect(hwnd, &pbps->rcClient);
	pbps->windowStyle = (UINT)GetWindowLongPtrW(hwnd, GWL_STYLE);
	pbps->buttonState = (UINT)SendMessageW(hwnd, BM_GETSTATE, 0, 0L);
	pbps->skinnedStyle = skinStyle;

	if (hoverHWND == hwnd)
	{
		if (hoverSplit) pbps->skinnedStyle |= SWBS_SPLITHOVER;
		else pbps->skinnedStyle |= SWBS_BUTTONHOVER;
	}

	if ((BST_FOCUS & pbps->buttonState) && (0x01/*UISF_HIDEFOCUS*/ & uiState))
	{
		pbps->buttonState &= ~BST_FOCUS;
	}

	if (SWBS_DROPDOWNBUTTON & pbps->skinnedStyle)
	{
		if (hoverHWND == hwnd) pbps->skinnedStyle |= SWBS_BUTTONHOVER;
		if (SWBS_SPLITPRESSED & pbps->skinnedStyle) pbps->buttonState |= BST_PUSHED;
	}

	pbps->textFormat = 0;
	pbps->cchText = (INT)SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0L);

	if (!IsWindowEnabled(hwnd)) pbps->windowStyle |= WS_DISABLED;
	if (extendedColors.updateRequired) UpdateExtendedColorTable();

	if (BS_OWNERDRAW == (BS_TYPEMASK & pbps->windowStyle)) pbps->windowStyle = ((pbps->windowStyle & ~BS_OWNERDRAW) | BS_PUSHBUTTON); 

	INT type;
	if (BS_PUSHLIKE & pbps->windowStyle) type = BS_PUSHBUTTON;
	else type = (BS_TYPEMASK & pbps->windowStyle);

	switch(type)
	{
		case BS_PUSHBUTTON:
		case BS_DEFPUSHBUTTON:
			if (SWBS_TOOLBAR & pbps->skinnedStyle)
			{
				BOOL bChecked = (BS_PUSHLIKE & pbps->windowStyle) ? (BST_CHECKED == SendMessageW(hwnd, BM_GETCHECK, 0, 0L)) : FALSE;
				if ((BST_PUSHED & pbps->buttonState) || hoverHWND == hwnd || bChecked) pbps->windowStyle |= WS_BORDER;

				if (BST_PUSHED & pbps->buttonState) 
					pbps->rgbTextBk = extendedColors.rgbToolBkPressed;
				else if (hoverHWND  == hwnd || bChecked) pbps->rgbTextBk = extendedColors.rgbToolBkHigh;
				else pbps->rgbTextBk = WADlg_getColor(WADLG_WNDBG);

				pbps->rgbText =  WADlg_getColor(WADLG_WNDFG);
			}
			else
			{
				pbps->rgbTextBk = (BST_PUSHED & pbps->buttonState) ? extendedColors.rgbTextBkDown : extendedColors.rgbTextBk;
				pbps->rgbText =  (WS_DISABLED & pbps->windowStyle) ? extendedColors.rgbTextDisabled : WADlg_getColor(WADLG_BUTTONFG);
			}
			break;
		default:
			pbps->rgbTextBk = WADlg_getColor(WADLG_WNDBG);
			pbps->rgbText = (WS_DISABLED & pbps->windowStyle) ? extendedColors.rgbTextDisabled2 : WADlg_getColor(WADLG_WNDFG);
			break;
	}

	if (pbps->cchText)
	{	
		if (pbps->cchText > BUTTON_TEXT_MAX) pbps->cchText = BUTTON_TEXT_MAX;
		SendMessageW(hwnd, WM_GETTEXT, (WPARAM)BUTTON_TEXT_MAX, (LPARAM)pbps->szText);

		pbps->hFont = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
		if (NULL == pbps->hFont) pbps->hFont = (HFONT)MlStockObjects_Get(DEFAULT_FONT);

		if ( 0 == (BS_MULTILINE & pbps->windowStyle)) pbps->textFormat |= DT_SINGLELINE;
		if (0x02/*UISF_HIDEACCEL*/ & uiState) pbps->textFormat |= 0x00100000 /*DT_HIDEPREFIX*/;
	}

	MLBUTTONIMAGELIST buttonIL;
	if(MLSkinnedButton_GetImageList(hwnd, &buttonIL) && buttonIL.hmlil) 
	{	
		pbps->imageIndex = -1;
		if (WS_DISABLED & pbps->windowStyle) pbps->imageIndex = buttonIL.disabledIndex;
		else if (BST_PUSHED & pbps->buttonState) pbps->imageIndex = buttonIL.pressedIndex;

		if (-1 == pbps->imageIndex)
			pbps->imageIndex = ((SWBS_BUTTONHOVER & pbps->skinnedStyle) && -1 != buttonIL.hoverIndex) ? buttonIL.hoverIndex : buttonIL.normalIndex;

		if (-1 != pbps->imageIndex) pbps->hImage = (HBITMAP)buttonIL.hmlil;
		else pbps->hImage = NULL;
	}
	else
	{
		pbps->hImage = (HBITMAP)SendMessageW(hwnd, BM_GETIMAGE, IMAGE_BITMAP, 0L);
		pbps->imageIndex = -1;
	}

	if (pbps->cchText || pbps->hImage)
	{
		switch((BS_CENTER & pbps->windowStyle))
		{
			case BS_LEFT:	pbps->textFormat |= DT_LEFT; break;
			case BS_RIGHT:	pbps->textFormat |= DT_RIGHT; break;
			case BS_CENTER: pbps->textFormat |= DT_CENTER; break;
			case 0:
				switch(BS_TYPEMASK & pbps->windowStyle)
				{
					case BS_PUSHBUTTON:
					case BS_DEFPUSHBUTTON:	pbps->textFormat |= DT_CENTER; break;
					default:				pbps->textFormat |= DT_LEFT; break;
				}
				break;
		}	

		switch((BS_VCENTER & pbps->windowStyle))
		{
			case BS_TOP:	pbps->textFormat |= DT_TOP; break;
			case BS_BOTTOM:	pbps->textFormat |= DT_BOTTOM; break;
			case 0:
			case BS_VCENTER: pbps->textFormat |= DT_VCENTER; break;
		}
	}
}

void SkinnedButton::OnPaint()
{
	PAINTSTRUCT ps;
	BUTTONPAINTSTRUCT bps;

	PrePaint(hwnd, &bps, style, uiState);
	bps.hdc = BeginPaint(hwnd, &ps);
	if (NULL == bps.hdc) return;
	CopyRect(&bps.rcPaint, &ps.rcPaint);
	DrawButton(&bps);
	EndPaint(hwnd, &ps);
}

void SkinnedButton::OnPrintClient(HDC hdc, UINT options)
{
	if ((PRF_CLIENT & options) && (0 == (PRF_CHECKVISIBLE & options) || IsWindowVisible(hwnd))) 
	{
		BUTTONPAINTSTRUCT bps;

		PrePaint(hwnd, &bps, style, uiState);
		bps.hdc = hdc;
		CopyRect(&bps.rcPaint, &bps.rcClient);
		DrawButton(&bps);
	}
}

void SkinnedButton::Emulate_LeftButtonDown(UINT flags, POINTS pts, BOOL forwardMessage)
{
	if(SWBS_SPLITBUTTON & style)
	{
		POINT pt;
		RECT rc;
		UINT os = (style & ~SWBS_SPLITSETMANUAL);
		POINTSTOPOINT(pt, pts);
		GetClientRect(hwnd, &rc);
		rc.left = rc.right - SPLITAREA_WIDTH;

		style &= ~(SWBS_SPLITPRESSED | SWBS_SPLITSETMANUAL);
		if (PtInRect(&rc, pt))
		{
			style |= SWBS_SPLITPRESSED;
			if (0 == (SWBS_TOOLBAR & style) && hwnd != GetFocus()) SetFocus(hwnd);
		}
		if (style != os) InvalidateRect(hwnd, NULL, FALSE);
		if (SWBS_SPLITPRESSED & style) 
		{
			return;
		}
	}
	else if (SWBS_DROPDOWNBUTTON & style)
	{
		UINT os = (style & ~SWBS_SPLITSETMANUAL);

		style &= ~SWBS_SPLITSETMANUAL;
		style |= SWBS_SPLITPRESSED;

		if ( 0 == (SWBS_TOOLBAR & style) && hwnd != GetFocus()) SetFocus(hwnd);

		if (style != os) InvalidateRect(hwnd, NULL, FALSE);
		return;
	}

	if (SWBS_TOOLBAR & style)
	{
		if (lButtonDownHWND != hwnd)
		{
			lButtonDownHWND = hwnd;
			SendMessageW(hwnd, BM_SETSTATE, TRUE, 0L);
			InvalidateRect(hwnd, NULL, FALSE);
			UpdateWindow(hwnd);
		}
		return;
	}

	UINT s = (UINT)SendMessageW(hwnd, BM_GETSTATE, 0, 0L);
	if (BST_PUSHED & s) return;

	if (FALSE != forwardMessage)
	{
		DisableRedraw();
		CallPrevWndProc(WM_LBUTTONDOWN, (WPARAM)flags, *((LONG*)&pts));
		EnableRedraw(SWR_INVALIDATE | SWR_UPDATE);
	}
}

void SkinnedButton::Emulate_LeftButtonUp(UINT flags, POINTS pts, BOOL forwardMessage)
{
	if ((SWBS_DROPDOWNBUTTON | SWBS_SPLITBUTTON) & style)
	{
		if (SWBS_SPLITPRESSED & style)
		{					
			SendMessageW(GetParent(hwnd), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwnd), MLBN_DROPDOWN), (LPARAM)hwnd); 
			if (0 == (SWBS_SPLITSETMANUAL &style))
			{
				style &= ~SWBS_SPLITPRESSED;
				InvalidateRect(hwnd, NULL, FALSE);
				return;
			}
		}
	}

	if (FALSE != forwardMessage)
	{
		__super::CallPrevWndProc(WM_LBUTTONUP, (WPARAM)flags, *((LONG*)&pts));
	}

	if (lButtonDownHWND)
	{
		lButtonDownHWND = FALSE;
		HWND hParent = GetParent(hwnd);
		SendMessageW(hwnd, BM_SETSTATE, FALSE, 0L);

		DWORD ws = GetWindowLongPtrW(hwnd, GWL_STYLE);
		INT type = (BS_TYPEMASK & ws); 
		switch(type)
		{
			case BS_AUTO3STATE:
			case BS_AUTORADIOBUTTON:
			case BS_AUTOCHECKBOX:
				{					
					INT state = (INT)SendMessageW(hwnd, BM_GETCHECK, 0, 0L);
					if (BS_AUTORADIOBUTTON == type && BST_CHECKED == state) break;
					switch(state)
					{
						case BST_CHECKED:
							state = (BS_AUTO3STATE == type) ? BST_INDETERMINATE : BST_UNCHECKED;
							break;
						case BST_INDETERMINATE:
						case BST_UNCHECKED:
							state = BST_CHECKED;
							break;
					}
					if (BS_AUTORADIOBUTTON == type)
					{
						HWND h = hwnd;
						wchar_t szClass[32] = {0};
						DWORD lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
						while (NULL != (h = GetWindow(h, GW_HWNDPREV)))
						{							
							if (!GetClassNameW(h, szClass, sizeof(szClass)/sizeof(szClass[0])) ||
								CSTR_EQUAL != CompareStringW(lcid, NORM_IGNORECASE, WC_BUTTONW, -1, szClass, -1)) break;
							SendMessageW(h, BM_SETCHECK, BST_UNCHECKED, 0L);
							if (WS_GROUP & GetWindowLongPtrW(h, GWL_STYLE)) break;
						}
						SendMessageW(hwnd, BM_SETCHECK, state, 0L);

						h = hwnd;
						while (NULL != (h = GetWindow(h, GW_HWNDNEXT)))
						{							
							if (!GetClassNameW(h, szClass, sizeof(szClass)/sizeof(szClass[0])) ||
								CSTR_EQUAL != CompareStringW(lcid, NORM_IGNORECASE, WC_BUTTONW, -1, szClass, -1)) break;
							SendMessageW(h, BM_SETCHECK, BST_UNCHECKED, 0L);
							if (WS_GROUP & GetWindowLongPtrW(h, GWL_STYLE)) break;
						}
					}
					else SendMessageW(hwnd, BM_SETCHECK, state, 0L);
				}
				break;
		}

		if (hParent)
			SendMessageW(hParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), BN_CLICKED), (LPARAM)hwnd);
	}
	UpdateWindow(hwnd);
}

void SkinnedButton::OnLButtonDown(UINT flags, POINTS pts)
{
	Emulate_LeftButtonDown(flags, pts, TRUE);
}

void SkinnedButton::OnLButtonUp(UINT flags, POINTS pts)
{
	Emulate_LeftButtonUp(flags, pts, TRUE);
}

void SkinnedButton::OnMouseMove(UINT flags, POINTS pts)
{
	BOOL bInvalidate(FALSE);
	if (hwnd != hoverHWND && (!lButtonDownHWND || lButtonDownHWND == hwnd) && (0 == (SWBS_TOOLBAR & style) || IsChild(GetActiveWindow(), hwnd)))
	{
		if (NULL != hoverHWND && hoverHWND != hwnd) SendMessageW(hoverHWND, WM_MOUSELEAVE, 0, 0L);

		hoverHWND = hwnd;
		if (lButtonDownHWND == hwnd) SendMessageW(hwnd, BM_SETSTATE, TRUE, 0L);
		bInvalidate = TRUE;
		hoverSplit = (SWBS_DROPDOWNBUTTON & style);
		TRACKMOUSEEVENT track;
		track.cbSize = sizeof(TRACKMOUSEEVENT);
		track.dwFlags = TME_LEAVE;
		track.hwndTrack = hwnd;
		track.dwHoverTime = HOVER_DEFAULT;
		_TrackMouseEvent(&track);
	}

	if (SWBS_SPLITBUTTON == ((SWBS_SPLITPRESSED | SWBS_SPLITBUTTON) & style))
	{
		BOOL bSplit;
		POINT pt;
		RECT rc;
		POINTSTOPOINT(pt, pts);
		GetClientRect(hwnd, &rc);
		rc.left = rc.right - SPLITAREA_WIDTH;

		bSplit = PtInRect(&rc, pt);
		if (hoverSplit != bSplit)
		{
			hoverSplit = bSplit;
			bInvalidate = TRUE;
		}
	}

	__super::CallPrevWndProc(WM_MOUSEMOVE, (WPARAM)flags, *((LONG*)&pts));

	if (bInvalidate) 
	{
		InvalidateRect(hwnd, NULL, FALSE);
		UpdateWindow(hwnd);
	}
}

LRESULT SkinnedButton::GetIdealSize(LPCWSTR pszText)
{
	INT cchText;
	SIZE szButton;
	szButton.cx = 0;
	szButton.cy = 0;

	cchText = (pszText) ? lstrlenW(pszText) : (INT)SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0L);

	{
		HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
		if (hdc)
		{
			wchar_t szText[BUTTON_TEXT_MAX] = {0};
			if (NULL == pszText) 
			{
				SendMessageW(hwnd, WM_GETTEXT, (WPARAM)BUTTON_TEXT_MAX, (LPARAM)szText);
				pszText = szText;
			}

			HFONT hFont = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
			if (NULL == hFont) hFont = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
			HFONT hfo = (NULL != hFont) ? (HFONT)SelectObject(hdc, hFont) : NULL;

			if (0 != cchText)
			{
				RECT rt;
				SetRect(&rt, 0, 0, 0, 0);
				if (FALSE == DrawTextW(hdc, pszText, cchText, &rt, DT_CALCRECT | DT_SINGLELINE))
				{
					szButton.cx = 0;
					szButton.cy = 0;
				}
				else
				{
					szButton.cx = rt.right - rt.left;
					szButton.cy = rt.bottom - rt.top;
				}
			}
			else
			{
				TEXTMETRIC metrics;

				szButton.cx = 0;
				if (FALSE == GetTextMetrics(hdc, &metrics))
					szButton.cy = 0;
				else 
					szButton.cy = metrics.tmHeight;
			}

			if (0 != szButton.cy)
				szButton.cy += (MARGIN_TOP + MARGIN_BOTTOM);

			if (0 != szButton.cx)
				szButton.cx += (MARGIN_LEFT + MARGIN_RIGHT) + 2;

			if (NULL != hfo) 
				SelectObject(hdc, hfo);

			ReleaseDC(hwnd, hdc);
		}
	}

	if (SWBS_DROPDOWNBUTTON & style) szButton.cx += (SPLITAREA_WIDTH + SPLITAREA_SPACE);
	if (SWBS_SPLITBUTTON & style) szButton.cx += (SPLITAREA_WIDTH + SPLITAREA_SPACE_ALT);

	MLBUTTONIMAGELIST buttonIL;
	if(MLSkinnedButton_GetImageList(hwnd, &buttonIL) && buttonIL.hmlil) 
	{
		INT imageCX, imageCY;
		if (MLImageListI_GetImageSize(buttonIL.hmlil, &imageCX, &imageCY))
		{
			imageCY += (MARGIN_TOP + MARGIN_BOTTOM);
			if (szButton.cy < imageCY) szButton.cy = imageCY;
			szButton.cx += imageCX;
			if (szButton.cx != imageCX) szButton.cx += IMAGE_SPACE;
			else szButton.cx += (MARGIN_LEFT + MARGIN_RIGHT);
		}
	}

	return MAKELPARAM(szButton.cx, szButton.cy);
}

BOOL SkinnedButton::OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult)
{
	switch(msg)
	{
		case ML_IPC_SKINNEDBUTTON_SETIMAGELIST:
			ZeroMemory(&imagelist, sizeof(MLBUTTONIMAGELIST));
			if (param) CopyMemory(&imagelist, (void*)param, sizeof(MLBUTTONIMAGELIST));
			*pResult = TRUE;
			InvalidateRect(hwnd, NULL, FALSE);
			return TRUE;
		case ML_IPC_SKINNEDBUTTON_GETIMAGELIST:
			if (!param) *pResult = FALSE;
			else
			{
				CopyMemory((void*)param, &imagelist, sizeof(MLBUTTONIMAGELIST));
				*pResult = TRUE;
			}
			return TRUE;

		case ML_IPC_SKINNEDBUTTON_SETDROPDOWNSTATE:
			{
				UINT os = style;
				style &= ~SWBS_SPLITPRESSED;
				if (param) style |= SWBS_SPLITPRESSED;
				style |= SWBS_SPLITSETMANUAL;
				if (style != os) InvalidateRect(hwnd, NULL, FALSE);
			}
			return TRUE;
		case ML_IPC_SKINNEDBUTTON_GETIDEALSIZE:
			*pResult = GetIdealSize((LPCWSTR)param);
			return TRUE;
	}
	return __super::OnMediaLibraryIPC(msg, param, pResult);
}

LRESULT SkinnedButton::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (SWS_USESKINCOLORS & style)
	{
		switch(uMsg)
		{
			case WM_PAINT: OnPaint(); return 0;
			case WM_ERASEBKGND: return 0;
			case WM_PRINTCLIENT: OnPrintClient((HDC)wParam, (UINT)lParam); return 0;

			case WM_SETFOCUS: 
			case WM_KILLFOCUS: 
			case 0x0128/*WM_UPDATEUISTATE*/:
			case WM_SETTEXT:
			case WM_ENABLE:
			{
				DisableRedraw();
				LRESULT r = __super::WindowProc(uMsg, wParam, lParam);
				EnableRedraw(SWR_INVALIDATE | SWR_UPDATE);
				return r;
			}
			case BM_SETSTATE:
				if (GetUpdateRect(hwnd, NULL, FALSE)) UpdateWindow(hwnd);
				{
					LRESULT os = SendMessageW(hwnd, BM_GETSTATE, 0, 0L);
					DisableRedraw();
					__super::WindowProc(uMsg, wParam, lParam);
					EnableRedraw(SWR_NONE);
					if (os != SendMessageW(hwnd, BM_GETSTATE, 0, 0L))
					{
						InvalidateRect(hwnd, NULL, FALSE);
						UpdateWindow(hwnd);
					}
				}
				break;

			case BM_SETCHECK:
				if (GetUpdateRect(hwnd, NULL, FALSE)) UpdateWindow(hwnd);
				{
					LRESULT os = SendMessageW(hwnd, BM_GETCHECK, 0, 0L);
					DisableRedraw();
					__super::WindowProc(uMsg, wParam, lParam);
					EnableRedraw(SWR_NONE);
					if (os != SendMessageW(hwnd, BM_GETCHECK, 0, 0L))
					{
						InvalidateRect(hwnd, NULL, FALSE);
						UpdateWindow(hwnd);
					}
				}
				break;
			case 0x0127/*WM_CHANGEUISTATE*/:
			{
				if (GetUpdateRect(hwnd, NULL, FALSE)) UpdateWindow(hwnd);
				DisableRedraw();
				LRESULT r = __super::WindowProc(uMsg, wParam, lParam);
				EnableRedraw(SWR_NONE);
				return r;
			}
			case WM_GETDLGCODE:
			{
				LRESULT r = __super::WindowProc(uMsg, wParam, lParam);
				if ((SWBS_SPLITBUTTON | SWBS_DROPDOWNBUTTON) & style) 
					r |= DLGC_WANTARROWS;
				return r;
			}
			case WM_KEYDOWN:
				switch(wParam)
				{
					case VK_LEFT:
					case VK_RIGHT:
						if ((SWBS_SPLITBUTTON | SWBS_DROPDOWNBUTTON) & style)		
						{
							HWND hwndParent = GetParent(hwnd);
							if (hwndParent) SendMessageW(hwndParent, WM_NEXTDLGCTL, (VK_LEFT == wParam), 0L);
							SendMessageW(hwnd, 0x127/*WM_CHANGEUISTATE*/, MAKELONG(2/*UIS_CLEAR*/, 1/*UISF_HIDEFOCUS*/), 0L);
							return 0;
						}
						break;
					case VK_DOWN:
					case VK_UP:
					case VK_SPACE:
						if (((SWBS_SPLITBUTTON | SWBS_DROPDOWNBUTTON) & style) && 
							1 == LOWORD(lParam) && 0 == (0x40000000 & lParam))
						{
							RECT rc;
							GetClientRect(hwnd, &rc);
							if (VK_SPACE != wParam) rc.left = rc.right - SPLITAREA_WIDTH;
							OffsetRect(&rc, 1, 1);
							LONG pts =  POINTTOPOINTS(*(POINT*)&rc);
							Emulate_LeftButtonDown(0, MAKEPOINTS(pts), FALSE); 
							SendMessageW(hwnd, 0x127/*WM_CHANGEUISTATE*/, MAKELONG(2/*UIS_CLEAR*/, 1/*UISF_HIDEFOCUS*/), 0L);
							return 0;
						}
						break;
				}
				break;
			case WM_KEYUP:
				switch(wParam)
				{
					case VK_DOWN:
					case VK_UP:
					case VK_SPACE:
						if (((SWBS_SPLITBUTTON | SWBS_DROPDOWNBUTTON) & style) && 
							1 == LOWORD(lParam))
						{
							RECT rc;
							GetClientRect(hwnd, &rc);
							if (VK_SPACE != wParam) rc.left = rc.right - SPLITAREA_WIDTH;
							OffsetRect(&rc, 1, 1);
							LONG pts =  POINTTOPOINTS(*(POINT*)&rc);
							Emulate_LeftButtonUp(0, MAKEPOINTS(pts), FALSE); 
							return 0;
						}
						break;
				}
				break;
			case WM_LBUTTONDBLCLK:
			case WM_LBUTTONDOWN:	OnLButtonDown((UINT)wParam, MAKEPOINTS(lParam)); return 0;
			case WM_MOUSEMOVE:		OnMouseMove((UINT)wParam, MAKEPOINTS(lParam)); return 0;
			case WM_LBUTTONUP:		OnLButtonUp((UINT)wParam, MAKEPOINTS(lParam)); return 0;
			case WM_MOUSELEAVE:
				if (hwnd == hoverHWND || 
					(SWBS_SPLITPRESSED | SWBS_SPLITBUTTON) == ((SWBS_SPLITPRESSED | SWBS_SPLITBUTTON | SWBS_SPLITSETMANUAL) & style))
				{
					hoverHWND = NULL;
					hoverSplit = FALSE;
					if (0 == (SWBS_SPLITSETMANUAL &style)) style &= ~SWBS_SPLITPRESSED;
					InvalidateRect(hwnd, NULL, FALSE);
					UpdateWindow(hwnd);
				}
				if (lButtonDownHWND == hwnd) SendMessageW(hwnd, BM_SETSTATE, FALSE, 0L);
				
				break;
		}
	}

	return __super::WindowProc(uMsg, wParam, lParam);
}

static BOOL CALLBACK ImageFilter(LPBYTE pData, LONG cx, LONG cy, INT bpp, COLORREF rgbBk, COLORREF rgbFg, INT_PTR imageTag, LPARAM lParam)
{
	LONG pitch, x, dibPitch;
	LPBYTE dib, cursor, line;
	HDC hdc, hdcSrc;
	HBITMAP hbmp, hbmpOld;
	IMAGEFILTERPARAM *param;

	param = &imagefilterParam;
	if (32 != bpp || !param) 
		return FALSE;
	pitch = cx*4;

	hdcSrc = param->pbps->hdc;
	rgbFg = param->pbps->rgbText;
	rgbBk = param->pbps->rgbTextBk;

	BITMAPINFOHEADER bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.biSize		= sizeof(bi);
	bi.biWidth		= param->pbps->rcClient.right - param->pbps->rcClient.left;
	bi.biHeight		= -(param->pbps->rcClient.bottom - param->pbps->rcClient.top);
	bi.biPlanes		= 1;
	bi.biBitCount	= 32;

	hdc = CreateCompatibleDC(NULL);
	if (NULL == hdc) 
		return FALSE;

	IntersectClipRect(hdc, param->prcImage->left, param->prcImage->top, 
						param->prcImage->right, param->prcImage->bottom);

	hbmp = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (VOID**)&dib, NULL, NULL);
	if (NULL == hbmp) 
	{
		DeleteDC(hdc);
		return FALSE;
	}

	hbmpOld = (HBITMAP)SelectObject(hdc, hbmp);
	param->pbps->hdc = hdc;

	RECT rcPaint;
	CopyRect(&rcPaint, &param->pbps->rcPaint);
	CopyRect(&param->pbps->rcPaint, param->prcImage);

	PushButton_DrawBackground(param->pbps);

	param->pbps->hdc = hdcSrc;
	CopyRect(&param->pbps->rcPaint, &rcPaint);

	SelectObject(hdc, hbmpOld);
	DeleteDC(hdc);

	dibPitch = 4*(bi.biWidth);
	dib += 4*(param->prcImage->left) + param->prcImage->top*dibPitch;
	cy = param->prcImage->bottom - param->prcImage->top;
	for (line = pData; cy-- != 0; line += pitch, dib += dibPitch )
	{
		for (x = cx, cursor = line; x-- != 0; cursor += 4) 
		{
			if (IMAGEFILTER_BLENDPLUSCOLOR == lParam)
			{
				BYTE luma;
				luma = 255 - (BYTE)((cursor[2]*30 + cursor[1]*59 + cursor[0]*11)/100); 
				cursor[0] = GetBValue(rgbFg) - ((GetBValue(rgbFg) - GetBValue(rgbBk))*luma>>8);
				cursor[1] = GetGValue(rgbFg) - ((GetGValue(rgbFg) - GetGValue(rgbBk))*luma>>8);
				cursor[2] = GetRValue(rgbFg) - ((GetRValue(rgbFg) - GetRValue(rgbBk))*luma>>8);
			}
			if (0x00 == cursor[3]) 
			{
				cursor[0] = dib[0];
				cursor[1] = dib[1];
				cursor[2] = dib[2];
				cursor[3] = 0xFF;
			}
			else if (cursor[3] != 0xFF)
			{
				cursor[0] = (cursor[0]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*dib[0] + 127)/255;
				cursor[1] = (cursor[1]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*dib[1] + 127)/255;
				cursor[2] = (cursor[2]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*dib[2] + 127)/255;
				cursor[3] = 0xFF;
			}
		}
	}

	DeleteObject(hbmp);  ///Last call
	return TRUE;
}

BOOL SkinnedButton::RegisterImageFilter(HANDLE filterMananger)
{
	BOOL result;
	MLIMAGEFILTERINFO_I mlif;
	ZeroMemory(&mlif, sizeof(MLIMAGEFILTERINFO_I));

	mlif.mask		= MLIFF_TITLE_I | MLIFF_FLAGS_I | MLIFF_PROC_I | MLIFF_PARAM_I;
	mlif.uid			= MLIF_BUTTONBLEND_UID;	
	mlif.fnProc		= ImageFilter;
	mlif.pszTitle	= L"Button image filter (blend)";
	mlif.lParam		= IMAGEFILTER_BLEND;
	mlif.fFlags		= 0;
	result = MLImageFilterI_Register((HMLIMGFLTRMNGR)filterMananger, &mlif);

	mlif.mask		= MLIFF_TITLE_I | MLIFF_FLAGS_I | MLIFF_PROC_I | MLIFF_PARAM_I;
	mlif.uid			= MLIF_BUTTONBLENDPLUSCOLOR_UID;	
	mlif.fnProc		= ImageFilter;
	mlif.pszTitle	= L"Button image filter (blend plus color)";
	mlif.lParam		= IMAGEFILTER_BLENDPLUSCOLOR;
	mlif.fFlags		= 0;
	if (!MLImageFilterI_Register((HMLIMGFLTRMNGR)filterMananger, &mlif)) result = FALSE;
	return result;
}