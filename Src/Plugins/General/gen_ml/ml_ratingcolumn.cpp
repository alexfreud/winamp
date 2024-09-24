#include "main.h"
#include "./ml_ratingcolumn.h"
#include "./ml_rating.h"
#include "api__gen_ml.h"
#include "./ml.h"
#include "./ml_IPC_0313.h"
#include "./resource.h"
#include "../winamp/gen.h"
#include "./stockobjects.h"
#include <commctrl.h>
#include <strsafe.h>

extern HMLIMGLST hmlilRating;
extern UINT ratingGlobalStyle;

#define RATING_IMAGELIST		hmlilRating
#define RATING_IMAGEINDEX		0
#define RATING_MAXVALUE			5

#define RATING_LEFTPADDING		5
#define RATING_RIGHTPADDING		2

#define RATING_AUTOUNHOVERDELAY		80
#define RATING_ANIMATIONINTERVAL	200
#define RAITNG_ANIMATIONMAX			1000

#define RATING_DRAGFORGIVENESS_LEFT		0xFFFF	
#define RATING_DRAGFORGIVENESS_RIGHT	0xFFFF
#define RATING_DRAGFORGIVENESS_TOP		12
#define RATING_DRAGFORGIVENESS_BOTTOM	12

#define RATING_FILLCHAR					L'x'

typedef struct _RATINGTRACKING
{
	HWND        hwndList;
	UINT        iItem;
	UINT        iSubItem;
	RECT        rc;			// trackin cell 
	INT         value;
	UINT_PTR    timerId;
} RATINGTRACKING;

typedef struct _RATINGANIMATION
{
	HWND        hwndList;
	UINT        iItem;
	UINT_PTR    timerId;
	UINT        durationMs;
	UINT        startedMs;
	WORD        stage;
} RATINGANIMATION;

typedef struct _RATINGDRAG
{
	HWND    hwndList;
	UINT    iItem;
	UINT    iSubItem;
	RECT    rc;
	INT     value;
	BOOL    update;
	BOOL    outside;
	UINT    fStyle;
} RATINGDRAG;

typedef struct _TWEAKPARAM
{
    ONRATINGTWEAKAPLLY fnApply;
    UINT fStyle;
} TWEAKPARAM;

static INT  ratingMinWidth		= 65;
static INT	ratingCharWidth		= 8;

static RATINGTRACKING ratingTracking;
static RATINGANIMATION ratingAnimation;
static RATINGDRAG ratingDrag;

static void CALLBACK Timer_AutoUnhover(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
static void CALLBACK Timer_Animation(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
static void CorrectDragPoint(CONST RECT *prc, POINT *ppt);
static BOOL IsTrakingAllowed(HWND hwndList, UINT fStyle);
static BOOL IsItemTrackable(HWND hwndList, UINT iItem, UINT fStyle);
static INT_PTR WINAPI TweakDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL MLRatingColumnI_Initialize(void)
{
	RECT rc;

	ZeroMemory(&ratingTracking, sizeof(RATINGTRACKING));
	ratingTracking.iItem = (UINT)-1;

	ZeroMemory(&ratingAnimation, sizeof(RATINGANIMATION));
	ratingAnimation.iItem = (UINT)-1;

	ratingMinWidth = ((MLRatingI_CalcMinRect(RATING_MAXVALUE, RATING_IMAGELIST, &rc)) ? (rc.right - rc.left) : 0);

	return TRUE;
}

BOOL MLRatingColumnI_Update(void)
{
	HDC hdc;
	HFONT hFontOld, hFont;
	INT length;
	WCHAR test[6], *p;

	hdc = (HDC)MlStockObjects_Get(CACHED_DC);
	if (!hdc) return FALSE;

	hFont = (HFONT)MlStockObjects_Get(SKIN_FONT);

	length = sizeof(test)/sizeof(WCHAR);
	p = test + length;
	while (p-- != test) *p =  RATING_FILLCHAR;
	hFontOld = (HFONT)SelectObject(hdc, hFont);

	SIZE sz;

	if (GetTextExtentPoint32W(hdc, test, length, &sz)) ratingCharWidth = sz.cx/length;
	else ratingCharWidth = 8;

	SelectObject(hdc, hFontOld);
	return TRUE;
}

INT MLRatingColumnI_GetMinWidth(void)
{
	return ratingMinWidth + RATING_LEFTPADDING + RATING_RIGHTPADDING;
}

LPCWSTR MLRatingColumnI_FillBackString(LPWSTR pszText, INT cchTextMax, INT nColumnWidth, UINT fStyle)
{
	INT width, count;
	LPWSTR p;
	if (!pszText) return NULL;
	pszText[0] = 0x00;

	fStyle = (RCS_DEFAULT == fStyle) ? ratingGlobalStyle : fStyle;
	
	width = MLRatingColumnI_GetMinWidth();
	if (nColumnWidth < width) return pszText;

	if (RCS_ALLIGN_RIGHT_I & fStyle) width = nColumnWidth;
	else if (RCS_ALLIGN_CENTER_I & fStyle) width += (nColumnWidth - width)/2;
	if (width <= 0) return pszText;
	count = (width - 4)/ratingCharWidth;
	if (count >= cchTextMax) count = cchTextMax -1;
	for (p = pszText; count--; p++) *p = RATING_FILLCHAR;
	*p = 0x00;
	return pszText;
}

BOOL MLRatingColumnI_Paint(RATINGCOLUMNPAINT_I *pRCPaint)
{
	RECT rc;
	UINT fStyle, fRCStyle;

	fRCStyle = (RCS_DEFAULT == pRCPaint->fStyle) ? ratingGlobalStyle : pRCPaint->fStyle;

	rc.left = LVIR_BOUNDS;
	rc.top = pRCPaint->iSubItem;
	if (SendMessageW(pRCPaint->hwndList, LVM_GETSUBITEMRECT, pRCPaint->iItem, (LPARAM)&rc))
	{
		if ((rc.right - rc.left - RATING_LEFTPADDING - RATING_RIGHTPADDING) >= ratingMinWidth && 
			(rc.left + RATING_LEFTPADDING) < (rc.right - RATING_RIGHTPADDING) && rc.top < rc.bottom)
		{
			INT left;
			COLORREF rgbBkOld;
			BOOL tracking;

			if (rc.right <= pRCPaint->prcView->left || rc.left >= pRCPaint->prcView->right) return TRUE;

			tracking = (pRCPaint->hwndList == ratingTracking.hwndList && pRCPaint->iItem == ratingTracking.iItem);

			rgbBkOld = SetBkColor(pRCPaint->hdc, pRCPaint->rgbBk);

			left = rc.left;
			if (0 == rc.left) rc.left = 3;
			ExtTextOutW(pRCPaint->hdc, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);

			fStyle = RDS_VCENTER_I; 
			if (RCS_ALLIGN_RIGHT_I & fRCStyle) fStyle |= RDS_RIGHT_I;
			else if (RCS_ALLIGN_CENTER_I & fRCStyle) fStyle |= RDS_HCENTER_I;

			INT value = pRCPaint->value, trackingValue;

			if (tracking) 
			{
				fStyle |= RDS_HOT_I;
				if(RCS_SHOWEMPTY_HOT_I & fRCStyle) fStyle |= RDS_SHOWEMPTY_I;
                trackingValue = ratingTracking.value;
			}
			else if (pRCPaint->hwndList == ratingDrag.hwndList && pRCPaint->iItem == ratingDrag.iItem && !ratingDrag.outside)
			{
				value = 0;
				trackingValue = 0;
				fStyle |= (RDS_SHOWEMPTY_I | RDS_HOT_I);
			}
			else 
			{
				if(RCS_SHOWEMPTY_NORMAL_I & fRCStyle) fStyle |= RDS_SHOWEMPTY_I;
				trackingValue = 0;
			}

			if(RCS_SHOWINACTIVE_HOT_I & fRCStyle) fStyle |= RDS_INACTIVE_HOT_I;

			if (pRCPaint->hwndList == ratingAnimation.hwndList && pRCPaint->iItem == ratingAnimation.iItem)
			{
				if (RCS_SHOWEMPTY_ANIMATION_I & fRCStyle) fStyle |= RDS_SHOWEMPTY_I;
				switch(ratingAnimation.stage)
				{
					case 1:
						rc.top -= 1;
						rc.bottom -= 1;
						break;
				}
			}

			if (value || (RDS_SHOWEMPTY_I & fStyle))
			{
				COLORREF rgbFgOld = SetTextColor(pRCPaint->hdc, pRCPaint->rgbFg);
				rc.left = left + RATING_LEFTPADDING;
				rc.right -= RATING_RIGHTPADDING;
				MLRatingI_Draw(pRCPaint->hdc, RATING_MAXVALUE, value, trackingValue, RATING_IMAGELIST, RATING_IMAGEINDEX, &rc, fStyle);
				if (pRCPaint->rgbFg != rgbFgOld) SetTextColor(pRCPaint->hdc, rgbFgOld);
			}
			if (pRCPaint->rgbBk != rgbBkOld) SetBkColor(pRCPaint->hdc, rgbBkOld);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL MLRatingColumnI_Click(RATINGCOLUMN_I *pRating)	
{
	UINT fRCStyle;
	LVHITTESTINFO lvhit;

	fRCStyle = (RCS_DEFAULT == pRating->fStyle) ? ratingGlobalStyle : pRating->fStyle;

	lvhit.pt = pRating->ptAction;
	SendMessageW(pRating->hwndList, LVM_SUBITEMHITTEST, 0, (LPARAM)&lvhit);
	pRating->iItem = lvhit.iItem;
	
	if (ratingTracking.hwndList && ratingTracking.hwndList != pRating->hwndList || 
		!IsTrakingAllowed(pRating->hwndList, fRCStyle)) MLRatingColumnI_CancelTracking(TRUE);

	if (-1 != ratingTracking.iItem && ratingTracking.iItem != (UINT)lvhit.iItem)
	{
		if (-1 == lvhit.iItem) MLRatingColumnI_CancelTracking(TRUE);
		else 
		{
			ratingTracking.iItem = (UINT)lvhit.iItem;
			ratingTracking.iSubItem = lvhit.iSubItem;
		}
	}
	
	if (-1 != lvhit.iItem && (0 == (RCS_BLOCKCLICK_I & fRCStyle)))
	{
		RECT rc;
		UINT fStyle;
		rc.left = LVIR_BOUNDS;	
		rc.top	= lvhit.iSubItem;
		fStyle	= RDS_VCENTER_I;

		if (RCS_ALLIGN_RIGHT_I & fRCStyle) fStyle |= RDS_RIGHT_I;
		if (RCS_ALLIGN_CENTER_I & fRCStyle) fStyle |= RDS_HCENTER_I;

		if (SendMessageW(pRating->hwndList, LVM_GETSUBITEMRECT, lvhit.iItem, (LPARAM)&rc))
		{
			if (0 == lvhit.iSubItem)
			{
				HWND hwndHeader;
				RECT rh;
				hwndHeader = (HWND)SendMessageW(pRating->hwndList, LVM_GETHEADER, 0, 0L);
				if (hwndHeader && SendMessageW(hwndHeader, HDM_GETITEMRECT, lvhit.iSubItem, (LPARAM)&rh)) 
				{
					rc.left = rh.left;
					rc.right = rh.right;
				}
				else SetRect(&rc, 0, 0, 0, 0);
			}

			rc.left += RATING_LEFTPADDING;
			rc.right -= RATING_RIGHTPADDING;

			if ((rc.right - rc.left) >= ratingMinWidth)
			{
				pRating->value = MLRatingI_HitTest(pRating->ptAction, RATING_MAXVALUE, RATING_IMAGELIST, &rc, fStyle);
				pRating->value = LOWORD(pRating->value);
				if (0 == pRating->value && (RCS_BLOCKUNRATECLICK_I & fRCStyle)) return FALSE;
				if (ratingTracking.iItem == (UINT)lvhit.iItem && ratingTracking.value != pRating->value)
				{
					ratingTracking.value = pRating->value;
					SendMessageW(pRating->hwndList, LVM_REDRAWITEMS, lvhit.iItem, lvhit.iItem);
					if (pRating->bRedrawNow) UpdateWindow(pRating->hwndList);
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

void MLRatingColumnI_Track(RATINGCOLUMN_I *pRating)
{
	UINT fRCStyle;
	BOOL trackingOk;

	fRCStyle = (RCS_DEFAULT == pRating->fStyle) ? ratingGlobalStyle : pRating->fStyle;

	if (ratingTracking.hwndList != pRating->hwndList) MLRatingColumnI_CancelTracking(pRating->bRedrawNow);
	trackingOk = (IsTrakingAllowed(pRating->hwndList, fRCStyle) && IsItemTrackable(pRating->hwndList, pRating->iItem, fRCStyle));
			
	if (ratingTracking.iItem != pRating->iItem || ratingTracking.iSubItem != pRating->iSubItem || (-1 != ratingTracking.iItem && !trackingOk))
	{
		if (-1 != ratingTracking.iItem) MLRatingColumnI_CancelTracking(FALSE);
		if (trackingOk)
		{
			ratingTracking.rc.left = LVIR_BOUNDS;
			ratingTracking.rc.top = pRating->iSubItem;

			if (SendMessageW(pRating->hwndList, LVM_GETSUBITEMRECT, pRating->iItem, (LPARAM)&ratingTracking.rc))
			{
				if (0 == pRating->iSubItem)
				{
					HWND hwndHeader;
					RECT rh;

					hwndHeader = (HWND)SendMessageW(pRating->hwndList, LVM_GETHEADER, 0, 0L);
					if (hwndHeader && SendMessageW(hwndHeader, HDM_GETITEMRECT, pRating->iSubItem, (LPARAM)&rh)) 
					{
						ratingTracking.rc.left = rh.left;
						ratingTracking.rc.right = rh.right;
					}
					else SetRect(&ratingTracking.rc, 0, 0, 0, 0);
				}
				ratingTracking.rc.left += RATING_LEFTPADDING;
				ratingTracking.rc.right -= RATING_RIGHTPADDING;
				if ((ratingTracking.rc.right - ratingTracking.rc.left) < ratingMinWidth)
				{
					SetRect(&ratingTracking.rc, 0, 0, 0, 0);
				}
			}
			else SetRect(&ratingTracking.rc, 0, 0, 0, 0);

			if (ratingTracking.rc.left != ratingTracking.rc.right)
			{
				ratingTracking.hwndList = pRating->hwndList;
				ratingTracking.iItem		= pRating->iItem;
				ratingTracking.iSubItem = pRating->iSubItem;
				ratingTracking.timerId = SetTimer(NULL, NULL, RATING_AUTOUNHOVERDELAY, Timer_AutoUnhover);
			}
			else 
			{
				ratingTracking.hwndList = NULL;
				ratingTracking.iItem		= -1;
				ratingTracking.iSubItem = 0;
			}
		}
	}

	if (-1 != ratingTracking.iItem)
	{
		UINT fStyle;
		INT value;

		fStyle	= RDS_VCENTER_I;
		if (RCS_ALLIGN_RIGHT_I & fRCStyle) fStyle |= RDS_RIGHT_I;
		if (RCS_ALLIGN_CENTER_I & fRCStyle) fStyle |= RDS_HCENTER_I;

		value = LOWORD(MLRatingI_HitTest(pRating->ptAction, RATING_MAXVALUE, RATING_IMAGELIST, &ratingTracking.rc, fStyle));

		if (ratingTracking.value != value)
		{
			ratingTracking.value = value;
			SendMessageW(pRating->hwndList, LVM_REDRAWITEMS, ratingTracking.iItem, ratingTracking.iItem);
			if (pRating->bRedrawNow) UpdateWindow(pRating->hwndList);
		}
	}
}

BOOL MLRatingColumnI_BeginDrag(RATINGCOLUMN_I *pRating)
{
	POINT pt;

	if (ratingDrag.hwndList) 
	{		
		RATINGCOLUMN_I rcol;
		rcol.bCanceled = TRUE;
		rcol.bRedrawNow = TRUE;
		MLRatingColumnI_EndDrag(&rcol);
	}

	ratingDrag.fStyle = (RCS_DEFAULT == pRating->fStyle) ? ratingGlobalStyle : pRating->fStyle;
	ratingDrag.rc.left = LVIR_BOUNDS;	
	ratingDrag.rc.top	= pRating->iSubItem;
	ratingDrag.fStyle = ratingDrag.fStyle & 
						~(RCS_TRACKITEM_SELECTED_I | RCS_TRACKITEM_FOCUSED_I | RCS_BLOCKCLICK_I | RCS_BLOCKUNRATECLICK_I) | 
						(RCS_TRACKITEM_ALL_I | RCS_TRACK_ALWAYS_I);

	if ((RCS_BLOCKDRAG_I & ratingDrag.fStyle ) || !pRating->hwndList || !IsWindow(pRating->hwndList) || 
		(UINT)-1 == pRating->iItem || (UINT)-1 == pRating->iSubItem ||
		!SendMessageW(pRating->hwndList, LVM_GETSUBITEMRECT, pRating->iItem, (LPARAM)&ratingDrag.rc)) return FALSE;

	if (0 == pRating->iSubItem)
	{
		HWND hwndHeader;
		RECT rh;
		hwndHeader = (HWND)SendMessageW(pRating->hwndList, LVM_GETHEADER, 0, 0L);
		if (!hwndHeader || !SendMessageW(hwndHeader, HDM_GETITEMRECT, pRating->iSubItem, (LPARAM)&rh))  return FALSE;
		ratingDrag.rc.left = rh.left;
		ratingDrag.rc.right = rh.right;
	}
	ratingDrag.rc.left += RATING_LEFTPADDING;
	ratingDrag.rc.right -= RATING_RIGHTPADDING;

	if ((ratingDrag.rc.right - ratingDrag.rc.left) < ratingMinWidth)
	{
		SetRect(&ratingDrag.rc, 0, 0, 0, 0);
		return FALSE;
	}

	if (RCS_ALLIGN_RIGHT_I & ratingDrag.fStyle) ratingDrag.rc.left = ratingDrag.rc.right - ratingMinWidth;
	else 
	{
		if (RCS_ALLIGN_CENTER_I & ratingDrag.fStyle) ratingDrag.rc.left += ((ratingDrag.rc.right - ratingDrag.rc.left) - ratingMinWidth)/2;
		ratingDrag.rc.right = ratingDrag.rc.left + ratingMinWidth;
	}

	GetCursorPos(&pt);
	MapWindowPoints(HWND_DESKTOP, pRating->hwndList, &pt, 1);
	if (ratingDrag.rc.left <= pt.x && pt.x <= ratingDrag.rc.right)
	{
		ratingDrag.hwndList	= pRating->hwndList;
		ratingDrag.iItem		= pRating->iItem;
		ratingDrag.iSubItem	= pRating->iSubItem;
		ratingDrag.value	= pRating->value;
		ratingDrag.update	= TRUE;
		ratingDrag.outside	= TRUE;
		return TRUE;
	}
	return FALSE;
}

BOOL MLRatingColumnI_Drag(POINT pt)
{
	if (ratingDrag.hwndList && ratingDrag.hwndList == WindowFromPoint(pt))
	{
		SendMessageW(ratingDrag.hwndList, WM_SETCURSOR, (WPARAM)ratingDrag.hwndList, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
		ratingDrag.outside = FALSE;

		MapWindowPoints(HWND_DESKTOP, ratingDrag.hwndList, &pt, 1);
		CorrectDragPoint(&ratingDrag.rc, &pt);

		if (PtInRect(&ratingDrag.rc, pt))
		{
			RATINGCOLUMN_I rcol;

			ratingDrag.update = TRUE;
			rcol.hwndList	= ratingDrag.hwndList;
			rcol.iItem		= ratingDrag.iItem;
			rcol.iSubItem	= ratingDrag.iSubItem;
			rcol.value		= ratingDrag.value;
			rcol.ptAction	= pt;
			rcol.bRedrawNow = FALSE;
			rcol.fStyle		= ratingDrag.fStyle;
			MLRatingColumnI_Track(&rcol);
            KillTimer(NULL, ratingTracking.timerId);

			if (-1 != ratingTracking.iItem && ratingTracking.value) 
			{
				UpdateWindow(ratingDrag.hwndList);
				return TRUE;
			}
		}

		if (ratingDrag.update)
		{
			MLRatingColumnI_CancelTracking(FALSE);
			SendMessageW(ratingDrag.hwndList, LVM_REDRAWITEMS, ratingDrag.iItem, ratingDrag.iItem);
			UpdateWindow(ratingDrag.hwndList);
			ratingDrag.update	= FALSE;
		}
		return TRUE;
	}
	
	if (ratingTracking.hwndList) 
	{ 
		MLRatingColumnI_CancelTracking(FALSE); 
		ratingDrag.update = FALSE; 
	}
	
	if (!ratingDrag.update)
	{
		ratingDrag.outside = TRUE;
		if (NULL != ratingDrag.hwndList)
		{
			SendMessageW(ratingDrag.hwndList, LVM_REDRAWITEMS, ratingDrag.iItem, ratingDrag.iItem);
			UpdateWindow(ratingDrag.hwndList);
		}
		ratingDrag.update = TRUE;
	}
	return FALSE;
}

BOOL MLRatingColumnI_EndDrag(RATINGCOLUMN_I *pRating)
{
	BOOL result;
	RATINGCOLUMN_I rcol;
	RECT rc;

	rcol.hwndList	= ratingDrag.hwndList;
	rcol.iItem		= ratingDrag.iItem;
	rcol.iSubItem	= ratingDrag.iSubItem;
	rcol.fStyle		= ratingDrag.fStyle;
	rcol.bRedrawNow	= FALSE;
	CopyRect(&rc, &ratingDrag.rc);

	ZeroMemory(&ratingDrag, sizeof(RATINGDRAG));
	ratingDrag.iItem = (UINT)-1;

	result = FALSE;

	if (rcol.hwndList) SendMessageW(rcol.hwndList, LVM_REDRAWITEMS, rcol.iItem, rcol.iItem);

	if (rcol.hwndList && rcol.hwndList == WindowFromPoint(pRating->ptAction))
	{
		if (!pRating->bCanceled)
		{
			rcol.ptAction = pRating->ptAction;
			MapWindowPoints(HWND_DESKTOP, rcol.hwndList, &rcol.ptAction, 1);
			CorrectDragPoint(&rc, &rcol.ptAction);

			pRating->value = (PtInRect(&rc, rcol.ptAction) && MLRatingColumnI_Click(&rcol)) ? rcol.value : 0;
			pRating->hwndList = rcol.hwndList;
			pRating->iItem = rcol.iItem;
			pRating->iSubItem = rcol.iSubItem;
			result = TRUE;
		}
	}
	if (pRating->bRedrawNow && NULL != rcol.hwndList) 
		UpdateWindow(rcol.hwndList);

	return result;
}

void MLRatingColumnI_Animate(HWND hwndList, UINT iItem, UINT durationMs)
{
	if (ratingAnimation.timerId)
	{
		HWND ratingList;
		KillTimer(NULL, ratingAnimation.timerId);
		ratingList = ratingAnimation.hwndList;
		ratingAnimation.hwndList = NULL;
		if (ratingList && IsWindow(ratingList)) 
		{						
			SendMessageW(ratingList, LVM_REDRAWITEMS, ratingAnimation.iItem, ratingAnimation.iItem);
			UpdateWindow(ratingList);
		}
	}

	if ((UINT)-1 != iItem && IsWindow(hwndList))
	{
		ratingAnimation.hwndList = hwndList;
		ratingAnimation.durationMs = (durationMs > RAITNG_ANIMATIONMAX) ? RAITNG_ANIMATIONMAX : durationMs;
		ratingAnimation.stage = 1;
		ratingAnimation.iItem = iItem;
		ratingAnimation.startedMs = GetTickCount();

		ratingAnimation.timerId = SetTimer(NULL, NULL, RATING_ANIMATIONINTERVAL, Timer_Animation);
	}
	else ratingAnimation.timerId = 0;

	if (ratingAnimation.timerId)
	{
		SendMessageW(ratingAnimation.hwndList, LVM_REDRAWITEMS, ratingAnimation.iItem, ratingAnimation.iItem);
		UpdateWindow(ratingAnimation.hwndList);
	}
	else
	{
		ZeroMemory(&ratingAnimation, sizeof(RATINGANIMATION));
		ratingAnimation.iItem = (UINT)-1;
	}
}

void MLRatingColumnI_CancelTracking(BOOL bRedrawNow)
{
	HWND hwndList;
	UINT iItem;

	if (ratingTracking.timerId)
	{
		KillTimer(NULL, ratingTracking.timerId);
		ratingTracking.timerId	= 0;
	}

	if (ratingTracking.hwndList && IsWindow(ratingTracking.hwndList) && -1 != ratingTracking.iItem)
	{
		hwndList = ratingTracking.hwndList;
		iItem = ratingTracking.iItem;
	}
	else 
	{
		hwndList = NULL;
		iItem = (UINT)-1;
	}

	ratingTracking.hwndList	= NULL;
	ratingTracking.iItem		= (UINT)-1;
	ratingTracking.iSubItem	= (UINT)-1;
	ratingTracking.value	= -1;

	if(hwndList) 
	{
		SendMessageW(hwndList, LVM_REDRAWITEMS, (WPARAM)iItem, (LPARAM)iItem);
		if(bRedrawNow) UpdateWindow(hwndList);
	}
}

INT MLRatingColumnI_GetWidth(INT width, UINT fStyle)
{
	INT minWidth;
	if (RCS_DEFAULT == fStyle) fStyle = ratingGlobalStyle;
	minWidth = MLRatingColumnI_GetMinWidth();
	if (width < minWidth && 0 == (RCS_SIZE_ALLOWDECREASE_I & fStyle)) width = minWidth;
	if (width > minWidth && 0 == (RCS_SIZE_ALLOWINCREASE_I & fStyle)) width = minWidth;
	return width;
}

HWND MLRatingColumnI_TweakDialog(HWND hwndParent, UINT fStyle, ONRATINGTWEAKAPLLY fnApply, BOOL bVisible)
{
	HWND hwndDlg;
	TWEAKPARAM param;

	if (!hwndParent || !IsWindow(hwndParent) || !fnApply) return NULL;

	param.fnApply = fnApply;
	param.fStyle = fStyle;
	hwndDlg = WASABI_API_CREATEDIALOGPARAMW(IDD_RATINGTWEAK, hwndParent, TweakDialogProc, (LPARAM)&param);
	if (IsWindow(hwndDlg))
	{
		RECT rw, rc;
		GetWindowRect((IsWindowVisible(hwndParent)?hwndParent:prefsWnd), &rc);
		GetWindowRect(hwndDlg, &rw);
		rc.left += ((rc.right - rc.left) - (rw.right - rw.left))/2;
		rc.top +=  ((rc.bottom - rc.top) - (rw.bottom - rw.top))/2;
		SetWindowPos(hwndDlg, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | ((bVisible) ? SWP_SHOWWINDOW : SWP_NOACTIVATE));
	}
	return hwndDlg;
}

static void CorrectDragPoint(CONST RECT *prc, POINT *ppt)
{
	if (ppt->x <= prc->left && (prc->left - ppt->x) < RATING_DRAGFORGIVENESS_LEFT)		ppt->x = prc->left + 1;
	if (ppt->x >= prc->right && (ppt->x - prc->right) < RATING_DRAGFORGIVENESS_RIGHT)	ppt->x = prc->right - 1;
	if (ppt->y <= prc->top && (prc->top - ppt->y) < RATING_DRAGFORGIVENESS_TOP)			ppt->y = prc->top + 1;
	if (ppt->y >= prc->bottom && (ppt->y - prc->bottom) < RATING_DRAGFORGIVENESS_BOTTOM)	ppt->y = prc->bottom -1;
}

static BOOL IsTrakingAllowed(HWND hwndList, UINT fStyle)
{
	if (RCS_TRACK_ALWAYS_I & fStyle) return TRUE;
	if (RCS_TRACK_WNDFOCUSED_I & fStyle) 
	{
		return (hwndList == GetFocus());
	}
	if (RCS_TRACK_ANCESTORACITVE_I & fStyle) 
	{
		HWND hwndActive;
		hwndActive = GetActiveWindow();
		return (hwndList == hwndActive || IsChild(hwndActive, hwndList));
	}
	if (RCS_TRACK_PROCESSACTIVE_I & fStyle)
	{
		GUITHREADINFO gui;
		gui.cbSize = sizeof(GUITHREADINFO);
		return (!GetGUIThreadInfo(GetWindowThreadProcessId(hwndList, NULL), &gui) || gui.hwndActive);
	}
	return FALSE;
}

static BOOL IsItemTrackable(HWND hwndList, UINT iItem, UINT fStyle)
{
	if (RCS_TRACKITEM_SELECTED_I & fStyle) return (LVIS_SELECTED == (LVIS_SELECTED & SendMessageW(hwndList, LVM_GETITEMSTATE, (WPARAM)iItem, (LPARAM)LVIS_SELECTED)));
	if (RCS_TRACKITEM_FOCUSED_I & fStyle) return (LVIS_FOCUSED == (LVIS_FOCUSED & SendMessageW(hwndList, LVM_GETITEMSTATE, (WPARAM)iItem, (LPARAM)LVIS_FOCUSED)));
	return TRUE;
}

static void CALLBACK Timer_AutoUnhover(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (-1 == ratingTracking.iItem || NULL == ratingTracking.hwndList) KillTimer(NULL, idEvent);
	else
	{
		POINT pt;
		GetCursorPos(&pt);
		MapWindowPoints(HWND_DESKTOP, ratingTracking.hwndList, &pt, 1);
			
		if (pt.x > (ratingTracking.rc.right + RATING_RIGHTPADDING) || pt.x < (ratingTracking.rc.left - RATING_LEFTPADDING) ||
			pt.y > ratingTracking.rc.bottom || pt.y < ratingTracking.rc.top)
		{
			MLRatingColumnI_CancelTracking(FALSE);
		}
	}
}

static void CALLBACK Timer_Animation(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (-1 == ratingAnimation.iItem || !ratingAnimation.hwndList) KillTimer(NULL, idEvent);
	else
	{
		UINT iItem;
		HWND hwndList;
		
		iItem = ratingAnimation.iItem;
		hwndList = ratingAnimation.hwndList;

		if ((GetTickCount() - ratingAnimation.startedMs) >  ratingAnimation.durationMs)
		{
			KillTimer(NULL, idEvent);
			ZeroMemory(&ratingAnimation, sizeof(RATINGANIMATION));
			ratingAnimation.iItem = (UINT)-1;
		}
		else if (++ratingAnimation.stage > 1) ratingAnimation.stage = 0; 
		
		SendMessageW(hwndList, LVM_REDRAWITEMS, iItem, iItem);
		UpdateWindow(hwndList);
	}
}

static void TweakDialog_ApplyStyle(HWND hwndDlg, TWEAKPARAM *pTweak, UINT newStyle, BOOL bClosing)
{
	if (!pTweak->fnApply(newStyle, bClosing))
	{
		wchar_t title[32] = {0};
		MessageBoxW(hwndDlg, WASABI_API_LNGSTRINGW(IDS_UNABLE_TO_APPLY_NEW_STYLE),
				    WASABI_API_LNGSTRINGW_BUF(IDS_TWEAK_ERROR,title,32), MB_OK);
		newStyle = pTweak->fStyle;
	}
	else pTweak->fStyle = newStyle;
}

static void TweakDialog_InitializeControls(HWND hwndDlg, UINT fStyle)
{
	INT i;
	HWND hwndCtrl;

	if (NULL != (hwndCtrl = GetDlgItem(hwndDlg, IDC_CMB_TRACKWHEN)))
	{
		int pszTrackWhen[] = { IDS_ALWAYS, IDS_PROCESS_ACTIVE,  IDS_ANCESTOR_ACTIVE, IDS_WINDOW_FOCUSED, IDS_NEVER};
		for (i = 0; i <  sizeof(pszTrackWhen)/sizeof(char*); i++) SendMessageW(hwndCtrl, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(pszTrackWhen[i]));
		if (RCS_TRACK_ALWAYS_I & fStyle)	 i = 0;
		else if (RCS_TRACK_PROCESSACTIVE_I & fStyle) i = 1;
		else if (RCS_TRACK_ANCESTORACITVE_I & fStyle) i = 2;
		else if (RCS_TRACK_WNDFOCUSED_I & fStyle) i = 3;
		else i = 4;
		SendMessage(hwndCtrl, CB_SETCURSEL, (WPARAM)i, 0L);
	}

	if (NULL != (hwndCtrl = GetDlgItem(hwndDlg, IDC_CMB_TRACKWHAT)))
	{
		int pszTrackWhat[] = { IDS_ALL, IDS_SELECTED, IDS_FOCUSED,};
		for (i = 0; i <  sizeof(pszTrackWhat)/sizeof(char*); i++) SendMessageW(hwndCtrl, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(pszTrackWhat[i]));
		if (RCS_TRACKITEM_SELECTED_I & fStyle) i = 1;
		else if (RCS_TRACKITEM_FOCUSED_I & fStyle) i = 2;
		else i = 0;
		SendMessage(hwndCtrl, CB_SETCURSEL, (WPARAM)i, 0L);
	}

	if (NULL != (hwndCtrl = GetDlgItem(hwndDlg, IDC_CMB_ALIGNMENT)))
	{
		int pszAlignment[] = { IDS_LEFT, IDS_CENTER, IDS_RIGHT,};
		for (i = 0; i <  sizeof(pszAlignment)/sizeof(char*); i++) SendMessageW(hwndCtrl, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(pszAlignment[i]));
		if (RCS_ALLIGN_CENTER_I & fStyle) i = 1;
		else if (RCS_ALLIGN_RIGHT_I & fStyle) i = 2;
		else i = 0;
		SendMessage(hwndCtrl, CB_SETCURSEL, (WPARAM)i, 0L);
	}

	CheckDlgButton(hwndDlg, IDC_CHK_SHOWEMPTY_NORMAL, (RCS_SHOWEMPTY_NORMAL_I & fStyle) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_SHOWEMPTY_HOT, (RCS_SHOWEMPTY_HOT_I & fStyle) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_SHOWEMPTY_ANIMATION, (RCS_SHOWEMPTY_ANIMATION_I & fStyle) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_SHOWINACTIVE_HOT, (RCS_SHOWINACTIVE_HOT_I & fStyle) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_BLOCKUNRATE, (RCS_BLOCKUNRATECLICK_I & fStyle) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_BLOCKCLICK, (RCS_BLOCKCLICK_I & fStyle) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_BLOCKDRAG, (RCS_BLOCKDRAG_I & fStyle) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_SIZEDECREASE, (RCS_SIZE_ALLOWDECREASE_I & fStyle) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_SIZEINCREASE, (RCS_SIZE_ALLOWINCREASE_I & fStyle) ? BST_CHECKED : BST_UNCHECKED);
}

static UINT TweakDialog_GetStyle(HWND hwndDlg)
{
	INT i;
	HWND hwndCtrl;
	UINT fStyle;
	fStyle = 0;

	if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_SHOWEMPTY_NORMAL)) fStyle |= RCS_SHOWEMPTY_NORMAL_I;
	if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_SHOWEMPTY_HOT)) fStyle |= RCS_SHOWEMPTY_HOT_I;
	if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_SHOWEMPTY_ANIMATION)) fStyle |= RCS_SHOWEMPTY_ANIMATION_I;
	if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_SHOWINACTIVE_HOT)) fStyle |= RCS_SHOWINACTIVE_HOT_I;
	if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_BLOCKUNRATE)) fStyle |= RCS_BLOCKUNRATECLICK_I;
	if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_BLOCKCLICK)) fStyle |= RCS_BLOCKCLICK_I;
	if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_BLOCKDRAG)) fStyle |= RCS_BLOCKDRAG_I;
	if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_SIZEDECREASE)) fStyle |= RCS_SIZE_ALLOWDECREASE_I;
	if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CHK_SIZEINCREASE)) fStyle |= RCS_SIZE_ALLOWINCREASE_I;

	i = (NULL != (hwndCtrl = GetDlgItem(hwndDlg, IDC_CMB_TRACKWHEN))) ? (INT)SendMessage(hwndCtrl, CB_GETCURSEL, 0, 0L) : CB_ERR;
	switch(i)
	{
		case 0: fStyle |= RCS_TRACK_ALWAYS_I; break;
		case 1: fStyle |= RCS_TRACK_PROCESSACTIVE_I; break;
		case 2: fStyle |= RCS_TRACK_ANCESTORACITVE_I; break;
		case 3: fStyle |= RCS_TRACK_WNDFOCUSED_I; break;
	}
	i = (NULL != (hwndCtrl = GetDlgItem(hwndDlg, IDC_CMB_TRACKWHAT))) ? (INT)SendMessage(hwndCtrl, CB_GETCURSEL, 0,0L) : CB_ERR;
	switch(i)
	{
		case 1: fStyle |= RCS_TRACKITEM_SELECTED_I; break;
		case 2: fStyle |= RCS_TRACKITEM_FOCUSED_I; break;
	}
	i = (NULL != (hwndCtrl = GetDlgItem(hwndDlg, IDC_CMB_ALIGNMENT))) ? (INT)SendMessage(hwndCtrl, CB_GETCURSEL, 0,0L) : CB_ERR;
	switch(i)
	{
		case 1: fStyle |= RCS_ALLIGN_CENTER_I; break;
		case 2: fStyle |= RCS_ALLIGN_RIGHT_I; break;
	}

	return fStyle;
}

static INT_PTR WINAPI TweakDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static TWEAKPARAM tweak;
	static UINT	fStyle;
	switch(uMsg)
	{
		case WM_INITDIALOG:

			tweak.fnApply = (lParam) ? ((TWEAKPARAM*)lParam)->fnApply : NULL;
			tweak.fStyle = (lParam) ? ((TWEAKPARAM*)lParam)->fStyle : 0x000;
			if (RCS_DEFAULT == tweak.fStyle) tweak.fStyle = ratingGlobalStyle;
			fStyle = tweak.fStyle;
			TweakDialog_InitializeControls(hwnd, fStyle);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDCANCEL:
				{
					wchar_t title[32] = {0};
					fStyle = TweakDialog_GetStyle(hwnd);
					TweakDialog_ApplyStyle(hwnd, &tweak, 
											(tweak.fStyle != fStyle && 
											IDYES == MessageBoxW(hwnd, WASABI_API_LNGSTRINGW(IDS_DO_YOU_WANT_TO_SAVE_CHANGES_FIRST),
											WASABI_API_LNGSTRINGW_BUF(IDS_CONFIRM_CLOSE,title,32), MB_YESNO)) ?
											fStyle :tweak.fStyle, TRUE);
					DestroyWindow(hwnd);
					break;
				}
				case IDOK:
					if (BN_CLICKED == HIWORD(wParam)) 
					{
						fStyle = TweakDialog_GetStyle(hwnd);
						TweakDialog_ApplyStyle(hwnd, &tweak, fStyle, FALSE);
					}
					DestroyWindow(hwnd);
					break;
			}
	}
	return 0;
}