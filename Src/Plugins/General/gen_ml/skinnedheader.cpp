#include "main.h"
#include "config.h"
#include "../winamp/wa_dlg.h"
#include "./skinnedheader.h"
#include "./skinning.h"
#include "./ml_imagelist.h"
#include "./imagefilters.h"
#include "./ml_cloudcolumn.h"
#include "./resource.h"

// Timers
#define TIMER_UNBLOCKREDRAW_ID		1978
#define TIMER_ENDTRACK_ID			1979
#define TIMER_ENDTRACK_DELAY		5

// Header internal flags (LOWORD reserved to sort index)
#define HIF_ASCENDING		0x00010000
#define HIF_OWNPARENTSW		0x00020000	// parent was subclassed by this control
#define HIF_REALCXY			0x00040000
#define HIF_DEFSIZERULE		0x00100000  // item will use default size rule.
#define HIF_BLOCKCHANGING	0x10000000
#define HIF_BLOCKCHANGED	0x20000000
#define HIF_BLOCKWNDPOS		0x40000000
#define HIF_BLOCKREDRAW		0x80000000

extern HMLIMGFLTRMNGR hmlifMngr;	// default gen_ml fitler manager

typedef struct _HDSIZE
{
	INT instanceRef;
	INT *pCXY;		// stored by index
	INT *pOrder;	// current left-to-right order of items (index values for items in the header).
	INT count;
	INT allocated;
	INT headerCXY;
	INT	lastCXY;
	HWND hwndParent;
	HDITEMW hdi;		// safe to use duaring size
} HDSIZE;

typedef struct _HDCURSOR
{
	HWND				hwndParent;
	NMMOUSE			nm;
	HDHITTESTINFO	hitTest;
	DWORD			pts;
} HDCURSOR;

static HDSIZE hdrSize = {0,0,0, } ;

static INT GetDefaultSizeRule(void)
{
	INT rule;
	rule = g_config->ReadInt(L"column_resize_mode", 0);
	switch(rule)
	{
		case 1:	return SHS_SIZERULE_ADJUSTONE;
		case 2:	return SHS_SIZERULE_PROPORTIONAL;
		case 3:	return SHS_SIZERULE_ADJUSTALL;
	}
	return SHS_SIZERULE_WINDOWS;
}

SkinnedHeader::SkinnedHeader(void) : SkinnedWnd(FALSE), hcurNormal(NULL), cloudColumn(-1)
{
	if (!hdrSize.instanceRef)
		ZeroMemory(&hdrSize, sizeof(HDSIZE));

	hdrSize.instanceRef++;

	hdrFlags = LOWORD(-1) | HIF_ASCENDING | HIF_DEFSIZERULE;
	hdrSizeRule = (HIF_DEFSIZERULE & hdrFlags) ? GetDefaultSizeRule() : SHS_SIZERULE_WINDOWS;
}

SkinnedHeader::~SkinnedHeader(void)
{
	if (hdrSize.instanceRef && !--hdrSize.instanceRef)
	{
		if (hdrSize.pCXY) 
			free(hdrSize.pCXY); // pOrder - use  same buffer
		ZeroMemory(&hdrSize, sizeof(HDSIZE));
	}
}

extern HMLIMGLST hmlilCloud;
BOOL SkinnedHeader::Attach(HWND hwndHeader)
{
	HWND hwndParent;

	if(!__super::Attach(hwndHeader)) return FALSE;

	SetType(SKINNEDWND_TYPE_HEADER);

	hwndParent = GetParent(hwndHeader);
	if (hwndParent) SkinWindow(hwndParent, SWS_NORMAL); 

	hdrFlags = LOWORD(-1) | HIF_ASCENDING | HIF_DEFSIZERULE;
	hdrSizeRule = (HIF_DEFSIZERULE & hdrFlags) ? GetDefaultSizeRule() : SHS_SIZERULE_WINDOWS;

	// set default height
	SetHeight(-1);

	return TRUE;
}

void SkinnedHeader::OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw)
{
	hcurNormal = (HCURSOR)SendMessageW(plugin.hwndParent, WM_WA_IPC, WACURSOR_NORMAL, IPC_GETSKINCURSORS);
	__super::OnSkinChanged(bNotifyChildren, bRedraw);
}

void SkinnedHeader::BlockRedraw(BOOL bBlock, UINT unblockDelay)
{
	KillTimer(hwnd, TIMER_UNBLOCKREDRAW_ID);

	if (FALSE != bBlock)
		hdrFlags |= HIF_BLOCKREDRAW;
	else
		hdrFlags &= ~HIF_BLOCKREDRAW;

	CallDefWndProc(WM_SETREDRAW, (WPARAM)!bBlock, 0L);

	if (FALSE != bBlock && -1 != unblockDelay) 
		SetTimer(hwnd, TIMER_UNBLOCKREDRAW_ID, unblockDelay, NULL);
}

DWORD SkinnedHeader::GetSortArrowSize(void) 
{
	return MAKELPARAM(7, 8);
}

BOOL SkinnedHeader::DrawSortArrow(HDC hdc, RECT *prc, COLORREF rgbBk, COLORREF rgbFg, BOOL bAscending)
{		
	static HMLIMGLST hmlilSort = NULL; 
	IMAGELISTDRAWPARAMS ildp;

	if (!hmlilSort)
	{
		DWORD arrowSize = GetSortArrowSize();
		hmlilSort = MLImageListI_Create(GET_X_LPARAM(arrowSize)*2, GET_Y_LPARAM(arrowSize), MLILC_COLOR24_I, 1, 1, 1, hmlifMngr);
		if (hmlilSort) 
		{
			MLIMAGESOURCE_I mlis;
			ZeroMemory(&mlis, sizeof(MLIMAGESOURCE_I));
			mlis.type		= SRC_TYPE_PNG_I;
			mlis.hInst		= plugin.hDllInstance;
			mlis.lpszName	= MAKEINTRESOURCEW(IDB_SORTARROW);
		    MLImageListI_Add(hmlilSort, &mlis, MLIF_FILTER1_UID, 0);
		}
	}

	ZeroMemory(&ildp, sizeof(IMAGELISTDRAWPARAMS));
	ildp.cbSize	= 56; // keep it hardcoded for now - otherwise brakes win2000. sizeof(IMAGELISTDRAWPARAMS);

	ildp.i = MLImageListI_GetRealIndex(hmlilSort, 0, rgbBk, rgbFg);
	if (-1 != ildp.i)
	{
		ildp.hdcDst = hdc;
		ildp.himl = MLImageListI_GetRealList(hmlilSort);

		ildp.cx			= prc->right - prc->left;
		ildp.cy			= prc->bottom - prc->top;
		ildp.x			= prc->left;
		ildp.y			= prc->top;
		ildp.fStyle		= ILD_NORMAL;
		ildp.dwRop		= SRCCOPY;
		ildp.xBitmap	= (bAscending) ? ildp.cx : 0;
		return ImageList_DrawIndirect(&ildp);
	}

	return FALSE;
}

BOOL SkinnedHeader::DrawCloudIcon(HDC hdc, RECT *prc, COLORREF rgbBk, COLORREF rgbFg)
{
	IMAGELISTDRAWPARAMS ildp;

	ZeroMemory(&ildp, sizeof(IMAGELISTDRAWPARAMS));
	ildp.cbSize	= 56; // keep it hardcoded for now - otherwise brakes win2000. sizeof(IMAGELISTDRAWPARAMS);

	ildp.i = MLImageListI_GetRealIndex(hmlilCloud, 0, rgbBk, rgbFg);
	if (-1 != ildp.i)
	{
		ildp.hdcDst = hdc;
		ildp.himl = MLImageListI_GetRealList(hmlilCloud);

		ildp.cx			= 16;
		ildp.cy			= 16;
		ildp.x			= prc->left - 7;
		ildp.y			= prc->top - 4;
		ildp.fStyle		= ILD_NORMAL;
		ildp.dwRop		= SRCCOPY;
		ildp.xBitmap	= 0;
		return ImageList_DrawIndirect(&ildp);
	}
	return FALSE;
}

void SkinnedHeader::DrawHeaderItem(LPNMCUSTOMDRAW pnmcd)
{
	HPEN pen, penOld;
	HDC hdc;
	RECT *prc, rcText;	
	INT textRight;
	INT ox, oy;

	hdc = pnmcd->hdc;
	prc = &pnmcd->rc;

	ox = (CDIS_SELECTED & pnmcd->uItemState) ? 1 : 0;
	oy = 0;

	SetBkColor(hdc, WADlg_getColor(WADLG_LISTHEADER_BGCOLOR));
	ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, prc, L"", 0, 0);

	pen = (HPEN)MlStockObjects_Get((CDIS_SELECTED & pnmcd->uItemState) ? HEADERBOTTOM_PEN : HEADERTOP_PEN);
	penOld = (HPEN)SelectObject(hdc, pen);

	MoveToEx(hdc, prc->left, prc->top, NULL);
	LineTo(hdc, prc->right, prc->top);
	MoveToEx(hdc, prc->left, prc->top, NULL);
	LineTo(hdc, prc->left, prc->bottom);

	if (0 == (CDIS_SELECTED & pnmcd->uItemState))
	{
		SelectObject(hdc, penOld);

		pen = (HPEN)MlStockObjects_Get(HEADERBOTTOM_PEN);
		penOld = (HPEN)SelectObject(hdc, pen);
	}

	MoveToEx(hdc, prc->right - 1, prc->top, NULL);
	LineTo(hdc, prc->right - 1, prc->bottom);
	MoveToEx(hdc, prc->right - 1, prc->bottom - 1, NULL);
	LineTo(hdc, prc->left - 1, prc->bottom - 1);

	if (0 == (CDIS_SELECTED & pnmcd->uItemState))
	{
		SelectObject(hdc, penOld);

		pen = (HPEN)MlStockObjects_Get(HEADERMIDDLE_PEN);
		penOld = (HPEN)SelectObject(hdc, pen);

		MoveToEx(hdc, prc->right - 2, prc->top + 1, NULL);
		LineTo(hdc, prc->right - 2, prc->bottom - 2);
		MoveToEx(hdc, prc->right - 2, prc->bottom - 2, NULL);
		LineTo(hdc, prc->left, prc->bottom - 2);
	}

	textRight = prc->right - 5 + ox;
	if (LOWORD(hdrFlags) == pnmcd->dwItemSpec || pnmcd->dwItemSpec == cloudColumn)
	{
		RECT r;
		DWORD arrowSize;
		int dividerArea;

		arrowSize = GetSortArrowSize();
		SetRect(&r, 0, 0, GET_X_LPARAM(arrowSize), GET_Y_LPARAM(arrowSize));

		dividerArea = 4*GetSystemMetrics(SM_CXEDGE);

		OffsetRect(&r, prc->right - r.right - dividerArea, (((prc->bottom - prc->top) - r.bottom) + 1)/2);

		if (pnmcd->dwItemSpec != cloudColumn && (r.left > (prc->left + 16) && r.top > (prc->top + 2)))
		{
			if (DrawSortArrow(hdc, &r,
							  WADlg_getColor(WADLG_LISTHEADER_BGCOLOR),
							  WADlg_getColor(WADLG_LISTHEADER_FONTCOLOR),
							  (HIF_ASCENDING & hdrFlags))) textRight = r.left - 3;
		}
		else if (pnmcd->dwItemSpec == cloudColumn)
		{
			if ((prc->right - prc->left) >= MLCloudColumnI_GetMinWidth())
			{
				DrawCloudIcon(hdc, &r,
							  WADlg_getColor(WADLG_LISTHEADER_BGCOLOR),
							  WADlg_getColor(WADLG_LISTHEADER_FONTCOLOR));
			}
		}
	}

	if (pnmcd->dwItemSpec != cloudColumn && (textRight > (prc->left + 4 + ox)))
	{
		WCHAR buffer[128] = {0};
		HDITEMW item;

		item.mask = HDI_TEXT | HDI_FORMAT;
		item.pszText = buffer;
		item.cchTextMax = sizeof(buffer)/sizeof(wchar_t) -1;

		if (CallPrevWndProc(HDM_GETITEMW, (WPARAM)pnmcd->dwItemSpec, (LPARAM)&item))
		{
			DWORD format;
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, WADlg_getColor(WADLG_LISTHEADER_FONTCOLOR));

			SetRect(&rcText, prc->left + 5 + ox, prc->top + 2 + oy, textRight, prc->bottom - 2);
			format = DT_VCENTER | DT_SINGLELINE | DT_LEFT | DT_NOCLIP;

			switch(HDF_JUSTIFYMASK & item.fmt)
			{
				case HDF_RIGHT: format |= DT_RIGHT; break;
				case HDF_CENTER: format |= DT_CENTER; break;
			}
			// because we draw with DT_NOCLIP, make sure we don't overhang to the right
			RECT testRect = rcText;
			DrawTextW(hdc, item.pszText, -1, &testRect, format | DT_CALCRECT);
			if (testRect.right > textRight)
			{
				format &= ~(HDF_JUSTIFYMASK | DT_NOCLIP);
				format |= DT_LEFT;

				if ((testRect.right - textRight) > 6)
					format |= DT_END_ELLIPSIS;
			}

			DrawTextW(hdc, item.pszText, -1, &rcText, format);
		}
	}

	SelectObject(hdc, penOld);
}

BOOL SkinnedHeader::OnCustomDraw(HWND hwndFrom, NMCUSTOMDRAW *pnmcd, LRESULT *pResult)
{
	switch(pnmcd->dwDrawStage)
	{
		case CDDS_PREPAINT:		
			*pResult = (SWS_USESKINCOLORS & style) ? 
						CDRF_NOTIFYITEMDRAW : 
						CDRF_DODEFAULT; 
			return TRUE;	
		case CDDS_ITEMPREPAINT:	
			DrawHeaderItem(pnmcd); 
			*pResult = CDRF_SKIPDEFAULT; 
			return TRUE;
	}
	return FALSE;
}

UINT SkinnedHeader::SizeRuleAdjustOne(HDITEMW *phdi, INT index, UINT uMsg)
{
	UINT result, flags;
	
	flags = hdrFlags;
	hdrFlags |= HIF_REALCXY;

	result = 0;
	if (index < hdrSize.count)
	{
		INT delta;
		delta = phdi->cxy - hdrSize.pCXY[index];

		hdrSize.hdi.mask = HDI_WIDTH;
		phdi = &hdrSize.hdi;

		if (delta)
		{
			if(CallPrevWndProc(HDM_GETITEMW, index + 1, (LPARAM)phdi)) 
			{
				INT temp = phdi->cxy;
				phdi->cxy -= delta;
				if (CallPrevWndProc(HDM_SETITEMW, index + 1, (LPARAM)phdi)) delta = temp - phdi->cxy;
				else delta = 0;
			}
			hdrSize.pCXY[index] += delta;
			result = 1;
		}
	}

	hdrFlags = flags;

	return result;
}

UINT SkinnedHeader::SizeRuleProportional(HDITEMW *phdi, INT index, UINT uMsg)
{
	UINT flags;
	INT o, delta;

	if (index >= hdrSize.count || 
		hdrSize.lastCXY == phdi->cxy || 
		0 == (phdi->cxy - hdrSize.pCXY[index])) 
	{
		return 0;
	}

	flags = hdrFlags;
	hdrFlags |= HIF_REALCXY;

	delta = phdi->cxy - hdrSize.pCXY[index];

	hdrSize.lastCXY = phdi->cxy;

	for (o = 0; o < hdrSize.count; o++) 
	{
		if (hdrSize.pOrder[o] == index) 
			break;
	}

	phdi = &hdrSize.hdi;
	phdi->mask = HDI_WIDTH;

	for(++o; o < hdrSize.count && 0 != delta; o++)
	{
		INT i = hdrSize.pOrder[o];

		INT r =  delta / (hdrSize.count - o) + ((delta % (hdrSize.count - o)) ? ((delta >0) ? 1 : -1) : 0);
		if (0 == r)
			r = delta;

		phdi->cxy = hdrSize.pCXY[i] - r;
		INT temp = phdi->cxy;

		CallPrevWndProc(HDM_SETITEMW, i, (LPARAM)phdi);
		delta -= r;

	//	hdrSize.pCXY[i] = phdi->cxy;

		if (phdi->cxy != temp) 
			delta += (phdi->cxy- temp);
	}

	hdrFlags = flags;

	return 1;
}

BOOL SkinnedHeader::OnBeginTrack(HWND hwndFrom, NMHEADERW *phdn, LRESULT *pResult)
{
	if (HIF_DEFSIZERULE & hdrFlags) hdrSizeRule = GetDefaultSizeRule();

	KillTimer(hwnd, TIMER_ENDTRACK_ID);

	hdrSize.headerCXY	= 0;
	hdrSize.lastCXY		= -1;
	hdrSize.hwndParent = hwndFrom;

	hdrSize.count		= (INT)CallPrevWndProc(HDM_GETITEMCOUNT, 0, 0L);
	if (hdrSize.count < 0)  hdrSize.count = 0;

	if (hdrSize.count > 0)
	{
		HDITEMW item;

		if (hdrSize.count > hdrSize.allocated)
		{
			VOID *data;
			data = realloc(hdrSize.pCXY, sizeof(INT)*hdrSize.count*2);
			if (data) 
			{
				hdrSize.pCXY = (INT*)data;
				hdrSize.pOrder = hdrSize.pCXY + hdrSize.count;
				hdrSize.allocated = hdrSize.count;
			}
			if (!data) hdrSize.count = 0;
		}

		if (hdrSize.count)
		{
			INT i;
			if (!CallPrevWndProc(HDM_GETORDERARRAY, hdrSize.count, (LPARAM)hdrSize.pOrder))
			{
				for (i = 0; i < hdrSize.count; i++) hdrSize.pOrder[i] = i;
			}

			item.mask = HDI_WIDTH; 
			for (INT o = 0; o < hdrSize.count; o++)
			{
				i = hdrSize.pOrder[o];
				if (SendMessageW(hwnd, HDM_GETITEMW, i, (LPARAM)&item) && item.lParam) 
				{
					hdrSize.pCXY[i] = item.cxy;
					hdrSize.headerCXY += item.cxy;
				}
			}
		}
	}

	return FALSE;
}

BOOL SkinnedHeader::OnEndTrack(HWND hwndFrom, NMHEADERW *phdn)
{
	if (hdrSize.count)
	{
		BlockRedraw(FALSE, 0);
		hdrFlags &= ~(HIF_BLOCKCHANGING | HIF_BLOCKCHANGED);
		SetTimer(hwnd, TIMER_ENDTRACK_ID, TIMER_ENDTRACK_DELAY, NULL);
	}
	return FALSE;
}

BOOL SkinnedHeader::OnItemChanging(HWND hwndFrom, NMHEADERW *phdn, LRESULT *pResult)
{
	if (HIF_BLOCKCHANGING & hdrFlags) 
	{ 
		*pResult = FALSE; 
		return TRUE; 
	}

	if (0 != hdrSize.count && 
		0 == (HIF_REALCXY & hdrFlags))
	{
		switch(hdrSizeRule)
		{
			case SHS_SIZERULE_ADJUSTONE:	
			case SHS_SIZERULE_PROPORTIONAL:
			case SHS_SIZERULE_ADJUSTALL:	
					BlockRedraw(TRUE, 10);
					break;
		}
	}
	return FALSE;
}

BOOL SkinnedHeader::OnItemChanged(HWND hwndFrom, NMHEADERW *phdn)
{
	UINT result;

	if (HIF_BLOCKCHANGED & hdrFlags) 
		return TRUE; 

	if((HIF_REALCXY & hdrFlags) || !phdn->pitem || 0== (HDI_WIDTH & phdn->pitem->mask)) 
		return FALSE;
 
	hdrFlags |= HIF_BLOCKCHANGED;

	switch(hdrSizeRule)
	{
		case SHS_SIZERULE_ADJUSTONE:	result = SizeRuleAdjustOne(phdn->pitem, phdn->iItem, phdn->hdr.code); break;
		case SHS_SIZERULE_PROPORTIONAL: result = SizeRuleProportional(phdn->pitem, phdn->iItem, phdn->hdr.code); break;
		case SHS_SIZERULE_ADJUSTALL:	result = 0; break;
		default: result = 0; break;
	}

    hdrFlags &= ~HIF_BLOCKCHANGED;

	if (result)
	{
		if(hdrSize.count)
		{
			INT i;
			HDITEMW item;
			item.mask = HDI_WIDTH; 
			i = hdrSize.pOrder[hdrSize.count - 1];
			if (CallPrevWndProc(HDM_GETITEMW, i, (LPARAM)&item))
			{
				long cxy = item.cxy;
				hdrFlags |= (HIF_BLOCKCHANGING | HIF_BLOCKCHANGED | HIF_REALCXY);
				item.cxy = cxy + 1; 
				CallPrevWndProc(HDM_SETITEMW, i, (LPARAM)&item);
				hdrFlags &= ~(HIF_BLOCKCHANGING | HIF_BLOCKCHANGED);
				item.cxy = cxy; 	
				CallPrevWndProc(HDM_SETITEMW, i, (LPARAM)&item);
				hdrFlags &= ~HIF_REALCXY;
			}
		}

		BlockRedraw(FALSE, 0);

		if (NULL != hdrSize.hwndParent) 
		{
			RedrawWindow(hdrSize.hwndParent, NULL, NULL, 
				RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN/*| RDW_VALIDATE*/);
		}
		else
			RedrawWindow(hwnd, NULL, NULL, 
				RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN/*| RDW_VALIDATE*/);

		return TRUE;
	}

	return FALSE;
}

BOOL SkinnedHeader::OnCursorNotify(HWND hwndFrom, NMMOUSE *pnm, LRESULT *pResult)
{
	if (0 == ((HHT_ONDIVIDER | HHT_ONDIVOPEN)& pnm->dwHitInfo) && (SWS_USESKINCURSORS & style) && hcurNormal) 
	{
		SetCursor(hcurNormal);
		*pResult = TRUE;
		return TRUE;
	}
	return FALSE;
}

BOOL SkinnedHeader::OnReflectedNotify(INT idCtrl, REFLECTPARAM *rParam)
{
	if (rParam->hwndFrom != GetParent(hwnd)) return FALSE; // controls such as listview forward headers notifications to they parent.

	switch(((NMHDR*)(rParam->lParam))->code)
	{
		case NM_CUSTOMDRAW:		return OnCustomDraw(rParam->hwndFrom, (NMCUSTOMDRAW*)rParam->lParam, &rParam->result);
		case HDN_ITEMCHANGINGA:
		case HDN_ITEMCHANGINGW:	return OnItemChanging(rParam->hwndFrom, (NMHEADERW*)rParam->lParam, &rParam->result);
		case HDN_ITEMCHANGEDA:
		case HDN_ITEMCHANGEDW:	return OnItemChanged(rParam->hwndFrom, (NMHEADERW*)rParam->lParam);
		case HDN_BEGINTRACKA:
		case HDN_BEGINTRACKW:	return OnBeginTrack(rParam->hwndFrom, (NMHEADERW*)rParam->lParam, &rParam->result);
		case HDN_ENDTRACKA:
		case HDN_ENDTRACKW:		return OnEndTrack(rParam->hwndFrom, (NMHEADERW*)rParam->lParam); 
		case NM_SETCURSOR:		return OnCursorNotify(rParam->hwndFrom, (NMMOUSE*)rParam->lParam, &rParam->result);
	}

	return FALSE;
}

void SkinnedHeader::OnPaint(void)
{
	if (HIF_BLOCKREDRAW & hdrFlags)
	{
		ValidateRgn(hwnd, NULL);
	}
	else if (SWS_USESKINCOLORS & style)
	{
		//process the grey area with our color
		RECT rc, ri;
		int i;
		GetClientRect(hwnd, &rc);
		i = (INT)CallPrevWndProc(HDM_GETITEMCOUNT, 0, 0L);
		if(i > 0)
		{
			Header_GetItemRect(hwnd, (INT)CallPrevWndProc(HDM_ORDERTOINDEX, (i-1), 0L), &ri);
			rc.left = ri.right;
		}

		if(rc.left < rc.right && GetUpdateRect(hwnd, &ri, FALSE) && IntersectRect(&rc, &rc, &ri))
		{
			UINT flags = DCX_PARENTCLIP | DCX_CACHE | DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN |
						 DCX_INTERSECTUPDATE | DCX_VALIDATE;

			HDC hdc = GetDCEx(hwnd, NULL, flags);
			if(hdc)
			{
				COLORREF rgbBkOld;
				rgbBkOld  = SetBkColor(hdc, WADlg_getColor(WADLG_LISTHEADER_EMPTY_BGCOLOR));
				ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rc, L"", 0, 0);
				SetBkColor(hdc, rgbBkOld);
				ReleaseDC(hwnd, hdc);
				ValidateRect(hwnd, &rc);
			}
		}
	}
}

void SkinnedHeader::OnTimer(UINT_PTR nIDEvent, TIMERPROC lpTimerFunc)
{
	switch (nIDEvent)
	{
		case TIMER_UNBLOCKREDRAW_ID:
			BlockRedraw(FALSE, 0);
			return;
		case TIMER_ENDTRACK_ID:
			KillTimer(hwnd, TIMER_ENDTRACK_ID);
			hdrSize.count		= 0;
			hdrSize.headerCXY	= 0;
			hdrSize.lastCXY		= -1;
			return;
	}
	__super::WindowProc(WM_TIMER, (WPARAM)nIDEvent, (LPARAM)lpTimerFunc);
}

void SkinnedHeader::SetHeight(INT nHeight)
{
	RECT rw;
	GetWindowRect(hwnd, &rw);

	if (nHeight < 0)
	{
		HDC hdc;
		hdc = GetWindowDC(hwnd);
		if (hdc)
		{
			HFONT hfnt;
			hfnt = (HFONT)CallPrevWndProc(WM_GETFONT, 0, 0L);
			if (!hfnt) hfnt = (HFONT)MlStockObjects_Get(DEFAULT_FONT);
			if (hfnt)
			{
				TEXTMETRICW tm = {0};
 				HFONT hfntOld = (HFONT)SelectObject(hdc, hfnt);

				if (GetTextMetricsW(hdc, &tm)) nHeight =  tm.tmHeight + 6/*Borders*/;
				SelectObject(hdc, hfntOld);
			}
			ReleaseDC(hwnd, hdc);
		}
	}

	if (nHeight != (rw.bottom - rw.top))
	{
		INT i;

		SetWindowPos(hwnd, NULL, 0, 0, rw.right - rw.left, nHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
		if (HDS_HIDDEN & GetWindowLongPtrW(hwnd, GWL_STYLE)) return;

		i = (INT)CallPrevWndProc(HDM_GETITEMCOUNT, 0, 0L);
		if(i > 0)
		{
			HDITEMW item;
			i = (INT)CallPrevWndProc(HDM_ORDERTOINDEX, (i-1), 0L);
			item.mask = HDI_WIDTH; 
			if (CallPrevWndProc(HDM_GETITEMW, i, (LPARAM)&item))
			{
				hdrFlags |= (HIF_BLOCKCHANGING | HIF_BLOCKCHANGED | HIF_REALCXY);
				item.cxy++; CallPrevWndProc(HDM_SETITEMW, i, (LPARAM)&item);
				hdrFlags &= ~(HIF_BLOCKCHANGING | HIF_BLOCKCHANGED);
				item.cxy--; 	CallPrevWndProc(HDM_SETITEMW, i, (LPARAM)&item);
				hdrFlags &= ~HIF_REALCXY;
			}
		}
	}
}

BOOL SkinnedHeader::OnLayout(HDLAYOUT *pLayout)
{
	BOOL result;

	result = (BOOL)__super::WindowProc(HDM_LAYOUT, 0, (LPARAM)pLayout);

	if (0 == (HDS_HIDDEN & GetWindowLongPtrW(hwnd, GWL_STYLE)))
	{
		RECT rw;

		GetWindowRect(hwnd, &rw);
		OffsetRect(&rw, -rw.left, -rw.top);

		pLayout->pwpos->cy= rw.bottom;
		pLayout->prc->top = rw.bottom;

		if (GetClientRect(GetParent(hwnd), &rw))
			pLayout->pwpos->cx = (rw.right - rw.left) - pLayout->pwpos->x;
	}
	return result;
}

BOOL SkinnedHeader::OnSetCursor(HWND hwdCursor, UINT hitTest, UINT message)
{
	static HDCURSOR hdrCursor;

	hdrCursor.hwndParent = GetParent(hwnd);
	if (hdrCursor.hwndParent)
	{
		hdrCursor.nm.hdr.code = NM_SETCURSOR;
		hdrCursor.nm.hdr.hwndFrom = hwnd;
		hdrCursor.nm.hdr.idFrom = GetDlgCtrlID(hwnd);

		hdrCursor.pts = GetMessagePos();
		POINTSTOPOINT(hdrCursor.nm.pt, hdrCursor.pts);

		hdrCursor.hitTest.pt = hdrCursor.nm.pt;
		MapWindowPoints(HWND_DESKTOP, hwnd, &hdrCursor.hitTest.pt, 1);
		CallPrevWndProc(HDM_HITTEST, 0, (LPARAM)&hdrCursor.hitTest);

		hdrCursor.nm.dwItemSpec = hdrCursor.hitTest.iItem;
		hdrCursor.nm.dwHitInfo = hdrCursor.hitTest.flags;
		if (SendMessage(hdrCursor.hwndParent, WM_NOTIFY, (WPARAM)hdrCursor.nm.hdr.code, (LPARAM)&hdrCursor.nm)) 
			return TRUE;
	}
	return (BOOL)__super::WindowProc(WM_SETCURSOR, (WPARAM)hwdCursor, MAKELPARAM(hitTest, message));
}

LRESULT SkinnedHeader::OnSetItem(INT iIndex, HDITEMW *phdItem, BOOL bUnicode)
{
	if (NULL != phdItem && 
		0 != (HDI_FORMAT & phdItem->mask))
	{
		DWORD newFlags = 0x0000FFFF | (hdrFlags & (0xFFFF0000 & ~HIF_ASCENDING));
		if (0 != ((HDF_SORTDOWN | HDF_SORTUP) & phdItem->fmt) &&
			iIndex >= 0 && iIndex < 0xFFFF)
		{
			newFlags = (newFlags & 0xFFFF0000) | (0xFFFF & iIndex);
			if (0 != (HDF_SORTUP & phdItem->fmt))
				newFlags |= HIF_ASCENDING;
		}

		if ((LOWORD(newFlags) != LOWORD(hdrFlags)) ||
			((HIF_ASCENDING & newFlags) != (HIF_ASCENDING & hdrFlags)))
		{
			hdrFlags = newFlags;
			RECT rcItem;
			if (NULL != CallPrevWndProc(HDM_GETITEMRECT, (WPARAM)iIndex, (LPARAM)&rcItem))
			{
				InvalidateRect(hwnd, &rcItem, FALSE);
			}
		}
	}

	return __super::WindowProc(((bUnicode) ? HDM_SETITEMW : HDM_SETITEMA), (WPARAM)iIndex, (LPARAM)phdItem);
}

BOOL SkinnedHeader::OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult)
{
	switch(msg)
	{
		case ML_IPC_SKINNEDHEADER_DISPLAYSORT:	
			hdrFlags = (hdrFlags & 0xFFFF0000) | LOWORD(param);
			hdrFlags = (hdrFlags & ~HIF_ASCENDING) | ((HIWORD(param)) ? HIF_ASCENDING : 0);
			*pResult = 1;
			InvalidateRect(hwnd, NULL, FALSE);
			UpdateWindow(hwnd);
			return TRUE;
		case ML_IPC_SKINNEDHEADER_SETHEIGHT: SetHeight((INT)param); *pResult = TRUE; return TRUE;
		case ML_IPC_SKINNEDHEADER_GETSORT:	*pResult = MAKELONG(LOWORD(hdrFlags), 0 != (HIF_ASCENDING & hdrFlags)); return TRUE;
		case ML_IPC_SKINNEDHEADER_SETCLOUDCOLUMN:
			cloudColumn = (INT)param;
			InvalidateRect(hwnd, NULL, FALSE);
			UpdateWindow(hwnd);
			return TRUE;
	}
	return __super::OnMediaLibraryIPC(msg, param, pResult);
}

LRESULT SkinnedHeader::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_ERASEBKGND:			return 1;
  		case WM_PAINT:				OnPaint(); break;
		case WM_TIMER:				OnTimer((UINT_PTR)wParam, (TIMERPROC)lParam); return 0;
		case WM_SETCURSOR:			return OnSetCursor((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
		case WM_CAPTURECHANGED:		InvalidateRect(hwnd, NULL, TRUE); break;
		case WM_WINDOWPOSCHANGING:	if (HIF_BLOCKWNDPOS & hdrFlags) return TRUE; 
									break;
		case REFLECTED_NOTIFY:		return OnReflectedNotify((INT)wParam, (REFLECTPARAM*)lParam);
		case HDM_LAYOUT:			return OnLayout((LPHDLAYOUT)lParam);
		case HDM_SETITEMA:		
		case HDM_SETITEMW:		
			return OnSetItem((INT)wParam, (HDITEMW*)lParam, (HDM_SETITEMW == uMsg));
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}