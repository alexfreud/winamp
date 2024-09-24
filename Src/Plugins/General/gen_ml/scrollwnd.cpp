// some code taken from (freeware) Cool ScrollBar library by J Brown
#include "main.h"
#include <windowsx.h>
#include <tchar.h>
#include "scrollwnd.h"
#include "../winamp/wa_dlg.h"

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#endif


extern HRESULT(WINAPI *SetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);  //xp theme shit
extern HRESULT(WINAPI *IsAppThemed)(void);

static TCHAR szPropStr[] = _T("CoolSBSubclassPtr");

//
//	Special thumb-tracking variables
//
//
static UINT uCurrentScrollbar = COOLSB_NONE;	//SB_HORZ / SB_VERT
static UINT uCurrentScrollPortion = HTSCROLL_NONE;
static UINT uCurrentButton = 0;

static RECT rcThumbBounds;		//area that the scroll thumb can travel in
static int  nThumbSize;			//(pixels)
static int  nThumbPos;			//(pixels)
static int  nThumbMouseOffset;	//(pixels)
static int  nLastPos = -1;		//(scrollbar units)
static int  nThumbPos0;			//(pixels) initial thumb position

//
//	Temporary state used to auto-generate timer messages
//
static UINT uMouseOverId = 0;
static UINT uMouseOverScrollbar = COOLSB_NONE;
static UINT uHitTestPortion = HTSCROLL_NONE;
static UINT uLastHitTestPortion = HTSCROLL_NONE;
static RECT MouseOverRect;

static UINT uScrollTimerMsg = 0;
static UINT uScrollTimerPortion = HTSCROLL_NONE;
static UINT_PTR uScrollTimerId = 0;
static HWND hwndCurCoolSB = 0;

ScrollWnd *GetScrollWndFromHwnd(HWND hwnd)
{
	return (ScrollWnd *)GetProp(hwnd, szPropStr);
}

//
//	swap the rectangle's x coords with its y coords
//
static void __stdcall RotateRect(RECT *rect)
{
	int temp;
	temp = rect->left;
	rect->left = rect->top;
	rect->top = temp;

	temp = rect->right;
	rect->right = rect->bottom;
	rect->bottom = temp;
}

//
//	swap the coords if the scrollbar is a SB_VERT
//
static void __stdcall RotateRect0(SCROLLBAR *sb, RECT *rect)
{
	if (sb->nBarType == SB_VERT)
		RotateRect(rect);
}

//
//	Calculate if the SCROLLINFO members produce
//  an enabled or disabled scrollbar
//
static BOOL IsScrollInfoActive(SCROLLINFO *si)
{
	if ((si->nPage > (UINT)si->nMax
	     || si->nMax <= si->nMin || si->nMax == 0))
		return FALSE;
	else
		return TRUE;
}

//
//	Return if the specified scrollbar is enabled or not
//
static BOOL IsScrollbarActive(SCROLLBAR *sb)
{
	SCROLLINFO *si = &sb->scrollInfo;
	if (((sb->fScrollFlags & ESB_DISABLE_BOTH) == ESB_DISABLE_BOTH) ||
	    !(sb->fScrollFlags & CSBS_THUMBALWAYS) && !IsScrollInfoActive(si))
		return FALSE;
	else
		return TRUE;
}

BOOL drawFrameControl(HDC hdc, LPRECT lprc, UINT uType, UINT state)
{
	HDC hdcbmp;
	HBITMAP hbmpOld, hbmp;
	int startx, starty;

	hbmp = WADlg_getBitmap();
	if (!hbmp) return FALSE;

	hdcbmp = CreateCompatibleDC(hdc);
	if (!hdcbmp) return FALSE;

	hbmpOld = (HBITMAP)SelectObject(hdcbmp, hbmp);
	
	startx = 0;
	starty = 31;
	switch (state&3)
	{
		case DFCS_SCROLLRIGHT: startx = 14; starty = 45; break;
		case DFCS_SCROLLLEFT: startx = 0; starty = 45; break;
		case DFCS_SCROLLDOWN: startx = 14; starty = 31; break;
	}
	if (state&DFCS_PUSHED) startx += 28;
	StretchBlt(hdc, lprc->left, lprc->top, lprc->right - lprc->left, lprc->bottom - lprc->top, hdcbmp, startx, starty, 14, 14, SRCCOPY);
	
	SelectObject(hdcbmp, hbmpOld);
	DeleteDC(hdcbmp);
	return 1;
}

//
//	Draw a standard scrollbar arrow
//
static int DrawScrollArrow(SCROLLBAR *sbar, HDC hdc, RECT *rect, UINT arrow, BOOL fMouseDown, BOOL fMouseOver)
{
	UINT ret;
	UINT flags = arrow;

	//HACKY bit so this routine can be called by vertical and horizontal code
	if (sbar->nBarType == SB_VERT)
	{
		if (flags & DFCS_SCROLLLEFT)		flags = flags & ~DFCS_SCROLLLEFT  | DFCS_SCROLLUP;
		if (flags & DFCS_SCROLLRIGHT)	flags = flags & ~DFCS_SCROLLRIGHT | DFCS_SCROLLDOWN;
	}

	if (fMouseDown) flags |= (DFCS_FLAT | DFCS_PUSHED);

	ret = drawFrameControl(hdc, rect, DFC_SCROLL, flags);

	return ret;
}

//
//	Return the size in pixels for the specified scrollbar metric,
//  for the specified scrollbar
//
static int GetScrollMetric(SCROLLBAR *sbar, int metric)
{
	if (sbar->nBarType == SB_HORZ)
	{
		if (metric == SM_CXHORZSB)
		{
			if (sbar->nArrowLength < 0)
				return -sbar->nArrowLength * 14; //GetSystemMetrics(SM_CXHSCROLL);
			else
				return sbar->nArrowLength;
		}
		else
		{
			if (sbar->nArrowWidth < 0)
				return -sbar->nArrowWidth * 14; //GetSystemMetrics(SM_CYHSCROLL);
			else
				return sbar->nArrowWidth;
		}
	}
	else if (sbar->nBarType == SB_VERT)
	{
		if (metric == SM_CYVERTSB)
		{
			if (sbar->nArrowLength < 0)
				return -sbar->nArrowLength * 14;//GetSystemMetrics(SM_CYVSCROLL);
			else
				return sbar->nArrowLength;
		}
		else
		{
			if (sbar->nArrowWidth < 0)
				return -sbar->nArrowWidth * 14;//GetSystemMetrics(SM_CXVSCROLL);
			else
				return sbar->nArrowWidth;
		}
	}

	return 0;
}

//
//
//
static COLORREF GetSBForeColor(void)
{
	return WADlg_getColor(WADLG_SCROLLBAR_FGCOLOR);
}

static COLORREF GetSBBackColor(void)
{
	return WADlg_getColor(WADLG_SCROLLBAR_BGCOLOR);
}

//
//	Paint a checkered rectangle, with each alternate
//	pixel being assigned a different colour
//
static void DrawCheckedRect(HDC hdc, RECT *rect, COLORREF fg, COLORREF bg)
{
	static WORD wCheckPat[8] =
	  {
	    0xaaaa, 0x5555, 0xaaaa, 0x5555, 0xaaaa, 0x5555, 0xaaaa, 0x5555
	  };

	HBITMAP hbmp;
	HBRUSH  hbr, hbrold;
	COLORREF fgold, bgold;

	hbmp = CreateBitmap(8, 8, 1, 1, wCheckPat);
	hbr  = CreatePatternBrush(hbmp);

	UnrealizeObject(hbr);
	SetBrushOrgEx(hdc, rect->left, rect->top, 0);

	hbrold = (HBRUSH)SelectObject(hdc, hbr);

	fgold = SetTextColor(hdc, fg);
	bgold = SetBkColor(hdc, bg);

	PatBlt(hdc, rect->left, rect->top,
	       rect->right - rect->left,
	       rect->bottom - rect->top,
	       PATCOPY);

	SetBkColor(hdc, bgold);
	SetTextColor(hdc, fgold);

	SelectObject(hdc, hbrold);
	DeleteObject(hbr);
	DeleteObject(hbmp);
}

//
//	Fill the specifed rectangle using a solid colour
//
static void PaintRect(HDC hdc, RECT *rect, COLORREF color)
{
	COLORREF oldcol = SetBkColor(hdc, color);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, rect, _T(""), 0, 0);
	SetBkColor(hdc, oldcol);
}

//
//	Draw a simple blank scrollbar push-button. Can be used
//	to draw a push button, or the scrollbar thumb
//	drawflag - could set to BF_FLAT to make flat scrollbars
//
void DrawBlankButton(HDC hdc, const RECT *rect, UINT drawflag, int pushed, int vertical)
{
	HBITMAP hbmp, hbmpOld;
	hbmp = WADlg_getBitmap();
	if (!hbmp) return;

	HDC hdcbmp = CreateCompatibleDC(hdc);
	if (!hdcbmp) return;

	hbmpOld = (HBITMAP)SelectObject(hdcbmp, hbmp);

#define PART1SIZE 4  //copied top
#define PART2SIZE 5  //stretched top
#define PART3SIZE 10  //copied middle
#define PART4SIZE 5  //stretched bottom
#define PART5SIZE 4  //copied bottom

	if (vertical)
	{
		int middle = (rect->bottom - rect->top) / 2;
		int startx = pushed ? 70 : 56;
		//top
		StretchBlt(hdc, rect->left, rect->top, rect->right - rect->left, PART1SIZE, hdcbmp, startx, 31, 14, PART1SIZE, SRCCOPY);
		int p = PART1SIZE;
		//stretched top
		int l = middle - PART1SIZE - (PART3SIZE / 2);
		if (l > 0)
		{
			StretchBlt(hdc, rect->left, rect->top + p, rect->right - rect->left, l, hdcbmp, startx, 31 + PART1SIZE, 14, PART2SIZE, SRCCOPY);
			p += middle - PART1SIZE - (PART3SIZE / 2);
		}
		//copied middle
		int m = (rect->bottom - rect->top) - PART1SIZE - PART5SIZE; //space that's available for middle
		m = min(m, PART3SIZE);
		if (m > 0)
		{
			StretchBlt(hdc, rect->left, rect->top + p, rect->right - rect->left, m, hdcbmp, startx, 31 + PART1SIZE + PART2SIZE, 14, m, SRCCOPY);
			p += m;
		}
		//stretched bottom
		l = rect->bottom - rect->top - p - PART5SIZE;
		if (l > 0) StretchBlt(hdc, rect->left, rect->top + p, rect->right - rect->left, l, hdcbmp, startx, 31 + PART1SIZE + PART2SIZE + PART3SIZE, 14, PART4SIZE, SRCCOPY);
		//bottom
		StretchBlt(hdc, rect->left, rect->bottom - PART5SIZE, rect->right - rect->left, PART5SIZE, hdcbmp, startx, 31 + PART1SIZE + PART2SIZE + PART3SIZE + PART4SIZE, 14, PART5SIZE, SRCCOPY);
	}
	else
	{
		int middle = (rect->right - rect->left) / 2;
		int starty = pushed ? 45 : 31;
		//top
		StretchBlt(hdc, rect->left, rect->top, PART1SIZE, rect->bottom - rect->top, hdcbmp, 84, starty, PART1SIZE, 14, SRCCOPY);
		int p = PART1SIZE;
		//stretched top
		int l = middle - PART1SIZE - (PART3SIZE / 2);
		if (l > 0)
		{
			StretchBlt(hdc, rect->left + p, rect->top, l, rect->bottom - rect->top, hdcbmp, 84 + PART1SIZE, starty, PART2SIZE, 14, SRCCOPY);
			p += middle - PART1SIZE - (PART3SIZE / 2);
		}
		//copied middle
		int m = (rect->right - rect->left) - PART1SIZE - PART5SIZE; //space that's available for middle
		m = min(m, PART3SIZE);
		if (m > 0)
		{
			StretchBlt(hdc, rect->left + p, rect->top, m, rect->bottom - rect->top, hdcbmp, 84 + PART1SIZE + PART2SIZE, starty, m, 14, SRCCOPY);
			p += m;
		}
		//stretched bottom
		l = rect->right - rect->left - p - PART5SIZE;
		if (l > 0) StretchBlt(hdc, rect->left + p, rect->top, l, rect->bottom - rect->top, hdcbmp, 84 + PART1SIZE + PART2SIZE + PART3SIZE, starty, PART4SIZE, 14, SRCCOPY);
		//bottom
		StretchBlt(hdc, rect->right - PART5SIZE, rect->top, PART5SIZE, rect->bottom - rect->top, hdcbmp, 84 + PART1SIZE + PART2SIZE + PART3SIZE + PART4SIZE, starty, PART5SIZE, 14, SRCCOPY);
	}

	SelectObject(hdcbmp, hbmpOld);
	DeleteDC(hdcbmp);
}

//
//	Send a WM_VSCROLL or WM_HSCROLL message
//
static void SendScrollMessage(HWND hwnd, UINT scrMsg, UINT scrId, UINT pos)
{
	SendMessage(hwnd, scrMsg, MAKEWPARAM(scrId, pos), 0);
}

//
//	Calculate the screen coordinates of the area taken by
//  the horizontal scrollbar. Take into account the size
//  of the window borders
//
static BOOL GetHScrollRect(ScrollWnd *sw, HWND hwnd, RECT *rect)
{
	GetWindowRect(hwnd, rect);

	if (sw->fLeftScrollbar)
	{
		rect->left  += sw->cxLeftEdge + (sw->sbarVert.fScrollVisible ?
		                                 GetScrollMetric(&sw->sbarVert, SM_CXVERTSB) : 0);
		rect->right -= sw->cxRightEdge;
	}
	else
	{
		rect->left   += sw->cxLeftEdge;					//left window edge

		rect->right  -= sw->cxRightEdge +				//right window edge
		                (sw->sbarVert.fScrollVisible ?
		                 GetScrollMetric(&sw->sbarVert, SM_CXVERTSB) : 0);
	}

	rect->bottom -= sw->cyBottomEdge;				//bottom window edge

	rect->top	  = rect->bottom -
	              (sw->sbarHorz.fScrollVisible ?
	               GetScrollMetric(&sw->sbarHorz, SM_CYHORZSB) : 0);

	return TRUE;
}

//
//	Calculate the screen coordinates of the area taken by the
//  vertical scrollbar
//
static BOOL GetVScrollRect(ScrollWnd *sw, HWND hwnd, RECT *rect)
{
	GetWindowRect(hwnd, rect);
	rect->top	 += sw->cyTopEdge;						//top window edge

	rect->bottom -= sw->cyBottomEdge +
	                (sw->sbarHorz.fScrollVisible ?		//bottom window edge
	                 GetScrollMetric(&sw->sbarHorz, SM_CYHORZSB) : 0);

	if (sw->fLeftScrollbar)
	{
		rect->left	+= sw->cxLeftEdge;
		rect->right = rect->left + (sw->sbarVert.fScrollVisible ?
		                            GetScrollMetric(&sw->sbarVert, SM_CXVERTSB) : 0);
	}
	else
	{
		rect->right  -= sw->cxRightEdge;
		rect->left    = rect->right - (sw->sbarVert.fScrollVisible ?
		                               GetScrollMetric(&sw->sbarVert, SM_CXVERTSB) : 0);
	}

	return TRUE;
}

//	Depending on what type of scrollbar nBar refers to, call the
//  appropriate Get?ScrollRect function
//
BOOL GetScrollRect(ScrollWnd *sw, UINT nBar, HWND hwnd, RECT *rect)
{
	if (nBar == SB_HORZ)
		return GetHScrollRect(sw, hwnd, rect);
	else if (nBar == SB_VERT)
		return GetVScrollRect(sw, hwnd, rect);
	else
		return FALSE;
}

//
//	Work out the scrollbar width/height for either type of scrollbar (SB_HORZ/SB_VERT)
//	rect - coords of the scrollbar.
//	store results into *thumbsize and *thumbpos
//
static int CalcThumbSize(SCROLLBAR *sbar, const RECT *rect, int *pthumbsize, int *pthumbpos)
{
	SCROLLINFO *si;
	int scrollsize;			//total size of the scrollbar including arrow buttons
	int workingsize;		//working area (where the thumb can slide)
	int siMaxMin;
	int butsize;
	int startcoord;
	int thumbpos = 0, thumbsize = 0;

	//work out the width (for a horizontal) or the height (for a vertical)
	//of a standard scrollbar button
	butsize = GetScrollMetric(sbar, SM_SCROLL_LENGTH);

	if (1) //sbar->nBarType == SB_HORZ)
	{
		scrollsize = rect->right - rect->left;
		startcoord = rect->left;
	}
	/*else if(sbar->nBarType == SB_VERT)
	{
		scrollsize = rect->bottom - rect->top;
		startcoord = rect->top;
	}
	else
	{
		return 0;
	}*/

	si = &sbar->scrollInfo;
	siMaxMin = si->nMax - si->nMin + 1;
	workingsize = scrollsize - butsize * 2;

	//
	// Work out the scrollbar thumb SIZE
	//
	if (si->nPage == 0)
	{
		thumbsize = butsize;
	}
	else if (siMaxMin > 0)
	{
		thumbsize = MulDiv(si->nPage, workingsize, siMaxMin);

		if (thumbsize < sbar->nMinThumbSize)
			thumbsize = sbar->nMinThumbSize;
	}

	//
	// Work out the scrollbar thumb position
	//
	if (siMaxMin > 0)
	{
		int pagesize = max(1, si->nPage);
		thumbpos = MulDiv(si->nPos - si->nMin, workingsize - thumbsize, siMaxMin - pagesize);

		if (thumbpos < 0)
			thumbpos = 0;

		if (thumbpos >= workingsize - thumbsize)
			thumbpos = workingsize - thumbsize;
	}

	thumbpos += startcoord + butsize;

	*pthumbpos  = thumbpos;
	*pthumbsize = thumbsize;

	return 1;
}

//
//	return a hit-test value for whatever part of the scrollbar x,y is located in
//	rect, x, y: SCREEN coordinates
//	the rectangle must not include space for any inserted buttons
//	(i.e, JUST the scrollbar area)
//
static UINT GetHorzScrollPortion(SCROLLBAR *sbar, HWND hwnd, const RECT *rect, int x, int y)
{
	int thumbwidth, thumbpos;
	int butwidth = GetScrollMetric(sbar, SM_SCROLL_LENGTH);
	int scrollwidth  = rect->right - rect->left;
	int workingwidth = scrollwidth - butwidth * 2;

	if (y < rect->top || y >= rect->bottom)
		return HTSCROLL_NONE;

	CalcThumbSize(sbar, rect, &thumbwidth, &thumbpos);

	//if we have had to scale the buttons to fit in the rect,
	//then adjust the button width accordingly
	if (scrollwidth <= butwidth * 2)
	{
		butwidth = scrollwidth / 2;
	}

	//check for left button click
	if (x >= rect->left && x < rect->left + butwidth)
	{
		return HTSCROLL_LEFT;
	}
	//check for right button click
	else if (x >= rect->right - butwidth && x < rect->right)
	{
		return HTSCROLL_RIGHT;
	}

	//if the thumb is too big to fit (i.e. it isn't visible)
	//then return a NULL scrollbar area
	if (thumbwidth >= workingwidth)
		return HTSCROLL_NONE;

	//check for point in the thumbbar
	if (x >= thumbpos && x < thumbpos + thumbwidth)
	{
		return HTSCROLL_THUMB;
	}
	//check for left margin
	else if (x >= rect->left + butwidth && x < thumbpos)
	{
		return HTSCROLL_PAGELEFT;
	}
	else if (x >= thumbpos + thumbwidth && x < rect->right - butwidth)
	{
		return HTSCROLL_PAGERIGHT;
	}

	return HTSCROLL_NONE;
}

//
//	For vertical scrollbars, rotate all coordinates by -90 degrees
//	so that we can use the horizontal version of this function
//
static UINT GetVertScrollPortion(SCROLLBAR *sb, HWND hwnd, RECT *rect, int x, int y)
{
	UINT r;

	RotateRect(rect);
	r = GetHorzScrollPortion(sb, hwnd, rect, y, x);
	RotateRect(rect);
	return r;
}

//
//	CUSTOM DRAW support
//
static LRESULT PostCustomPrePostPaint0(HWND hwnd, HDC hdc, SCROLLBAR *sb, UINT dwStage)
{
	return 0;
}

static LRESULT PostCustomDrawNotify0(HWND hwnd, HDC hdc, UINT nBar, RECT *prect, UINT nItem, BOOL fMouseDown, BOOL fMouseOver, BOOL fInactive)
{
	return 0;
}

// Depending on if we are supporting custom draw, either define
// a macro to the function name, or to nothing at all. If custom draw
// is turned off, then we can save ALOT of code space by binning all
// calls to the custom draw support.
//
#define PostCustomDrawNotify	1 ? (void)0 : PostCustomDrawNotify0
#define PostCustomPrePostPaint	1 ? (void)0 : PostCustomPrePostPaint0

static LRESULT PostMouseNotify0(HWND hwnd, UINT msg, UINT nBar, RECT *prect, UINT nCmdId, POINT pt)
{
	return 0;
}

#ifdef NOTIFY_MOUSE
#define PostMouseNotify			PostMouseNotify0
#else
#define PostMouseNotify			1 ? (void)0 : PostMouseNotify0
#endif



//
//	Draw a complete HORIZONTAL scrollbar in the given rectangle
//	Don't draw any inserted buttons in this procedure
//
//	uDrawFlags - hittest code, to say if to draw the
//  specified portion in an active state or not.
//
//
static LRESULT NCDrawHScrollbar(SCROLLBAR *sb, HWND hwnd, HDC hdc, const RECT *rect, UINT uDrawFlags)
{
	SCROLLINFO *si;
	RECT ctrl, thumb;
	RECT sbm;
	int butwidth	 = GetScrollMetric(sb, SM_SCROLL_LENGTH);
	int scrollwidth  = rect->right - rect->left;
	int workingwidth = scrollwidth - butwidth * 2;
	int thumbwidth   = 0, thumbpos = 0;
	BOOL fCustomDraw = 0;

	BOOL fMouseDownL = 0, fMouseOverL = 0;
	BOOL fMouseDownR = 0, fMouseOverR = 0;

	COLORREF crCheck1   = GetSBForeColor();
	COLORREF crCheck2   = GetSBBackColor();
	COLORREF crInverse1 = WADlg_getColor(WADLG_SCROLLBAR_INV_FGCOLOR);
	COLORREF crInverse2 = WADlg_getColor(WADLG_SCROLLBAR_INV_BGCOLOR);

	UINT uDEFlat  = sb->fFlatScrollbar ? BF_FLAT   : 0;

	//drawing flags to modify the appearance of the scrollbar buttons
	UINT uLeftButFlags  = DFCS_SCROLLLEFT;
	UINT uRightButFlags = DFCS_SCROLLRIGHT;

	if (scrollwidth <= 0)
		return 0;

	si = &sb->scrollInfo;

	if (hwnd != hwndCurCoolSB)
		uDrawFlags = HTSCROLL_NONE;
	//
	// work out the thumb size and position
	//
	CalcThumbSize(sb, rect, &thumbwidth, &thumbpos);

	if (sb->fScrollFlags & ESB_DISABLE_LEFT)		uLeftButFlags  |= DFCS_INACTIVE;
	if (sb->fScrollFlags & ESB_DISABLE_RIGHT)	uRightButFlags |= DFCS_INACTIVE;

	//if we need to grey the arrows because there is no data to scroll
	if (!IsScrollInfoActive(si) && !(sb->fScrollFlags & CSBS_THUMBALWAYS))
	{
		uLeftButFlags  |= DFCS_INACTIVE;
		uRightButFlags |= DFCS_INACTIVE;
	}

	if (hwnd == hwndCurCoolSB)
	{
		fMouseDownL = (uDrawFlags == HTSCROLL_LEFT);
		fMouseDownR = (uDrawFlags == HTSCROLL_RIGHT);
	}

	//
	// Draw the scrollbar now
	//
	if (scrollwidth > butwidth*2)
	{
		//LEFT ARROW
		SetRect(&ctrl, rect->left, rect->top, rect->left + butwidth, rect->bottom);

		RotateRect0(sb, &ctrl);

		if (fCustomDraw)
			PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &ctrl, SB_LINELEFT, fMouseDownL, fMouseOverL, uLeftButFlags & DFCS_INACTIVE);
		else
			DrawScrollArrow(sb, hdc, &ctrl, uLeftButFlags, fMouseDownL, fMouseOverL);

		RotateRect0(sb, &ctrl);

		//MIDDLE PORTION
		//if we can fit the thumbbar in, then draw it
		if (thumbwidth > 0 && thumbwidth <= workingwidth
		    && IsScrollInfoActive(si) && ((sb->fScrollFlags & ESB_DISABLE_BOTH) != ESB_DISABLE_BOTH))
		{
			//Draw the scrollbar margin above the thumb
			SetRect(&sbm, rect->left + butwidth, rect->top, thumbpos, rect->bottom);

			RotateRect0(sb, &sbm);

			if (fCustomDraw)
			{
				PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &sbm, SB_PAGELEFT, uDrawFlags == HTSCROLL_PAGELEFT, FALSE, FALSE);
			}
			else
			{
				if (uDrawFlags == HTSCROLL_PAGELEFT)
					DrawCheckedRect(hdc, &sbm, crInverse1, crInverse2);
				else
					DrawCheckedRect(hdc, &sbm, crCheck1, crCheck2);

			}

			RotateRect0(sb, &sbm);

			//Draw the margin below the thumb
			sbm.left = thumbpos + thumbwidth;
			sbm.right = rect->right - butwidth;

			RotateRect0(sb, &sbm);
			if (fCustomDraw)
			{
				PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &sbm, SB_PAGERIGHT, uDrawFlags == HTSCROLL_PAGERIGHT, 0, 0);
			}
			else
			{
				if (uDrawFlags == HTSCROLL_PAGERIGHT)
					DrawCheckedRect(hdc, &sbm, crInverse1, crInverse2);
				else
					DrawCheckedRect(hdc, &sbm, crCheck1, crCheck2);

			}
			RotateRect0(sb, &sbm);

			//Draw the THUMB finally
			SetRect(&thumb, thumbpos, rect->top, thumbpos + thumbwidth, rect->bottom);

			RotateRect0(sb, &thumb);

			if (fCustomDraw)
			{
				PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &thumb, SB_THUMBTRACK, uDrawFlags == HTSCROLL_THUMB, uHitTestPortion == HTSCROLL_THUMB, FALSE);
			}
			else
			{
				int track = 0;
				if (uCurrentScrollbar == (UINT)sb->nBarType) track = GetScrollWndFromHwnd(hwnd)->fThumbTracking;
				DrawBlankButton(hdc, &thumb, uDEFlat, track, sb->nBarType == SB_VERT);
			}
			RotateRect0(sb, &thumb);

		}
		//otherwise, just leave that whole area blank
		else
		{
			OffsetRect(&ctrl, butwidth, 0);
			ctrl.right = rect->right - butwidth;

			//if we always show the thumb covering the whole scrollbar,
			//then draw it that way
			if (!IsScrollInfoActive(si)	&& (sb->fScrollFlags & CSBS_THUMBALWAYS)
			    && ctrl.right - ctrl.left > sb->nMinThumbSize)
			{
				//leave a 1-pixel gap between the thumb + right button
				ctrl.right --;
				RotateRect0(sb, &ctrl);

				if (fCustomDraw)
					PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &ctrl, SB_THUMBTRACK, fMouseDownL, FALSE, FALSE);
				else
				{
					DrawBlankButton(hdc, &ctrl, uDEFlat, 0, sb->nBarType == SB_VERT);

				}
				RotateRect0(sb, &ctrl);

				//draw the single-line gap
				ctrl.left = ctrl.right;
				ctrl.right += 1;

				RotateRect0(sb, &ctrl);

				if (fCustomDraw)
					PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &ctrl, SB_PAGERIGHT, 0, 0, 0);
				else
					PaintRect(hdc, &ctrl, GetSysColor(COLOR_SCROLLBAR));

				RotateRect0(sb, &ctrl);
			}
			//otherwise, paint a blank if the thumb doesn't fit in
			else
			{
				RotateRect0(sb, &ctrl);

				if (fCustomDraw)
					PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &ctrl, SB_PAGERIGHT, 0, 0, 0);
				else
					DrawCheckedRect(hdc, &ctrl, crCheck1, crCheck2);

				RotateRect0(sb, &ctrl);
			}
		}

		//RIGHT ARROW
		SetRect(&ctrl, rect->right - butwidth, rect->top, rect->right, rect->bottom);

		RotateRect0(sb, &ctrl);

		if (fCustomDraw)
			PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &ctrl, SB_LINERIGHT, fMouseDownR, fMouseOverR, uRightButFlags & DFCS_INACTIVE);
		else
			DrawScrollArrow(sb, hdc, &ctrl, uRightButFlags, fMouseDownR, fMouseOverR);

		RotateRect0(sb, &ctrl);
	}
	//not enough room for the scrollbar, so just draw the buttons (scaled in size to fit)
	else
	{
		butwidth = scrollwidth / 2;

		//LEFT ARROW
		SetRect(&ctrl, rect->left, rect->top, rect->left + butwidth, rect->bottom);

		RotateRect0(sb, &ctrl);
		if (fCustomDraw)
			PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &ctrl, SB_LINELEFT, fMouseDownL, fMouseOverL, uLeftButFlags & DFCS_INACTIVE);
		else
			DrawScrollArrow(sb, hdc, &ctrl, uLeftButFlags, fMouseDownL, fMouseOverL);
		RotateRect0(sb, &ctrl);

		//RIGHT ARROW
		OffsetRect(&ctrl, scrollwidth - butwidth, 0);

		RotateRect0(sb, &ctrl);
		if (fCustomDraw)
			PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &ctrl, SB_LINERIGHT, fMouseDownR, fMouseOverR, uRightButFlags & DFCS_INACTIVE);
		else
			DrawScrollArrow(sb, hdc, &ctrl, uRightButFlags, fMouseDownR, fMouseOverR);
		RotateRect0(sb, &ctrl);

		//if there is a gap between the buttons, fill it with a solid color
		//if(butwidth & 0x0001)
		if (ctrl.left != rect->left + butwidth)
		{
			ctrl.left --;
			ctrl.right -= butwidth;
			RotateRect0(sb, &ctrl);

			if (fCustomDraw)
				PostCustomDrawNotify(hwnd, hdc, sb->nBarType, &ctrl, SB_PAGERIGHT, 0, 0, 0);
			else
				DrawCheckedRect(hdc, &ctrl, crCheck1, crCheck2);

			RotateRect0(sb, &ctrl);
		}

	}

	return fCustomDraw;
}

//
//	Draw a vertical scrollbar using the horizontal draw routine, but
//	with the coordinates adjusted accordingly
//
static LRESULT NCDrawVScrollbar(SCROLLBAR *sb, HWND hwnd, HDC hdc, const RECT *rect, UINT uDrawFlags)
{
	LRESULT ret;
	RECT rc;

	rc = *rect;
	RotateRect(&rc);
	ret = NCDrawHScrollbar(sb, hwnd, hdc, &rc, uDrawFlags);
	RotateRect(&rc);

	return ret;
}

//
//	Generic wrapper function for the scrollbar drawing
//
static LRESULT NCDrawScrollbar(SCROLLBAR *sb, HWND hwnd, HDC hdc, const RECT *rect, UINT uDrawFlags)
{
	if (sb->nBarType == SB_HORZ)
		return NCDrawHScrollbar(sb, hwnd, hdc, rect, uDrawFlags);
	else
		return NCDrawVScrollbar(sb, hwnd, hdc, rect, uDrawFlags);
}

//
//	Define these two for proper processing of NCPAINT
//	NOT needed if we don't bother to mask the scrollbars we draw
//	to prevent the old window procedure from accidently drawing over them
//
HDC CoolSB_GetDC(HWND hwnd, WPARAM wParam)
{
	// I just can't figure out GetDCEx, so I'll just use this:
	return GetWindowDC(hwnd);
}

static LRESULT NCPaint(ScrollWnd *sw, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLBAR *sb;
	HDC hdc;
	HRGN hrgn;
	RECT winrect, rect;
	HRGN clip;
	BOOL fUpdateAll = ((LONG)wParam == 1);
	BOOL fCustomDraw = FALSE;
	LRESULT ret;
	DWORD dwStyle;

	GetWindowRect(hwnd, &winrect);

	//if entire region needs painting, then make a region to cover the entire window
	hrgn = (HRGN)wParam;

	//hdc = GetWindowDC(hwnd);
	hdc = CoolSB_GetDC(hwnd, wParam);

	//
	//	Only draw the horizontal scrollbar if the window is tall enough
	//
	sb = &sw->sbarHorz;
	if (sb->fScrollVisible)
	{
		//get the screen coordinates of the whole horizontal scrollbar area
		GetHScrollRect(sw, hwnd, &rect);

		//make the coordinates relative to the window for drawing
		OffsetRect(&rect, -winrect.left, -winrect.top);

		if (uCurrentScrollbar == SB_HORZ)
			fCustomDraw |= NCDrawHScrollbar(sb, hwnd, hdc, &rect, uScrollTimerPortion);
		else
			fCustomDraw |= NCDrawHScrollbar(sb, hwnd, hdc, &rect, HTSCROLL_NONE);
	}

	//
	// Only draw the vertical scrollbar if the window is wide enough to accomodate it
	//
	sb = &sw->sbarVert;
	if (sb->fScrollVisible)
	{
		//get the screen cooridinates of the whole horizontal scrollbar area
		GetVScrollRect(sw, hwnd, &rect);

		//make the coordinates relative to the window for drawing
		OffsetRect(&rect, -winrect.left, -winrect.top);

		if (uCurrentScrollbar == SB_VERT)
			fCustomDraw |= NCDrawVScrollbar(sb, hwnd, hdc, &rect, uScrollTimerPortion);
		else
			fCustomDraw |= NCDrawVScrollbar(sb, hwnd, hdc, &rect, HTSCROLL_NONE);
	}

	//Call the default window procedure for WM_NCPAINT, with the
	//new window region. ** region must be in SCREEN coordinates **
	dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);

	// If the window has WS_(H-V)SCROLL bits set, we should reset them
	// to avoid windows taking the scrollbars into account.
	// We temporarily set a flag preventing the subsecuent
	// WM_STYLECHANGING/WM_STYLECHANGED to be forwarded to
	// the original window procedure
	if (dwStyle & (WS_VSCROLL | WS_HSCROLL))
	{
		sw->bPreventStyleChange = TRUE;
		SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle & ~(WS_VSCROLL | WS_HSCROLL));
	}

	ret = (sw->fWndUnicode) ? CallWindowProcW(sw->oldproc, hwnd, WM_NCPAINT, (WPARAM)hrgn, lParam) : 
									CallWindowProcA(sw->oldproc, hwnd, WM_NCPAINT, (WPARAM)hrgn, lParam);
	if (dwStyle & (WS_VSCROLL | WS_HSCROLL))
	{
		SetWindowLong(hwnd, GWL_STYLE, dwStyle);
		sw->bPreventStyleChange = FALSE;
	}


	// DRAW THE DEAD AREA
	// only do this if the horizontal and vertical bars are visible
	if (sw->sbarHorz.fScrollVisible && sw->sbarVert.fScrollVisible)
	{
		GetWindowRect(hwnd, &rect);
		OffsetRect(&rect, -winrect.left, -winrect.top);

		rect.bottom -= sw->cyBottomEdge;
		rect.top  = rect.bottom - GetScrollMetric(&sw->sbarHorz, SM_CYHORZSB);

		if (sw->fLeftScrollbar)
		{
			rect.left += sw->cxLeftEdge;
			rect.right = rect.left + GetScrollMetric(&sw->sbarVert, SM_CXVERTSB);
		}
		else
		{
			rect.right -= sw->cxRightEdge;
			rect.left = rect.right  - GetScrollMetric(&sw->sbarVert, SM_CXVERTSB);
		}

		if (fCustomDraw)
			PostCustomDrawNotify(hwnd, hdc, SB_BOTH, &rect, 32, 0, 0, 0);
		else
		{
			//calculate the position of THIS window's dead area
			//with the position of the PARENT window's client rectangle.
			//if THIS window has been positioned such that its bottom-right
			//corner sits in the parent's bottom-right corner, then we should
			//show the sizing-grip.
			//Otherwise, assume this window is not in the right place, and
			//just draw a blank rectangle
			RECT parent;
			RECT rect2;
			HWND hwndParent = GetParent(hwnd);

			GetClientRect(hwndParent, &parent);
			MapWindowPoints(hwndParent, 0, (POINT *)&parent, 2);

			CopyRect(&rect2, &rect);
			OffsetRect(&rect2, winrect.left, winrect.top);

			if (!sw->fLeftScrollbar && parent.right == rect2.right + sw->cxRightEdge && parent.bottom == rect2.bottom + sw->cyBottomEdge
			    || sw->fLeftScrollbar && parent.left  == rect2.left - sw->cxLeftEdge  && parent.bottom == rect2.bottom + sw->cyBottomEdge)
				drawFrameControl(hdc, &rect, DFC_SCROLL, sw->fLeftScrollbar ? DFCS_SCROLLSIZEGRIPRIGHT : DFCS_SCROLLSIZEGRIP);
			else
				PaintRect(hdc, &rect, WADlg_getColor(WADLG_SCROLLBAR_DEADAREA_COLOR));
		}
	}

	UNREFERENCED_PARAMETER(clip);

	ReleaseDC(hwnd, hdc);
	return ret;
}

//
// Need to detect if we have clicked in the scrollbar region or not
//
static LRESULT NCHitTest(ScrollWnd *sw, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	RECT hrect;
	RECT vrect;
	POINT pt;

	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);

	//work out exactly where the Horizontal and Vertical scrollbars are
	GetHScrollRect(sw, hwnd, &hrect);
	GetVScrollRect(sw, hwnd, &vrect);

	//Clicked in the horizontal scrollbar area
	if (sw->sbarHorz.fScrollVisible && PtInRect(&hrect, pt))
	{
		return HTHSCROLL;
	}
	//Clicked in the vertical scrollbar area
	else if (sw->sbarVert.fScrollVisible && PtInRect(&vrect, pt))
	{
		return HTVSCROLL;
	}
	//clicked somewhere else
	else
	{
		return (sw->fWndUnicode) ? CallWindowProcW(sw->oldproc, hwnd, WM_NCHITTEST, wParam, lParam) : 
									CallWindowProcA(sw->oldproc, hwnd, WM_NCHITTEST, wParam, lParam);

	}
}

//
//	Return a HT* value indicating what part of the scrollbar was clicked
//	Rectangle is not adjusted
//
static UINT GetHorzPortion(SCROLLBAR *sb, HWND hwnd, RECT *rect, int x, int y)
{
	RECT rc = *rect;

	if (y < rc.top || y >= rc.bottom) return HTSCROLL_NONE;

	//Now we have the rectangle for the scrollbar itself, so work out
	//what part we clicked on.
	return GetHorzScrollPortion(sb, hwnd, &rc, x, y);
}

//
//	Just call the horizontal version, with adjusted coordinates
//
static UINT GetVertPortion(SCROLLBAR *sb, HWND hwnd, RECT *rect, int x, int y)
{
	UINT ret;
	RotateRect(rect);
	ret = GetHorzPortion(sb, hwnd, rect, y, x);
	RotateRect(rect);
	return ret;
}

//
//	Wrapper function for GetHorzPortion and GetVertPortion
//
static UINT GetPortion(SCROLLBAR *sb, HWND hwnd, RECT *rect, int x, int y)
{
	if (sb->nBarType == SB_HORZ)
		return GetHorzPortion(sb, hwnd, rect, x, y);
	else if (sb->nBarType == SB_VERT)
		return GetVertPortion(sb, hwnd, rect, x, y);
	else
		return HTSCROLL_NONE;
}

//
//	Input: rectangle of the total scrollbar area
//	Output: adjusted to take the inserted buttons into account
//
static void GetRealHorzScrollRect(SCROLLBAR *sb, RECT *rect)
{
	if (sb->fButVisibleBefore) rect->left += sb->nButSizeBefore;
	if (sb->fButVisibleAfter)  rect->right -= sb->nButSizeAfter;
}

//
//	Input: rectangle of the total scrollbar area
//	Output: adjusted to take the inserted buttons into account
//
static void GetRealVertScrollRect(SCROLLBAR *sb, RECT *rect)
{
	if (sb->fButVisibleBefore) rect->top += sb->nButSizeBefore;
	if (sb->fButVisibleAfter)  rect->bottom -= sb->nButSizeAfter;
}

//
//	Decide which type of scrollbar we have before calling
//  the real function to do the job
//
static void GetRealScrollRect(SCROLLBAR *sb, RECT *rect)
{
	if (sb->nBarType == SB_HORZ)
	{
		GetRealHorzScrollRect(sb, rect);
	}
	else if (sb->nBarType == SB_VERT)
	{
		GetRealVertScrollRect(sb, rect);
	}
}

//
//	Left button click in the non-client area
//
static LRESULT NCLButtonDown(ScrollWnd *sw, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	RECT rect, winrect;
	HDC hdc;
	SCROLLBAR *sb;
	POINT pt;

	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);

	hwndCurCoolSB = hwnd;

	//
	//	HORIZONTAL SCROLLBAR PROCESSING
	//
	if (wParam == HTHSCROLL)
	{
		uScrollTimerMsg = WM_HSCROLL;
		uCurrentScrollbar = SB_HORZ;
		sb = &sw->sbarHorz;

		//get the total area of the normal Horz scrollbar area
		GetHScrollRect(sw, hwnd, &rect);
		uCurrentScrollPortion = GetHorzPortion(sb, hwnd, &rect, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}
	//
	//	VERTICAL SCROLLBAR PROCESSING
	//
	else if (wParam == HTVSCROLL)
	{
		uScrollTimerMsg = WM_VSCROLL;
		uCurrentScrollbar = SB_VERT;
		sb = &sw->sbarVert;

		//get the total area of the normal Horz scrollbar area
		GetVScrollRect(sw, hwnd, &rect);
		uCurrentScrollPortion = GetVertPortion(sb, hwnd, &rect, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}
	//
	//	NORMAL PROCESSING
	//
	else
	{
		uCurrentScrollPortion = HTSCROLL_NONE;
		return (sw->fWndUnicode) ? CallWindowProcW(sw->oldproc, hwnd, WM_NCLBUTTONDOWN, wParam, lParam) : 
									CallWindowProcA(sw->oldproc, hwnd, WM_NCLBUTTONDOWN, wParam, lParam);
	}

	//
	// we can now share the same code for vertical
	// and horizontal scrollbars
	//
	switch (uCurrentScrollPortion)
	{
			//inserted buttons to the left/right
		case HTSCROLL_THUMB:

			//if the scrollbar is disabled, then do no further processing
			if (!IsScrollbarActive(sb))
				return 0;

			GetRealScrollRect(sb, &rect);
			RotateRect0(sb, &rect);
			CalcThumbSize(sb, &rect, &nThumbSize, &nThumbPos);
			RotateRect0(sb, &rect);

			//remember the bounding rectangle of the scrollbar work area
			rcThumbBounds = rect;

			sw->fThumbTracking = TRUE;
			sb->scrollInfo.nTrackPos = sb->scrollInfo.nPos;

			if (wParam == HTVSCROLL)
				nThumbMouseOffset = pt.y - nThumbPos;
			else
				nThumbMouseOffset = pt.x - nThumbPos;

			nLastPos = -sb->scrollInfo.nPos;
			nThumbPos0 = nThumbPos;

			//if(sb->fFlatScrollbar)
			//{
			GetWindowRect(hwnd, &winrect);
			OffsetRect(&rect, -winrect.left, -winrect.top);
			hdc = GetWindowDC(hwnd);
			NCDrawScrollbar(sb, hwnd, hdc, &rect, HTSCROLL_THUMB);
			ReleaseDC(hwnd, hdc);
			//}

			break;

			//Any part of the scrollbar
		case HTSCROLL_LEFT:
			if (sb->fScrollFlags & ESB_DISABLE_LEFT)		return 0;
			else										goto target1;

		case HTSCROLL_RIGHT:
			if (sb->fScrollFlags & ESB_DISABLE_RIGHT)	return 0;
			else										goto target1;

			goto target1;

	case HTSCROLL_PAGELEFT:  case HTSCROLL_PAGERIGHT:

target1:

			//if the scrollbar is disabled, then do no further processing
			if (!IsScrollbarActive(sb))
				break;

			//ajust the horizontal rectangle to NOT include
			//any inserted buttons
			GetRealScrollRect(sb, &rect);

			SendScrollMessage(hwnd, uScrollTimerMsg, uCurrentScrollPortion, 0);

			// Check what area the mouse is now over :
			// If the scroll thumb has moved under the mouse in response to
			// a call to SetScrollPos etc, then we don't hilight the scrollbar margin
			if (uCurrentScrollbar == SB_HORZ)
				uScrollTimerPortion = GetHorzScrollPortion(sb, hwnd, &rect, pt.x, pt.y);
			else
				uScrollTimerPortion = GetVertScrollPortion(sb, hwnd, &rect, pt.x, pt.y);

			GetWindowRect(hwnd, &winrect);
			OffsetRect(&rect, -winrect.left, -winrect.top);
			hdc = GetWindowDC(hwnd);

			//if we aren't hot-tracking, then don't highlight
			//the scrollbar thumb unless we click on it
			if (uScrollTimerPortion == HTSCROLL_THUMB)
				uScrollTimerPortion = HTSCROLL_NONE;
			NCDrawScrollbar(sb, hwnd, hdc, &rect, uScrollTimerPortion);
			ReleaseDC(hwnd, hdc);

			//Post the scroll message!!!!
			uScrollTimerPortion = uCurrentScrollPortion;

			//set a timer going on the first click.
			//if this one expires, then we can start off a more regular timer
			//to generate the auto-scroll behaviour
			uScrollTimerId = SetTimer(hwnd, COOLSB_TIMERID1, COOLSB_TIMERINTERVAL1, 0);
			sw->update();
			break;
		default:
			return (sw->fWndUnicode) ? CallWindowProcW(sw->oldproc, hwnd, WM_NCLBUTTONDOWN, wParam, lParam) : 
									CallWindowProcA(sw->oldproc, hwnd, WM_NCLBUTTONDOWN, wParam, lParam);
			//return 0;
	}

	SetCapture(hwnd);
	return 0;
}

//
//	Left button released
//
static LRESULT LButtonUp(ScrollWnd *sw, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	POINT pt;
	RECT winrect;

	//current scrollportion is the button that we clicked down on
	if (uCurrentScrollPortion != HTSCROLL_NONE)
	{
		SCROLLBAR *sb = &sw->sbarHorz;
		lParam = GetMessagePos();
		ReleaseCapture();

		GetWindowRect(hwnd, &winrect);
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);

		//emulate the mouse input on a scrollbar here...
		if (uCurrentScrollbar == SB_HORZ)
		{
			//get the total area of the normal Horz scrollbar area
			sb = &sw->sbarHorz;
			GetHScrollRect(sw, hwnd, &rect);
		}
		else if (uCurrentScrollbar == SB_VERT)
		{
			//get the total area of the normal Horz scrollbar area
			sb = &sw->sbarVert;
			GetVScrollRect(sw, hwnd, &rect);
		}

		//we need to do different things depending on if the
		//user is activating the scrollbar itself, or one of
		//the inserted buttons
		switch (uCurrentScrollPortion)
		{
				//The scrollbar is active
		case HTSCROLL_LEFT:  case HTSCROLL_RIGHT:
		case HTSCROLL_PAGELEFT:  case HTSCROLL_PAGERIGHT:
			case HTSCROLL_NONE:

				KillTimer(hwnd, uScrollTimerId);

			case HTSCROLL_THUMB:

				sw->update();

				//In case we were thumb tracking, make sure we stop NOW
				if (sw->fThumbTracking == TRUE)
				{
					SendScrollMessage(hwnd, uScrollTimerMsg, SB_THUMBPOSITION, nLastPos);
					sw->fThumbTracking = FALSE;
				}

				//send the SB_ENDSCROLL message now that scrolling has finished
				SendScrollMessage(hwnd, uScrollTimerMsg, SB_ENDSCROLL, 0);

				//adjust the total scroll area to become where the scrollbar
				//really is (take into account the inserted buttons)
				GetRealScrollRect(sb, &rect);
				OffsetRect(&rect, -winrect.left, -winrect.top);
				HDC hdc = GetWindowDC(hwnd);

				//draw whichever scrollbar sb is
				NCDrawScrollbar(sb, hwnd, hdc, &rect, HTSCROLL_NORMAL);

				ReleaseDC(hwnd, hdc);
				break;
		}

		//reset our state to default
		uCurrentScrollPortion = HTSCROLL_NONE;
		uScrollTimerPortion	  = HTSCROLL_NONE;
		uScrollTimerId		  = 0;

		uScrollTimerMsg       = 0;
		uCurrentScrollbar     = COOLSB_NONE;

		return 0;
	}
	else
	{
		/*
		// Can't remember why I did this!
		if(GetCapture() == hwnd)
		{
			ReleaseCapture();
		}*/
	}

	//sw->update();

	return (sw->fWndUnicode) ? CallWindowProcW(sw->oldproc, hwnd, WM_LBUTTONUP, wParam, lParam) : 
									CallWindowProcA(sw->oldproc, hwnd, WM_LBUTTONUP, wParam, lParam);
}

//
//	This function is called whenever the mouse is moved and
//  we are dragging the scrollbar thumb about.
//
static LRESULT ThumbTrackHorz(SCROLLBAR *sbar, HWND hwnd, int x, int y)
{
	POINT pt;
	RECT rc, winrect, rc2;
	COLORREF crCheck1 = GetSBForeColor();
	COLORREF crCheck2 = GetSBBackColor();
	HDC hdc;
	int thumbpos = nThumbPos;
	int pos;
	int siMaxMin = 0;
	UINT flatflag = sbar->fFlatScrollbar ? BF_FLAT : 0;
	BOOL fCustomDraw = FALSE;

	SCROLLINFO *si;
	si = &sbar->scrollInfo;

	pt.x = x;
	pt.y = y;

	//draw the thumb at whatever position
	rc = rcThumbBounds;

	SetRect(&rc2, rc.left -  THUMBTRACK_SNAPDIST*2, rc.top -    THUMBTRACK_SNAPDIST,
	        rc.right + THUMBTRACK_SNAPDIST*2, rc.bottom + THUMBTRACK_SNAPDIST);

	rc.left +=  GetScrollMetric(sbar, SM_CXHORZSB);
	rc.right -= GetScrollMetric(sbar, SM_CXHORZSB);

	//if the mouse is not in a suitable distance of the scrollbar,
	//then "snap" the thumb back to its initial position
#ifdef SNAP_THUMB_BACK
	if (!PtInRect(&rc2, pt))
	{
		thumbpos = nThumbPos0;
	}
	//otherwise, move the thumb to where the mouse is
	else
#endif //SNAP_THUMB_BACK
	{
		//keep the thumb within the scrollbar limits
		thumbpos = pt.x - nThumbMouseOffset;
		if (thumbpos < rc.left) thumbpos = rc.left;
		if (thumbpos > rc.right - nThumbSize) thumbpos = rc.right - nThumbSize;
	}

	GetWindowRect(hwnd, &winrect);

	if (sbar->nBarType == SB_VERT)
		RotateRect(&winrect);

	hdc = GetWindowDC(hwnd);

	OffsetRect(&rc, -winrect.left, -winrect.top);
	thumbpos -= winrect.left;

	//draw the margin before the thumb
	SetRect(&rc2, rc.left, rc.top, thumbpos, rc.bottom);
	RotateRect0(sbar, &rc2);

	if (fCustomDraw)
		PostCustomDrawNotify(hwnd, hdc, sbar->nBarType, &rc2, SB_PAGELEFT, 0, 0, 0);
	else
		DrawCheckedRect(hdc, &rc2, crCheck1, crCheck2);

	RotateRect0(sbar, &rc2);

	//draw the margin after the thumb
	SetRect(&rc2, thumbpos + nThumbSize, rc.top, rc.right, rc.bottom);

	RotateRect0(sbar, &rc2);

	if (fCustomDraw)
		PostCustomDrawNotify(hwnd, hdc, sbar->nBarType, &rc2, SB_PAGERIGHT, 0, 0, 0);
	else
		DrawCheckedRect(hdc, &rc2, crCheck1, crCheck2);

	RotateRect0(sbar, &rc2);

	//finally draw the thumb itelf. This is how it looks on win2000, anyway
	SetRect(&rc2, thumbpos, rc.top, thumbpos + nThumbSize, rc.bottom);

	RotateRect0(sbar, &rc2);

	if (fCustomDraw)
		PostCustomDrawNotify(hwnd, hdc, sbar->nBarType, &rc2, SB_THUMBTRACK, TRUE, TRUE, FALSE);
	else
	{
		DrawBlankButton(hdc, &rc2, flatflag, 1, sbar->nBarType == SB_VERT);
	}

	RotateRect0(sbar, &rc2);
	ReleaseDC(hwnd, hdc);

	//post a SB_TRACKPOS message!!!
	siMaxMin = si->nMax - si->nMin;

	if (siMaxMin > 0)
		pos = MulDiv(thumbpos - rc.left, siMaxMin - si->nPage + 1, rc.right - rc.left - nThumbSize);
	else
		pos = thumbpos - rc.left;

	if (pos != nLastPos)
	{

		if (sbar->flags & SCROLLBAR_LISTVIEW)
		{
		// only for listviews
		if (sbar->nBarType == SB_HORZ)
		{
			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_TRACKPOS;
			if (GetScrollInfo(hwnd, SB_HORZ, &info))
			{
				int nPos = info.nTrackPos;
				SendMessage(hwnd, LVM_SCROLL, pos - nPos, 0);
				SetScrollInfo(hwnd, sbar->nBarType, &info, FALSE);
			}
		}
		else if (sbar->nBarType == SB_VERT)
		{
			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_TRACKPOS;
			if (GetScrollInfo(hwnd, SB_VERT, &info))
			{
				int nPos = info.nTrackPos;
				SendMessage(hwnd, LVM_SCROLL, 0, (pos - nPos)*14); //BIG FUCKO: get the text height size
				SetScrollInfo(hwnd, sbar->nBarType, &info, FALSE);
			}
		}
		}
		else
		{
			si->nTrackPos = pos;
			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_TRACKPOS;
			info.nTrackPos = pos;
	
			SetScrollInfo(hwnd, sbar->nBarType, &info, FALSE);
			SendScrollMessage(hwnd, uScrollTimerMsg, SB_THUMBTRACK, pos);
		}
	}

	nLastPos = pos;

	return 0;
}

//
//	remember to rotate the thumb bounds rectangle!!
//
static LRESULT ThumbTrackVert(SCROLLBAR *sb, HWND hwnd, int x, int y)
{
	//sw->swapcoords = TRUE;
	RotateRect(&rcThumbBounds);
	ThumbTrackHorz(sb, hwnd, y, x);
	RotateRect(&rcThumbBounds);
	//sw->swapcoords = FALSE;

	return 0;
}

//
//	Called when we have set the capture from the NCLButtonDown(...)
//
static LRESULT MouseMove(ScrollWnd *sw, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	POINT pt;
	RECT winrect;

	if (sw->fThumbTracking == TRUE)
	{
		int x, y;
		lParam = GetMessagePos();
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);

		if (uCurrentScrollbar == SB_HORZ)
			return ThumbTrackHorz(&sw->sbarHorz, hwnd, x, y);


		else if (uCurrentScrollbar == SB_VERT)
			return ThumbTrackVert(&sw->sbarVert, hwnd, x, y);
	}

	if (uCurrentScrollPortion == HTSCROLL_NONE)
	{

		return (sw->fWndUnicode) ? CallWindowProcW(sw->oldproc, hwnd, WM_MOUSEMOVE, wParam, lParam) : 
									CallWindowProcA(sw->oldproc, hwnd, WM_MOUSEMOVE, wParam, lParam);
	}
	else
	{
		LPARAM nlParam;
		SCROLLBAR *sb = &sw->sbarHorz;

		nlParam = GetMessagePos();

		GetWindowRect(hwnd, &winrect);

		pt.x = GET_X_LPARAM(nlParam);
		pt.y = GET_Y_LPARAM(nlParam);

		//emulate the mouse input on a scrollbar here...
		if (uCurrentScrollbar == SB_HORZ)
		{
			sb = &sw->sbarHorz;
		}
		else if (uCurrentScrollbar == SB_VERT)
		{
			sb = &sw->sbarVert;
		}

		//get the total area of the normal scrollbar area
		GetScrollRect(sw, sb->nBarType, hwnd, &rect);

		//see if we clicked in the inserted buttons / normal scrollbar
		//thisportion = GetPortion(sb, hwnd, &rect, LOWORD(lParam), HIWORD(lParam));
		UINT thisportion = GetPortion(sb, hwnd, &rect, pt.x, pt.y);

		//we need to do different things depending on if the
		//user is activating the scrollbar itself, or one of
		//the inserted buttons
		static UINT lastportion = 0;
		switch (uCurrentScrollPortion)
		{
				//The scrollbar is active
	case HTSCROLL_LEFT:		 case HTSCROLL_RIGHT: case HTSCROLL_THUMB:
		case HTSCROLL_PAGELEFT:  case HTSCROLL_PAGERIGHT:
			case HTSCROLL_NONE:

				//adjust the total scroll area to become where the scrollbar
				//really is (take into account the inserted buttons)
				GetRealScrollRect(sb, &rect);

				OffsetRect(&rect, -winrect.left, -winrect.top);
				HDC hdc = GetWindowDC(hwnd);

				if (thisportion != uCurrentScrollPortion)
				{
					uScrollTimerPortion = HTSCROLL_NONE;

					if (lastportion != thisportion)
						NCDrawScrollbar(sb, hwnd, hdc, &rect, HTSCROLL_NORMAL);
				}
				//otherwise, draw the button in its depressed / clicked state
				else
				{
					uScrollTimerPortion = uCurrentScrollPortion;

					if (lastportion != thisportion)
						NCDrawScrollbar(sb, hwnd, hdc, &rect, thisportion);
				}

				ReleaseDC(hwnd, hdc);

				break;
		}


		lastportion = thisportion;

		//must return zero here, because we might get cursor anomilies
		//CallWindowProc(sw->oldproc, hwnd, WM_MOUSEMOVE, wParam, lParam);
		return 0;

	}
}

//
//	We must allocate from in the non-client area for our scrollbars
//	Call the default window procedure first, to get the borders (if any)
//	allocated some space, then allocate the space for the scrollbars
//	if they fit
//
static LRESULT NCCalcSize(ScrollWnd *sw, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	NCCALCSIZE_PARAMS *nccsp;
	RECT *rect;
	RECT oldrect;
	//BOOL fCalcValidRects = (wParam == TRUE);
	SCROLLBAR *sb;
	LRESULT ret;
	DWORD dwStyle;

	//Regardless of the value of fCalcValidRects, the first rectangle
	//in the array specified by the rgrc structure member of the
	//NCCALCSIZE_PARAMS structure contains the coordinates of the window,
	//so we can use the exact same code to modify this rectangle, when
	//wParam is TRUE and when it is FALSE.
	nccsp = (NCCALCSIZE_PARAMS *)lParam;
	rect = &nccsp->rgrc[0];
	oldrect = *rect;

	dwStyle = GetWindowLong(hwnd, GWL_STYLE);

	// TURN OFF SCROLL-STYLES.
	if (dwStyle & (WS_VSCROLL | WS_HSCROLL))
	{
		sw->bPreventStyleChange = TRUE;
		SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~(WS_VSCROLL | WS_HSCROLL));
	}

	//call the default procedure to get the borders allocated

	ret = (sw->fWndUnicode) ? CallWindowProcW(sw->oldproc, hwnd, WM_NCCALCSIZE, wParam, lParam) : 
									CallWindowProcA(sw->oldproc, hwnd, WM_NCCALCSIZE, wParam, lParam);

	// RESTORE PREVIOUS STYLES (if present at all)
	if (dwStyle & (WS_VSCROLL | WS_HSCROLL))
	{
		SetWindowLong(hwnd, GWL_STYLE, dwStyle);
		sw->bPreventStyleChange = FALSE;
	}

	// calculate what the size of each window border is,
	sw->cxLeftEdge   = rect->left     - oldrect.left;
	sw->cxRightEdge  = oldrect.right  - rect->right;
	sw->cyTopEdge    = rect->top      - oldrect.top;
	sw->cyBottomEdge = oldrect.bottom - rect->bottom;

	sb = &sw->sbarHorz;

	//if there is room, allocate some space for the horizontal scrollbar
	//NOTE: Change the ">" to a ">=" to make the horz bar totally fill the
	//window before disappearing
	if ((sb->fScrollFlags & CSBS_VISIBLE) &&
#ifdef COOLSB_FILLWINDOW
	    rect->bottom - rect->top >= GetScrollMetric(sb, SM_CYHORZSB))
#else
	    rect->bottom - rect->top > GetScrollMetric(sb, SM_CYHORZSB))
#endif
	{
		rect->bottom -= GetScrollMetric(sb, SM_CYHORZSB);
		sb->fScrollVisible = TRUE;
	}
	else
		sb->fScrollVisible = FALSE;

	sb = &sw->sbarVert;

	//if there is room, allocate some space for the vertical scrollbar
	if ((sb->fScrollFlags & CSBS_VISIBLE) &&
	    rect->right - rect->left >= GetScrollMetric(sb, SM_CXVERTSB))
	{
		if (sw->fLeftScrollbar)
			rect->left  += GetScrollMetric(sb, SM_CXVERTSB);
		else
			rect->right -= GetScrollMetric(sb, SM_CXVERTSB);

		sb->fScrollVisible = TRUE;
	}
	else
		sb->fScrollVisible = FALSE;

	//don't return a value unless we actually modify the other rectangles
	//in the NCCALCSIZE_PARAMS structure. In this case, we return 0
	//no matter what the value of fCalcValidRects is
	return ret;//FALSE;
}

//
//	used for hot-tracking over the scroll buttons
//
static LRESULT NCMouseMove(ScrollWnd *sw, HWND hwnd, WPARAM wHitTest, LPARAM lParam)
{
	//install a timer for the mouse-over events, if the mouse moves
	//over one of the scrollbars
	return (sw->fWndUnicode) ? CallWindowProcW(sw->oldproc, hwnd, WM_NCMOUSEMOVE, wHitTest, lParam) : 
									CallWindowProcA(sw->oldproc, hwnd, WM_NCMOUSEMOVE, wHitTest, lParam);
}

//
//	Timer routine to generate scrollbar messages
//
static LRESULT CoolSB_Timer(ScrollWnd *swnd, HWND hwnd, WPARAM wTimerId, LPARAM lParam)
{
	//let all timer messages go past if we don't have a timer installed ourselves
	if (uScrollTimerId == 0 && uMouseOverId == 0)
	{
		return (swnd->fWndUnicode) ? CallWindowProcW(swnd->oldproc, hwnd, WM_TIMER, wTimerId, lParam) : 
								CallWindowProcA(swnd->oldproc, hwnd, WM_TIMER, wTimerId, lParam);

	}

	//if the first timer goes off, then we can start a more
	//regular timer interval to auto-generate scroll messages
	//this gives a slight pause between first pressing the scroll arrow, and the
	//actual scroll starting
	if (wTimerId == COOLSB_TIMERID1)
	{
		KillTimer(hwnd, uScrollTimerId);
		uScrollTimerId = SetTimer(hwnd, COOLSB_TIMERID2, COOLSB_TIMERINTERVAL2, 0);
		return 0;
	}
	//send the scrollbar message repeatedly
	else if (wTimerId == COOLSB_TIMERID2)
	{
		//need to process a spoof WM_MOUSEMOVE, so that
		//we know where the mouse is each time the scroll timer goes off.
		//This is so we can stop sending scroll messages if the thumb moves
		//under the mouse.
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hwnd, &pt);

		MouseMove(swnd, hwnd, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));

		if (uScrollTimerPortion != HTSCROLL_NONE)
			SendScrollMessage(hwnd, uScrollTimerMsg, uScrollTimerPortion, 0);

		swnd->update();
		return 0;
	}
	else
	{
		return (swnd->fWndUnicode) ? CallWindowProcW(swnd->oldproc, hwnd, WM_TIMER, wTimerId, lParam) : 
								CallWindowProcA(swnd->oldproc, hwnd, WM_TIMER, wTimerId, lParam);
	}
}

//
//	We must intercept any calls to SetWindowLong, to check if
//  left-scrollbars are taking effect or not
//
static LRESULT CoolSB_StyleChange(ScrollWnd *swnd, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	STYLESTRUCT *ss = (STYLESTRUCT *)lParam;

	if (wParam == GWL_EXSTYLE)
	{
		if (ss->styleNew & WS_EX_LEFTSCROLLBAR)
			swnd->fLeftScrollbar = TRUE;
		else
			swnd->fLeftScrollbar = FALSE;
	}

	return (swnd->fWndUnicode) ? CallWindowProcW(swnd->oldproc, hwnd, msg, wParam, lParam) : 
								CallWindowProcA(swnd->oldproc, hwnd, msg, wParam, lParam);
}

static UINT curTool = -1;
static LRESULT CoolSB_Notify(ScrollWnd *swnd, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return (swnd->fWndUnicode) ? CallWindowProcW(swnd->oldproc, hwnd, WM_NOTIFY, wParam, lParam) : 
								CallWindowProcA(swnd->oldproc, hwnd, WM_NOTIFY, wParam, lParam);
}

static LRESULT SendToolTipMessage0(HWND hwndTT, UINT message, WPARAM wParam, LPARAM lParam)
{
	return SendMessage(hwndTT, message, wParam, lParam);
}

#ifdef COOLSB_TOOLTIPS
#define SendToolTipMessage		SendToolTipMessage0
#else
#define SendToolTipMessage		1 ? (void)0 : SendToolTipMessage0
#endif


//
//	We must intercept any calls to SetWindowLong, to make sure that
//	the user does not set the WS_VSCROLL or WS_HSCROLL styles
//
static LRESULT CoolSB_SetCursor(ScrollWnd *swnd, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return (swnd->fWndUnicode) ? CallWindowProcW(swnd->oldproc, hwnd, WM_SETCURSOR, wParam, lParam) : 
								CallWindowProcA(swnd->oldproc, hwnd, WM_SETCURSOR, wParam, lParam);
}

//
//  CoolScrollbar subclass procedure.
//	Handle all messages needed to mimick normal windows scrollbars
//
LRESULT CALLBACK CoolSBWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC oldproc;

	ScrollWnd *swnd = GetScrollWndFromHwnd(hwnd);

	if (!swnd || !swnd->oldproc)
	{
		return (IsWindowUnicode(hwnd)) ? DefWindowProcW(hwnd, message, wParam, lParam) : 
										DefWindowProcA(hwnd, message, wParam, lParam);
	}

	switch (message)
	{
		case WM_NCDESTROY:
			//this should NEVER be called, because the user
			//should have called Uninitialize() themselves.

			//However, if the user tries to call Uninitialize()..
			//after this window is destroyed, this window's entry in the lookup
			//table will not be there, and the call will fail
			oldproc = swnd->oldproc;
			delete(swnd);

			//we must call the original window procedure, otherwise it
			//will never get the WM_NCDESTROY message, and it wouldn't
			//be able to clean up etc.
			return (IsWindowUnicode(hwnd)) ? CallWindowProcW(oldproc, hwnd, message, wParam, lParam) : CallWindowProcA(oldproc, hwnd, message, wParam, lParam);

		case WM_NCCALCSIZE:
			return NCCalcSize(swnd, hwnd, wParam, lParam);

		case WM_NCPAINT:
			return NCPaint(swnd, hwnd, wParam, lParam);

		case WM_NCHITTEST:
			return NCHitTest(swnd, hwnd, wParam, lParam);

	case WM_NCRBUTTONDOWN: case WM_NCRBUTTONUP:
	case WM_NCMBUTTONDOWN: case WM_NCMBUTTONUP:
			if (wParam == HTHSCROLL || wParam == HTVSCROLL)
				return 0;
			else
				break;

		case WM_NCLBUTTONDBLCLK:
			//TRACE("WM_NCLBUTTONDBLCLK %d\n", count++);
			if (wParam == HTHSCROLL || wParam == HTVSCROLL)
				return NCLButtonDown(swnd, hwnd, wParam, lParam);
			else
				break;

		case WM_NCLBUTTONDOWN:
			//TRACE("WM_NCLBUTTONDOWN%d\n", count++);
			return NCLButtonDown(swnd, hwnd, wParam, lParam);


		case WM_LBUTTONUP:
			//TRACE("WM_LBUTTONUP %d\n", count++);
			return LButtonUp(swnd, hwnd, wParam, lParam);

		case WM_NOTIFY:
			return CoolSB_Notify(swnd, hwnd, wParam, lParam);

			//Mouse moves are received when we set the mouse capture,
			//even when the mouse moves over the non-client area
		case WM_MOUSEMOVE:
			//TRACE("WM_MOUSEMOVE %d\n", count++);
			return MouseMove(swnd, hwnd, wParam, lParam);

		case WM_TIMER:
			return CoolSB_Timer(swnd, hwnd, wParam, lParam);

			//case WM_STYLECHANGING:
			//	return CoolSB_StyleChange(swnd, hwnd, WM_STYLECHANGING, wParam, lParam);
		case WM_STYLECHANGED:
			if (swnd->bPreventStyleChange)
			{
				// the NCPAINT handler has told us to eat this message!
				return 0;
			}
			else
			{
				if (message == WM_STYLECHANGED)
					return CoolSB_StyleChange(swnd, hwnd, WM_STYLECHANGED, wParam, lParam);
			}
			break;

		case WM_NCMOUSEMOVE:
		{
			static LPARAM lastpos = -1;

			//TRACE("WM_NCMOUSEMOVE %d\n", count++);

			//The problem with NCMOUSEMOVE is that it is sent continuously
			//even when the mouse is stationary (under win2000 / win98)
			//
			//Tooltips don't like being sent a continous stream of mouse-moves
			//if the cursor isn't moving, because they will think that the mouse
			//is moving position, and the internal timer will never expire
			//
			if (lastpos != lParam)
			{
				lastpos = lParam;
			}
		}

		return NCMouseMove(swnd, hwnd, wParam, lParam);


		case WM_SETCURSOR:
			return CoolSB_SetCursor(swnd, hwnd, wParam, lParam);

		case WM_CAPTURECHANGED:
			break;

		case WM_ERASEBKGND:
			if (swnd && !swnd->fThumbTracking && uCurrentScrollPortion == HTSCROLL_NONE)
			{
				//disable windows scrollbar painting (fixes gfx repainting weirdness)
				int style = GetWindowLong(hwnd, GWL_STYLE);
				if (style&(WS_HSCROLL | WS_VSCROLL))
				{
					SetWindowLong(hwnd, GWL_STYLE, style&~(WS_HSCROLL | WS_VSCROLL));
				}

				LRESULT ret = (swnd->fWndUnicode) ? CallWindowProcW(swnd->oldproc, hwnd, message, wParam, lParam) : 
													CallWindowProcA(swnd->oldproc, hwnd, message, wParam, lParam);
				swnd->update();
				return ret;
			}
			break;

			//needed if we want mousewheel to work properly because we disable the styles in WM_ERASEBKGND...
		case WM_MOUSEWHEEL:
		{
			int style = GetWindowLong(hwnd, GWL_STYLE);
			swnd->bPreventStyleChange = TRUE;
			SetWindowLong(hwnd, GWL_STYLE, style | (swnd->sbarHorz.fScrollVisible ? WS_HSCROLL : 0) | (swnd->sbarVert.fScrollVisible ? WS_VSCROLL : 0));
			LRESULT ret = (swnd->fWndUnicode) ? CallWindowProcW(swnd->oldproc, hwnd, message, wParam, lParam) : 
													CallWindowProcA(swnd->oldproc, hwnd, message, wParam, lParam);
			SetWindowLongPtr(hwnd, GWL_STYLE, style);
			swnd->bPreventStyleChange = FALSE;
			return ret;
		}

		case WM_USER + 0x3443: //manually sent by other windows (like columns header for ex.)
			if (swnd) swnd->update();
			break;

		default:
			break;
	}
	return (swnd->fWndUnicode) ? CallWindowProcW(swnd->oldproc, hwnd, message, wParam, lParam) : 
						CallWindowProcA(swnd->oldproc, hwnd, message, wParam, lParam);
}

SCROLLBAR *GetScrollBarFromHwnd(HWND hwnd, UINT nBar)
{
	ScrollWnd *sw = GetScrollWndFromHwnd(hwnd);

	if (!sw) return 0;

	if (nBar == SB_HORZ)
		return &sw->sbarHorz;
	else if (nBar == SB_VERT)
		return &sw->sbarVert;
	else
		return 0;
}

//
//	return the default minimum size of a scrollbar thumb
//
int WINAPI CoolSB_GetDefaultMinThumbSize(void)
{
	DWORD dwVersion = GetVersion();

	// set the minimum thumb size for a scrollbar. This
	// differs between NT4 and 2000, so need to check to see
	// which platform we are running under
	if (dwVersion < 0x80000000)             // Windows NT/2000
	{
		if (LOBYTE(LOWORD(dwVersion)) >= 5)
			return MINTHUMBSIZE_2000;
		else
			return MINTHUMBSIZE_NT4;
	}
	else
	{
		return MINTHUMBSIZE_NT4;
	}
}

//
//	Set the minimum size, in pixels, that the thumb box will shrink to.
//
BOOL WINAPI CoolSB_SetMinThumbSize(HWND hwnd, UINT wBar, UINT size)
{
	SCROLLBAR *sbar;

	if (!GetScrollWndFromHwnd(hwnd))
		return FALSE;

	if (size == -1)
		size = CoolSB_GetDefaultMinThumbSize();

	if ((wBar == SB_HORZ || wBar == SB_BOTH) &&
	    (sbar = GetScrollBarFromHwnd(hwnd, SB_HORZ)))
	{
		sbar->nMinThumbSize = size;
	}

	if ((wBar == SB_VERT || wBar == SB_BOTH) &&
	    (sbar = GetScrollBarFromHwnd(hwnd, SB_VERT)))
	{
		sbar->nMinThumbSize = size;
	}

	return TRUE;
}

static void RedrawNonClient(HWND hwnd, BOOL fFrameChanged)
{
	if (fFrameChanged == FALSE)
	{
		SendMessage(hwnd, WM_NCPAINT, (WPARAM)1, 0);
	}
	else
	{
		SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
		             | SWP_FRAMECHANGED | SWP_DRAWFRAME);
	}
}

ScrollWnd::ScrollWnd(HWND hwnd, int flags)
{
	SCROLLINFO *si;
	RECT rect;
	DWORD dwCurStyle;

	bars = 0;
	oldproc = NULL;
	memset(&sbarHorz, 0, sizeof(sbarHorz));
	memset(&sbarVert, 0, sizeof(sbarVert));
	sbarHorz.flags = flags;
	sbarVert.flags = flags;
	fThumbTracking = 0;
	fLeftScrollbar = 0;
	cxLeftEdge = cxRightEdge = cyTopEdge = cyBottomEdge = 0;
	bPreventStyleChange = 0;
	m_disable_hscroll = 0;
	m_xp_theme_disabled = 0;

	m_hwnd = hwnd;

	GetClientRect(hwnd, &rect);

	si = &sbarHorz.scrollInfo;
	si->cbSize = sizeof(SCROLLINFO);
	si->fMask  = SIF_ALL;
	GetScrollInfo(hwnd, SB_HORZ, si);

	si = &sbarVert.scrollInfo;
	si->cbSize = sizeof(SCROLLINFO);
	si->fMask  = SIF_ALL;
	GetScrollInfo(hwnd, SB_VERT, si);

	//check to see if the window has left-aligned scrollbars
	if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_LEFTSCROLLBAR)
		fLeftScrollbar = TRUE;
	else
		fLeftScrollbar = FALSE;

	dwCurStyle = GetWindowLong(hwnd, GWL_STYLE);

	SetProp(hwnd, szPropStr, (HANDLE)this);

	//scrollbars will automatically get enabled, even if
	//they aren't to start with....sorry, but there isn't an
	//easy alternative.
	if (dwCurStyle & WS_HSCROLL)
		sbarHorz.fScrollFlags = CSBS_VISIBLE;

	if (dwCurStyle & WS_VSCROLL)
		sbarVert.fScrollFlags = CSBS_VISIBLE;

	//need to be able to distinguish between horizontal and vertical
	//scrollbars in some instances
	sbarHorz.nBarType	     = SB_HORZ;
	sbarVert.nBarType	     = SB_VERT;

	sbarHorz.fFlatScrollbar  = CSBS_NORMAL;
	sbarVert.fFlatScrollbar  = CSBS_NORMAL;

	//set the default arrow sizes for the scrollbars
	sbarHorz.nArrowLength	 = SYSTEM_METRIC;
	sbarHorz.nArrowWidth	 = SYSTEM_METRIC;
	sbarVert.nArrowLength	 = SYSTEM_METRIC;
	sbarVert.nArrowWidth	 = SYSTEM_METRIC;

	bPreventStyleChange		 = FALSE;

	fWndUnicode = IsWindowUnicode(hwnd);
	oldproc = (WNDPROC)(LONG_PTR) ((fWndUnicode) ? SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)CoolSBWndProc) :
														SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)CoolSBWndProc));
	
	CoolSB_SetMinThumbSize(hwnd, SB_BOTH, CoolSB_GetDefaultMinThumbSize());

	//send the window a frame changed message to update the scrollbars
	RedrawNonClient(hwnd, TRUE);

	//disable XP styles
	if (SetWindowTheme && !m_xp_theme_disabled)
	{
		SetWindowTheme(m_hwnd, L" ", L" ");
		m_xp_theme_disabled = 1;

	}

}

BOOL WINAPI CoolSB_ShowScrollBar(HWND hwnd, int wBar, BOOL fShow)
{
	SCROLLBAR *sbar;
	BOOL bFailed = FALSE;
	DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

	if ((wBar == SB_HORZ || wBar == SB_BOTH) &&
	    (sbar = GetScrollBarFromHwnd(hwnd, SB_HORZ)))
	{
		sbar->fScrollFlags  =  sbar->fScrollFlags & ~CSBS_VISIBLE;
		sbar->fScrollFlags |= (fShow == TRUE ? CSBS_VISIBLE : 0);
		//bFailed = TRUE;

		if (fShow)	SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_HSCROLL);
		else		SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_HSCROLL);
	}

	if ((wBar == SB_VERT || wBar == SB_BOTH) &&
	    (sbar = GetScrollBarFromHwnd(hwnd, SB_VERT)))
	{
		sbar->fScrollFlags  =  sbar->fScrollFlags & ~CSBS_VISIBLE;
		sbar->fScrollFlags |= (fShow == TRUE ? CSBS_VISIBLE : 0);
		//bFailed = TRUE;

		if (fShow)	SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_VSCROLL);
		else		SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_VSCROLL);
	}

	if (bFailed)
	{
		return FALSE;
	}
	else
	{
		SetWindowPos(hwnd, 0, 0, 0, 0, 0,
		             SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
		             SWP_NOACTIVATE | SWP_FRAMECHANGED);

		return TRUE;
	}
}

ScrollWnd::~ScrollWnd()
{
	if (oldproc)
	{
		((fWndUnicode) ? SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)oldproc) :
														SetWindowLongPtrA(m_hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)oldproc));
	}
	RemoveProp(m_hwnd, szPropStr);
	RedrawNonClient(m_hwnd, TRUE);
}

void ScrollWnd::update()
{
	int dohorz = 0, dovert = 0;

	SCROLLINFO tsi = {sizeof(SCROLLINFO), SIF_ALL, };

	if (!m_disable_hscroll)
	{
		GetScrollInfo(m_hwnd, SB_HORZ, &tsi);
		if (memcmp(&tsi, &sbarHorz.scrollInfo, sizeof(SCROLLINFO)))
		{
			memcpy(&sbarHorz.scrollInfo, &tsi, sizeof(SCROLLINFO));
			dohorz = 1;
		}
	}
	GetScrollInfo(m_hwnd, SB_VERT, &tsi);
	if (memcmp(&tsi, &sbarVert.scrollInfo, sizeof(SCROLLINFO)))
	{
		memcpy(&sbarVert.scrollInfo, &tsi, sizeof(SCROLLINFO));
		dovert = 1;
	}

	BOOL fRecalcFrame = FALSE;
	if (dohorz) updatesb(SB_HORZ, &fRecalcFrame);
	if (dovert) updatesb(SB_VERT, &fRecalcFrame);

	if (dohorz || dovert) RedrawNonClient(m_hwnd, fRecalcFrame);
}

void ScrollWnd::updatesb(int fnBar, BOOL *fRecalcFrame)
{
	SCROLLBAR *sbar = (fnBar == SB_HORZ ? &sbarHorz : &sbarVert);
	SCROLLINFO *mysi = &sbar->scrollInfo;

	if (mysi->nPage > (UINT)mysi->nMax
	    || (mysi->nPage == (UINT)mysi->nMax && mysi->nMax == 0)
	    || mysi->nMax  <= mysi->nMin)
	{
		if (sbar->fScrollVisible)
		{
			CoolSB_ShowScrollBar(m_hwnd, fnBar, FALSE);
			*fRecalcFrame = TRUE;
		}
	}
	else
	{
		if (!sbar->fScrollVisible && mysi->nPage > 0)
		{
			CoolSB_ShowScrollBar(m_hwnd, fnBar, TRUE);
			*fRecalcFrame = TRUE;
		}
		else if (sbar->fScrollVisible && mysi->nPage == 0)
		{
			CoolSB_ShowScrollBar(m_hwnd, fnBar, FALSE);
			*fRecalcFrame = TRUE;
		}
	}
}

void ScrollWnd::disableHorzScroll()
{
	m_disable_hscroll = 1;
	sbarHorz.fScrollFlags = 0;
}

#if 0 // unused
int ScrollWnd::setScrollInfo(int fnBar, LPSCROLLINFO lpsi, BOOL fRedraw)
{
	SCROLLINFO *mysi;
	SCROLLBAR *sbar;
	BOOL       fRecalcFrame = FALSE;

	if (!lpsi)
		return FALSE;

	if (fnBar == SB_HORZ) mysi = &sbarHorz.scrollInfo;
	else mysi = &sbarVert.scrollInfo;

	if (lpsi->fMask & SIF_RANGE)
	{
		mysi->nMin = lpsi->nMin;
		mysi->nMax = lpsi->nMax;
	}

	//The nPage member must specify a value from 0 to nMax - nMin +1.
	if (lpsi->fMask & SIF_PAGE)
	{
		UINT t = (UINT)(mysi->nMax - mysi->nMin + 1);
		mysi->nPage = min(max(0, lpsi->nPage), t);
	}

	//The nPos member must specify a value between nMin and nMax - max(nPage - 1, 0).
	if (lpsi->fMask & SIF_POS)
	{
		mysi->nPos = max(lpsi->nPos, mysi->nMin);
		mysi->nPos = min((UINT)mysi->nPos, mysi->nMax - max(mysi->nPage - 1, 0));
	}

	sbar = GetScrollBarFromHwnd(m_hwnd, fnBar);

	if ((lpsi->fMask & SIF_DISABLENOSCROLL) || (sbar->fScrollFlags & CSBS_THUMBALWAYS))
	{
		if (!sbar->fScrollVisible)
		{
			CoolSB_ShowScrollBar(m_hwnd, fnBar, TRUE);
			fRecalcFrame = TRUE;
		}
	}
	else
	{
		if (mysi->nPage > (UINT)mysi->nMax
		    || mysi->nPage == (UINT)mysi->nMax && mysi->nMax == 0
		    || mysi->nMax  <= mysi->nMin)
		{
			if (sbar->fScrollVisible)
			{
				CoolSB_ShowScrollBar(m_hwnd, fnBar, FALSE);
				fRecalcFrame = TRUE;
			}
		}
		else
		{
			if (!sbar->fScrollVisible)
			{
				CoolSB_ShowScrollBar(m_hwnd, fnBar, TRUE);
				fRecalcFrame = TRUE;
			}
		}
	}

	if (fRedraw && !fThumbTracking)
		RedrawNonClient(m_hwnd, fRecalcFrame);

	return mysi->nPos;
}
#endif