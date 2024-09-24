#include "main.h"

#include "../Winamp/gen.h"
#include "../gen_ml/ml.h"
#include "../gen_ml/ml_ipc_0313.h"

#define WM_EX_GETREALLIST		(WM_USER + 0x01)
#define WM_EX_UNLOCKREDRAW		(WM_USER + 0x02)
#define WM_EX_UPDATESCROLLINFO	(WM_USER + 0x03)
#define WM_EX_GETCOUNTPERPAGE	(WM_USER + 0x04)

#define LVN_EX_SIZECHANGED		(LVN_LAST)

#define IWF_NORMAL			0x0000
#define IWF_ERASE 			0x0001
#define IWF_UPDATENOW		0x0002
#define IWF_FRAME			0x0004


typedef enum ScrollPosFlags
{
	SPF_NORMAL		=  0,
	SPF_NOREDRAW	= (1 << 0),
	SPF_FORCE		= (1 << 1),
	SPF_RELATIVE	= (1 << 2),
} ScrollPosFlags;
DEFINE_ENUM_FLAG_OPERATORS(ScrollPosFlags);

BOOL
CopyListColumnToHeaderItem(const LVCOLUMNW *column, HDITEMW *item);
BOOL
CopyHeaderItemToListColumn(const HDITEMW *item, LVCOLUMNW *column);


typedef enum PostProcessKeyCommands
{
	PostProcessKeyCmd_Nothing = 0,
	PostProcessKeyCmd_UpdateScrollPos = (1 << 0),
	PostProcessKeyCmd_EnsureFocusVisible = (1 << 1),
} PostProcessKeyCommands;
DEFINE_ENUM_FLAG_OPERATORS(PostProcessKeyCommands);

typedef struct SmoothScrollList 
{
  unsigned int itemHeight;
  unsigned int textHeight;
  long viewHeight;
  unsigned int listFontHeight;
  unsigned int headerFontHeight;
  int wheelCarryover;
} SmoothScrollList;

#define GetUserData(hwnd) ((SmoothScrollList*)(LONG_PTR)GetWindowLongPtrW(hwnd, GWLP_USERDATA))


static LRESULT 
SubclassedListView_CallPrevWndProc(HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
	WNDPROC windowProc;

	windowProc = (WNDPROC)(LONG_PTR)GetWindowLongPtrW(hwnd,GWLP_USERDATA);
	if (NULL == windowProc)
		return DefWindowProcW(hwnd, message, wParam, lParam);

	return CallWindowProcW(windowProc, hwnd, message,wParam,lParam);
}

static BOOL 
GetViewRect(HWND hwnd, RECT *prc)
{
	HWND headerWindow;
	GetClientRect(hwnd, prc);
	headerWindow = GetDlgItem(hwnd, 3);
	if (NULL != headerWindow) 
	{
		RECT rh;
		GetWindowRect(headerWindow, &rh);
		MapWindowPoints(HWND_DESKTOP,  headerWindow, ((POINT*)&rh) + 1, 1);
		prc->top = rh.bottom;
	}

	if (prc->right < prc->left)
		prc->right = prc->left;

	if (prc->bottom < prc->top)
		prc->bottom = prc->top;
	
	return TRUE;
}

static int 
SmoothScrollList_GetScrollPosFromItem(HWND hwnd, int iItem)
{
	HWND listWindow;
	RECT listRect, viewRect;
	int pos;
	int count;

	pos = 0;
		
	if (FALSE == GetViewRect(hwnd, &viewRect))
		return 0;

	listWindow = GetDlgItem(hwnd, 2);
	if (NULL == listWindow || 
		FALSE == GetWindowRect(listWindow, &listRect))
	{
		return 0;
	}

	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&listRect, 2);

	count = (int)SendMessageW(listWindow, LVM_GETITEMCOUNT, 0, 0L);
	if (0 != count)
	{
		SmoothScrollList *self;

		if (iItem < 0)
			iItem = 0;
		
		if (iItem >= count)
			iItem = count - 1;

		self = GetUserData(hwnd);
		if (NULL != self)
			pos = iItem * self->itemHeight;
	}
		
	pos += (viewRect.top - listRect.top);
	return pos;
}

static BOOL 
UpdateScrollInfo(HWND hwndView, UINT fFlags, BOOL bRedraw)
{
	RECT rv;
	HWND hwndList;
	SCROLLINFO si;
	BOOL needUpdate;
	BOOL needRedraw;
	HRGN regionUpdate, regionTemp;

	SmoothScrollList* s = GetUserData(hwndView);

	hwndList = GetDlgItem(hwndView, 2);
	if (!s || !hwndList ) return FALSE;

	if (FALSE!= bRedraw)
	{
		GetWindowRect(hwndView, &rv);
		MapWindowPoints(HWND_DESKTOP, hwndView, (POINT*)&rv, 2);
		regionUpdate = CreateRectRgnIndirect(&rv);
		GetClientRect(hwndView, &rv);
		regionTemp = CreateRectRgnIndirect(&rv);
		CombineRgn(regionUpdate, regionUpdate, regionTemp, RGN_DIFF);
	}
	else
	{
		regionUpdate = NULL;
		regionTemp = NULL;
	}

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	
	if (!GetScrollInfo(hwndView, SB_VERT, &si)) 
		return FALSE;
	
	if (FALSE == GetViewRect(hwndView, &rv))
		SetRectEmpty(&rv);

	needUpdate = FALSE;
	needRedraw = FALSE;
	
	if (SIF_RANGE & fFlags)
	{
		unsigned int count, nPage, nMax;

		nPage = rv.bottom - rv.top;

		count = (INT)SendMessageW(hwndList, LVM_GETITEMCOUNT, 0, 0L);
		nMax = count * s->itemHeight;

		if (si.nPage != nPage || si.nMax != nMax)
		{
			BOOL forcePos;
			unsigned int windowStyle;

			si.fMask = SIF_PAGE | SIF_RANGE;
			si.nPage = nPage;
			si.nMax = nMax;

			windowStyle = GetWindowLongPtrW(hwndView, GWL_STYLE);

			SetScrollInfo(hwndView, SB_VERT, &si, FALSE);

			needUpdate = TRUE;
			needRedraw = FALSE;
			forcePos = FALSE;

			if (nPage >= nMax && 
				0 != (WS_VSCROLL & windowStyle))
			{
				SetWindowLongPtrW(hwndView, GWL_STYLE, windowStyle & ~WS_VSCROLL);

				RECT rc;
				HWND hwndHeader;
			//	MLSkinnedScrollWnd_UpdateBars(hwndView, bRedraw);
				GetClientRect(hwndView, &rc);
				hwndHeader = GetDlgItem(hwndView, 3);
				if (hwndHeader) 
				{
					HDLAYOUT headerLayout;
					WINDOWPOS headerPos;

					headerLayout.prc = &rc;
					headerLayout.pwpos = &headerPos;
					
					if (FALSE != SendMessageW(hwndHeader, HDM_LAYOUT, 0, (LPARAM)&headerLayout))
					{
						headerPos.flags |= SWP_NOREDRAW | SWP_NOCOPYBITS;
						headerPos.flags &= ~SWP_NOZORDER;
						headerPos.hwndInsertAfter = HWND_TOP;
						SetWindowPos(hwndHeader, headerPos.hwndInsertAfter, headerPos.x, headerPos.y, 
									headerPos.cx, headerPos.cy, headerPos.flags);

						InvalidateRect(hwndHeader, NULL, FALSE);
					}
				}
				rv.right = rc.right;
				forcePos = TRUE;
				needUpdate = FALSE;
				needRedraw  = TRUE;
			}

			if (nPage >= nMax || forcePos)
			{
				RECT rl;
				GetWindowRect(hwndList, &rl);
				MapWindowPoints(HWND_DESKTOP, hwndView, (POINT*)&rl, 2);
				if (rv.top != rl.top || forcePos)
				{
					SetWindowPos(hwndList, NULL, rv.left, rv.top, rv.right - rv.left, rv.bottom - rv.top, 
										SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS); 
					needRedraw  = TRUE;
				}
			}
		}
	}
	if (SIF_POS & fFlags)
	{
		INT nTop;

		nTop = (INT)SendMessageW(hwndList, LVM_GETTOPINDEX, 0, 0L);
		nTop = SmoothScrollList_GetScrollPosFromItem(hwndView, nTop);

		if(si.nMax > 0)
		{
			if (nTop >= (si.nMax - (int)si.nPage)) 
				nTop = (si.nMax - si.nPage) + 1;
		}
		else
			nTop = 0;

		if (nTop < si.nMin)
			nTop = si.nMin;
		
		if (si.nPos != nTop)
		{
			si.fMask = SIF_POS;
			si.nPos = nTop;
			
			SetScrollInfo(hwndView, SB_VERT, &si, (FALSE == needRedraw && FALSE != bRedraw));

			needUpdate = TRUE;
		}
	}

	if (FALSE != needUpdate) 
		MLSkinnedScrollWnd_UpdateBars(hwndView, (FALSE == needRedraw && FALSE != bRedraw));

	if (FALSE != bRedraw && FALSE != needRedraw)
	{
		HRGN regionTemp2;
		GetWindowRect(hwndView, &rv);
		MapWindowPoints(HWND_DESKTOP, hwndView, (POINT*)&rv, 2);
		SetRectRgn(regionTemp, rv.left, rv.top, rv.right, rv.bottom);
		GetClientRect(hwndView, &rv);
		regionTemp2 = CreateRectRgnIndirect(&rv);
		CombineRgn(regionTemp, regionTemp, regionTemp2, RGN_DIFF);
		CombineRgn(regionUpdate, regionUpdate, regionTemp, RGN_OR);
		DeleteObject(regionTemp2);

		RedrawWindow(hwndView, NULL, regionUpdate, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME);
	}

	if (NULL != regionUpdate)
		DeleteObject(regionUpdate);
	if (NULL != regionTemp)
		DeleteObject(regionTemp);

	return TRUE;
}

static BOOL 
SmoothScrollList_SetScrollPos(HWND hwnd, int position, ScrollPosFlags flags)
{
	SmoothScrollList *self;
	HWND listWindow;
	BOOL invalidate, failed;
	unsigned long viewStyle;
	int y, scrollPos;
	RECT rv, rl;
	SCROLLINFO si;
	

	listWindow = GetDlgItem(hwnd, 2);
	if (NULL == listWindow)
		return FALSE;

	self = GetUserData(hwnd);
	if (NULL == self)
		return FALSE;
	
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	if (FALSE == GetScrollInfo(hwnd, SB_VERT, &si)) 
		return FALSE;

	scrollPos = si.nPos;
	
	viewStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
	
	invalidate = FALSE;
	failed = FALSE;
	y = 0;

	if (0 != (SPF_RELATIVE & flags))
	{		
		position = si.nPos + position;
		if (si.nPos > (si.nMax - (int)si.nPage))
			position -= (si.nPos - (si.nMax - (int)si.nPage));
	}

	if (position < si.nMin) 
		position = si.nMin;
	
	if (position > (si.nMax - (INT)si.nPage + 1)) 
		position = si.nMax - si.nPage + 1;
	
	if (position == si.nPos && 0 == (SPF_FORCE & flags)) 
		return TRUE;

	if (FALSE == GetViewRect(hwnd, &rv))
		SetRectEmpty(&rv);

	if (FALSE == GetWindowRect(listWindow, &rl))
		SetRectEmpty(&rl);

	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rl, 2);

	if (0 != (WS_VISIBLE & viewStyle))
		SetWindowLongPtrW(hwnd, GWL_STYLE, viewStyle & ~WS_VISIBLE);
		
	if (si.nMin == position) 
	{
		if (rl.top != rv.top || rl.bottom != rv.bottom || rl.left != rv.left || rl.right != rv.right)
		{
			SetWindowPos(listWindow, NULL, rv.left, rv.top, rv.right - rv.left, rv.bottom - rv.top, 
								SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS); 
			invalidate = TRUE;
		}

		if (0 != (int)SendMessageW(listWindow, LVM_GETITEMCOUNT, 0, 0L))
		{
			RECT rect;
			rect.left = LVIR_BOUNDS;
			if (FALSE != SendMessageW(listWindow, LVM_GETITEMRECT, 0, (LPARAM)&rect) &&
				0 != rect.top)
			{
				int scrollY;
				scrollY = rect.top;
				failed = !SendMessageW(listWindow, LVM_SCROLL, 0, scrollY);
				invalidate = TRUE;
			}
		}
	}
	else 
	{
		int iTop, iPos;
		
		iTop = (int)SendMessageW(listWindow, LVM_GETTOPINDEX, 0, 0L);
		if (position > (si.nMax  - (int)si.nPage))
			position = (si.nMax - si.nPage);
		
		iPos = position/self->itemHeight;
		y = (position - iPos*self->itemHeight);

		if (iTop > iPos)
		{
			failed = !SendMessageW(listWindow, LVM_SCROLL, 0, (iPos - iTop) * self->itemHeight);
			invalidate  = TRUE;
		}

		if (rl.top != rv.top + y || rl.bottom != rv.bottom || rl.left != rv.left || rl.right != rv.right)
		{
			SetWindowPos(listWindow, NULL, rv.left, rv.top - y, rv.right - rv.left, rv.bottom - rv.top + y, 
							SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			invalidate  = TRUE;
		}

		if (iTop < iPos)
		{
			failed = !SendMessageW(listWindow, LVM_SCROLL, 0, (iPos - iTop)*self->itemHeight);
			invalidate  = TRUE;
		}

	}	

	if (FALSE == failed)
	{
		if (position == si.nMax - si.nPage && 0 != si.nMax) 
			position++;

		if (scrollPos != position)
		{
			si.nPos = position;
			si.fMask = SIF_POS;
			SetScrollInfo(hwnd, SB_VERT, &si, (0 == (SPF_NOREDRAW & flags)));
		}
		
	}

	if (0 != (WS_VISIBLE & viewStyle))
	{
		viewStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
		if (0 == (WS_VISIBLE & viewStyle))
		{
			viewStyle |= WS_VISIBLE;
			SetWindowLongPtrW(hwnd, GWL_STYLE, viewStyle);
		}

		if (0 == (SPF_NOREDRAW & flags) && 
			FALSE != invalidate) 
		{
			InvalidateRect(listWindow, NULL, TRUE);
		}
	}

	

	return TRUE;
}

static BOOL
SmoothScrollList_EnsureVisible(HWND hwnd, int iItem, BOOL partialOk)
{
	int itemTop, itemBottom;
	int pageTop, pageBottom, delta;
	SCROLLINFO scrollInfo;
	SmoothScrollList *self;

	if (NULL == hwnd || iItem < 0)
		return FALSE;

	self = GetUserData(hwnd);
	if (NULL == self)
		return FALSE;
	
	scrollInfo.cbSize = sizeof(scrollInfo);
	scrollInfo.fMask = SIF_POS | SIF_PAGE;
	if (FALSE == GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
		return FALSE;

	itemTop = iItem * self->itemHeight;
	itemBottom = itemTop + self->itemHeight;

	pageTop = scrollInfo.nPos;
	pageBottom = pageTop + scrollInfo.nPage;

	if (FALSE != partialOk)
	{
		if (itemTop < pageBottom &&
			itemBottom > pageTop)
		{
			return TRUE;
		}
	}
	else
	{
		if (itemTop >= pageTop && 
			itemBottom <= pageBottom)
		{
			return TRUE;
		}
	}

	if (itemTop < pageTop)
		delta = itemTop - pageTop;
	else
	{
		delta = itemBottom - pageBottom;
		if ((itemTop - delta) < pageTop)
			delta = itemTop - pageTop;
	}

	if (FALSE == SmoothScrollList_SetScrollPos(hwnd, delta, SPF_RELATIVE | SPF_FORCE))
		return FALSE;

	MLSkinnedScrollWnd_UpdateBars(hwnd, TRUE);
	return TRUE;
}


static BOOL
SmoothScrollList_PreProcessKey(HWND hwnd, unsigned int vKey, unsigned int keyFlags, PostProcessKeyCommands *postProcessCommands)
{
	HWND listWindow;
	RECT viewRect;
	SmoothScrollList *self;
	int iItem, iNextItem, count;
	BOOL shortView;

	if (NULL != postProcessCommands)
		*postProcessCommands = PostProcessKeyCmd_Nothing;

	switch(vKey)
	{
		case VK_UP:
		case VK_DOWN:
		case VK_HOME:
		case VK_END:
		case VK_PRIOR:
		case VK_NEXT:
			break;

		default:
			return TRUE;
	}

	if (NULL == hwnd || FALSE == GetViewRect(hwnd, &viewRect))
		return FALSE;
	
	self = GetUserData(hwnd);
	if (NULL == self)
		return FALSE;

	listWindow = GetDlgItem(hwnd, 2);
	if (NULL == listWindow)
		return FALSE;

	iItem = (int)SendMessageW(listWindow, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)(LVNI_ALL | LVNI_FOCUSED));
	if (-1 == iItem)
		return FALSE;

	count = (int)SendMessageW(listWindow, LVM_GETITEMCOUNT, 0, 0L);

	iNextItem = iItem;

	shortView = ((viewRect.bottom - viewRect.top) < (long)self->itemHeight);

	switch(vKey)
	{
		case VK_UP:
			if (iNextItem > 0)
				iNextItem--;
			
			if (FALSE != shortView)
			{
				if (NULL != postProcessCommands)
					*postProcessCommands |= PostProcessKeyCmd_EnsureFocusVisible;
			}
			break;
		
		case VK_DOWN:
			if (FALSE == shortView)
			{
				if ((iNextItem + 1) < count)
					iNextItem++;
			}
			else
			{
				if (NULL != postProcessCommands)
					*postProcessCommands |= PostProcessKeyCmd_EnsureFocusVisible;
			}
			break;

		case VK_HOME:
			
			if (FALSE == shortView)
			{
				iNextItem = 0;
			}
			else 
			{
				iNextItem = 1;
				if (NULL != postProcessCommands)
					*postProcessCommands |= PostProcessKeyCmd_UpdateScrollPos;
			}
			break;
		case VK_END:
			if (FALSE == shortView)
			{
				iNextItem = count - 1;
			}
			else
			{
				iNextItem = -1;
				if (NULL != postProcessCommands)
					*postProcessCommands |= PostProcessKeyCmd_EnsureFocusVisible;
			}
			break;
		case VK_PRIOR:
			{
				RECT listRect;
		
				GetWindowRect(listWindow, &listRect);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&listRect, 2);
				if (listRect.top != viewRect.top)
					SmoothScrollList_SetScrollPos(hwnd, listRect.top - viewRect.top, SPF_RELATIVE);
				
				if (FALSE == shortView)
				{
					iNextItem = (viewRect.bottom - viewRect.top)/self->itemHeight;
					iNextItem = iItem - iNextItem;
					if (iNextItem < 0)
						iNextItem = 0;
				}
				else 
				{
					if (0 == iItem)
						iNextItem = 1;

					if (NULL != postProcessCommands)
						*postProcessCommands |= PostProcessKeyCmd_UpdateScrollPos;
				}
	
			}
			break;
		case VK_NEXT:
			{
				RECT listRect;
				int reminder;
				
				GetWindowRect(listWindow, &listRect);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&listRect, 2);

				reminder = (listRect.bottom - listRect.top)%self->itemHeight;
				if (0 != reminder)
					SmoothScrollList_SetScrollPos(hwnd, self->itemHeight - reminder, SPF_RELATIVE);

				if (FALSE == shortView)
				{
					iNextItem = (viewRect.bottom - viewRect.top)/self->itemHeight;
					iNextItem = iItem + iNextItem;
					if (iNextItem >= count)
						iNextItem = count - 1;

				}
				else
				{
					if (NULL != postProcessCommands)
						*postProcessCommands |= (PostProcessKeyCmd_UpdateScrollPos | PostProcessKeyCmd_EnsureFocusVisible);
				}

				
			}
			break;
	}

	if (iNextItem >= 0 && iNextItem < count)
		SmoothScrollList_EnsureVisible(hwnd, iNextItem, FALSE);
	
	return TRUE;
}

static void
SmoothScrollList_PostProcessKey(HWND hwnd, unsigned int vKey, unsigned int keyFlags, PostProcessKeyCommands processCommands)
{
	if (0 != (PostProcessKeyCmd_UpdateScrollPos & processCommands))
		UpdateScrollInfo(hwnd, SIF_POS, TRUE);

	if (0 != (PostProcessKeyCmd_EnsureFocusVisible & processCommands))
	{
		HWND listWindow = GetDlgItem(hwnd, 2);
		if (NULL != listWindow)
		{
			int iItem = (int)SendMessageW(listWindow, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)(LVNI_ALL | LVNI_FOCUSED));
			if (-1 != iItem)
			{
				SmoothScrollList_EnsureVisible(hwnd, iItem, FALSE);
			}
		}
	}
}

static void
SmoothScrollList_UpdateFontMetrics(HWND hwnd, BOOL redraw)
{
	SmoothScrollList *self;
	HWND controlWindow;
	unsigned int windowStyle;
	HFONT font, prevFont;
	HDC hdc;
	TEXTMETRICW textMetrics;
	unsigned int fontHeight;

	self = GetUserData(hwnd);
	if (NULL == self)
		return;

	windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	if(0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
	
	hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
		prevFont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
	else
		prevFont = NULL;
	
	controlWindow = GetDlgItem(hwnd, 3);
	if (NULL != controlWindow)
	{	
		font = (HFONT)SendMessageW(controlWindow, WM_GETFONT, 0, 0L);
		fontHeight = 0;

		if (NULL != hdc)
		{
			SelectObject(hdc, font);
			if (FALSE != GetTextMetricsW(hdc, &textMetrics))
				fontHeight = textMetrics.tmHeight;
		}

		if (self->headerFontHeight != fontHeight)
		{
			self->headerFontHeight = fontHeight;
			MLSkinnedHeader_SetHeight(controlWindow, -1);
		}
	}

	controlWindow = GetDlgItem(hwnd, 2);
	if (NULL != controlWindow)
	{
		font = (HFONT)SendMessageW(controlWindow, WM_GETFONT, 0, 0L);
		fontHeight = 0;

		if (NULL != hdc)
		{
			SelectObject(hdc, font);
			if (FALSE != GetTextMetricsW(hdc, &textMetrics))
				fontHeight = textMetrics.tmHeight;
		}

		if (self->listFontHeight != fontHeight)
		{
			self->listFontHeight = fontHeight;

			SmoothScrollList_SetScrollPos(hwnd, 0, SPF_NOREDRAW | SPF_FORCE);
			MLSkinnedScrollWnd_UpdateBars(hwnd, FALSE);
		}
	}

	
	if (NULL != hdc)
	{
		SelectObject(hdc, prevFont);
		ReleaseDC(hwnd, hdc);
	}

	if (0 != (WS_VISIBLE & windowStyle))
	{
		windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (0 == (WS_VISIBLE & windowStyle))
		{
			windowStyle |= WS_VISIBLE;
			SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle);
		}

		if (FALSE != redraw)
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
	}
}


static HHOOK hook = NULL;
static HWND hwndToMonitor = NULL;
static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *pMsg = (MSG*)lParam;
	if (pMsg->hwnd == hwndToMonitor)
	{
		static INT lastScrollPos = -1;
		switch(pMsg->message)
		{
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
				{
					LRESULT result = CallNextHookEx(hook, nCode, wParam, lParam);
					UnhookWindowsHookEx(hook);
					hwndToMonitor = NULL;
					hook = NULL;
					lastScrollPos = -1;
					return result;
				}
			case WM_MOUSEMOVE:
				if ((MK_LBUTTON | MK_RBUTTON) & pMsg->wParam)
				{
					RECT rw;
					POINTS pts(MAKEPOINTS(pMsg->lParam));
					POINT pt; 
					POINTSTOPOINT(pt, pts);
					MapWindowPoints(pMsg->hwnd, HWND_DESKTOP, &pt, 1);
					GetWindowRect(pMsg->hwnd, &rw);
					if (pt.y < rw.top || pt.y > rw.bottom)
					{
						HWND hwndParent = GetParent(pMsg->hwnd);
						if (hwndParent)
						{
							SCROLLINFO si;
							si.cbSize = sizeof(SCROLLINFO);
							si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
							if (GetScrollInfo(hwndParent, SB_VERT, &si))
							{
								if ((si.nPos > si.nMin && pt.y < rw.top) || (si.nPos <= (si.nMax - (INT)si.nPage) && pt.y > rw.bottom))
								{
									LRESULT result;
									if (lastScrollPos == si.nPos)
									{
										result = CallNextHookEx(hook, nCode, wParam, lParam);
										SmoothScrollList_SetScrollPos(hwndParent, (pt.y < rw.top) ? --si.nPos : ++si.nPos, SPF_NORMAL);
									}
									else
									{
										unsigned long windowStyle;
										windowStyle = GetWindowLongPtrW(hwndParent, GWL_STYLE);

										if (0 != (WS_VISIBLE & windowStyle))
											SetWindowLongPtrW(hwndParent, GWL_STYLE, windowStyle & ~WS_VISIBLE);

										result = CallNextHookEx(hook, nCode, wParam, lParam);
										PostMessageW(hwndParent, WM_EX_UPDATESCROLLINFO, SIF_POS, TRUE);

										if (0 != (WS_VISIBLE & windowStyle))
											PostMessageW(hwndParent, WM_EX_UNLOCKREDRAW, IWF_UPDATENOW | IWF_FRAME, 0L);
									}
									lastScrollPos = si.nPos;
									return result;
								}
							}
						}
					}
					SleepEx(1, TRUE);
				}
				break;
		}
	}
	return CallNextHookEx(hook, nCode, wParam, lParam);
}

static LRESULT CALLBACK ListViewSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if(uMsg == WM_MOUSEMOVE || 
		uMsg == WM_LBUTTONDOWN)
	{
		LVHITTESTINFO ht = {{LOWORD(lParam),HIWORD(lParam)},LVHT_ONITEM,-1,0};
		int item = ListView_SubItemHitTest(hwnd, &ht);
		{
			RECT r={0};
			ListView_GetItemRect(hwnd,item,&r,LVIR_BOUNDS);
			ht.pt.x -= r.left;
			ht.pt.y -= r.top;
			typedef struct {
				int x,y,item;
				HWND hwnd;
				UINT msg;
			} hitinfo;
			hitinfo info = {
				ht.pt.x, ht.pt.y, item, hwnd, uMsg,
			};
			SendMessage(GetParent(GetParent(hwnd)),WM_USER+700,(WPARAM)&info,0);
		}
	}

	switch(uMsg) 
	{
		case WM_HSCROLL:
		case WM_VSCROLL: 
		case WM_MOUSEWHEEL: 
			{
				HWND parentWindow;

				KillTimer(hwnd, 43);

				parentWindow = GetAncestor(hwnd, GA_PARENT);
				if (NULL != parentWindow)
					return SendMessageW(parentWindow, uMsg, wParam, lParam);
			}
			break;
		case WM_TIMER:	
			if (43 == wParam)
			{
				HWND parentWindow;

				KillTimer(hwnd, wParam);
	
				parentWindow = GetAncestor(hwnd, GA_PARENT);
				if (NULL != parentWindow)
				{
					int iFocused = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)(LVNI_ALL | LVNI_FOCUSED));
					if (-1 != iFocused)
						SmoothScrollList_EnsureVisible(parentWindow, iFocused, FALSE);

					return 0;	
				}
			}
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			hwndToMonitor = hwnd;
			hook = SetWindowsHookEx(WH_MSGFILTER, HookProc, NULL, GetCurrentThreadId()); 
	
			{
				unsigned int windowStyle;
				windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
				if (0 != (LVS_OWNERDRAWFIXED & windowStyle))
				{
					LRESULT result;
					
					SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle & ~LVS_OWNERDRAWFIXED);
					
					result = SubclassedListView_CallPrevWndProc(hwnd, uMsg, wParam, lParam);

					windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
					if (0 == (LVS_OWNERDRAWFIXED & windowStyle))
					{
						windowStyle |= LVS_OWNERDRAWFIXED;
						SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle);
					}

					return result;
				}
			}
			break;
	
		case WM_KEYDOWN:
			{
				HWND parentWindow;
				parentWindow = GetAncestor(hwnd, GA_PARENT);
				if (NULL != parentWindow)
				{
					PostProcessKeyCommands postProcessKeyCommands;
					if (FALSE == SmoothScrollList_PreProcessKey(parentWindow, 
															(unsigned int)wParam, 
															(unsigned int)lParam, 
															&postProcessKeyCommands))
					{
						postProcessKeyCommands = PostProcessKeyCmd_UpdateScrollPos;
					}
					
					SubclassedListView_CallPrevWndProc(hwnd, uMsg, wParam, lParam);
	
					SmoothScrollList_PostProcessKey(parentWindow, 
													(unsigned int)wParam, 
													(unsigned int)lParam,
													postProcessKeyCommands);
					return 0;
				}
			}
			break;

		case WM_CHAR:
		case WM_UNICHAR:
			{
				HWND parentWindow;
				parentWindow = GetAncestor(hwnd, GA_PARENT);
				if (NULL != parentWindow)
				{
					int iFocused;
					unsigned int windowStyle;
					
					windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
					
					if (0 != (WS_VISIBLE & windowStyle))
						SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

					SubclassedListView_CallPrevWndProc(hwnd, uMsg, wParam, lParam);

					iFocused = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)(LVNI_ALL | LVNI_FOCUSED));
					
					if (-1 != iFocused)
						SmoothScrollList_EnsureVisible(parentWindow, iFocused, FALSE);

					if (0 != (WS_VISIBLE & windowStyle))
					{
						windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
						windowStyle |= WS_VISIBLE;
						SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
					}

					InvalidateRect(hwnd, NULL, FALSE);
					return 0;
				}

			}
			break;

		case LVM_ENSUREVISIBLE:
			{
				HWND parentWindow = GetAncestor(hwnd, GA_PARENT);
				if (NULL != parentWindow)
					return SmoothScrollList_EnsureVisible(parentWindow, (int)wParam, (BOOL)lParam);
			}
			break;


		
			
	}

	return SubclassedListView_CallPrevWndProc(hwnd, uMsg, wParam, lParam);


}

static LRESULT
SmoothScrollList_OnCreate(HWND hwnd, CREATESTRUCT *createStruct)
{
	HWND hwndList, hwndHeader;
	MLSKINWINDOW m = {0};
	RECT rc;
	DWORD style;
	SmoothScrollList *self = (SmoothScrollList *)calloc(1, sizeof(SmoothScrollList));
	if (NULL == self)
		return -1;

	self->itemHeight = 1;
	self->textHeight = 1;
	SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONGX86)(LONG_PTR)self);

	m.skinType = SKINNEDWND_TYPE_SCROLLWND;
	m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
	m.hwndToSkin = hwnd;
	MLSkinWindow(g_hwnd, &m);
		
	SetScrollRange(hwnd, SB_VERT, 0, 0, FALSE);
	MLSkinnedScrollWnd_UpdateBars(hwnd, FALSE);
	
	if (FALSE == GetClientRect(hwnd, &rc))
		SetRectEmpty(&rc);

	style = WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | HDS_BUTTONS | HDS_FULLDRAG;
	hwndHeader = CreateWindowExW(WS_EX_NOPARENTNOTIFY, WC_HEADERW, NULL, style, 
				 0, 0, rc.right - rc.left, 0, hwnd, (HMENU)3,0,0);

	if (NULL != hwndHeader)
	{
		m.hwndToSkin = hwndHeader;
		m.skinType = SKINNEDWND_TYPE_HEADER;
		m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
		MLSkinWindow(g_hwnd, &m);
	}
		
	style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILD | WS_TABSTOP | WS_VISIBLE |
			LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDRAWFIXED | LVS_OWNERDATA | LVS_NOCOLUMNHEADER;
	
	hwndList = CreateWindowExW(WS_EX_NOPARENTNOTIFY, WC_LISTVIEWW, NULL, style,
								0, 0, rc.right - rc.left, rc.bottom - rc.top, hwnd,(HMENU)2,0,0);
	if (NULL != hwndList)
	{
		WNDPROC oldp = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(hwndList, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ListViewSubclass);
		SetWindowLongPtrW(hwndList,GWLP_USERDATA, (LONGX86)(LONG_PTR)oldp);

		if(NULL != hwndHeader)
			SetWindowPos(hwndHeader, hwndList, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	
		m.skinType = SKINNEDWND_TYPE_LISTVIEW;
		m.hwndToSkin = hwndList;
		m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS  |
				  SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS;

		MLSkinWindow(g_hwnd, &m);
		MLSkinnedScrollWnd_SetMode(hwndList, SCROLLMODE_STANDARD);
		MLSkinnedScrollWnd_ShowHorzBar(hwndList, FALSE);
		MLSkinnedScrollWnd_ShowVertBar(hwndList, FALSE);
	}

	SmoothScrollList_UpdateFontMetrics(hwnd, FALSE);

	return 0;
}

static void
SmoothScrollList_OnDestroy(HWND hwnd)
{
	SmoothScrollList *self;

	self  = (SmoothScrollList*)(LONG_PTR)SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
	if (NULL == self)
		return;
	
	free(self);
}

static void
SmoothScrollList_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	HWND controlWindow;
	RECT rect;
	long clientWidth;
	HWND parentWindow;
	SmoothScrollList *self;

	if ((SWP_NOSIZE | SWP_NOMOVE) == ((SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED) & windowPos->flags))
		return;
			
	self = GetUserData(hwnd);
	if (NULL == self)
		return;

	if (FALSE == GetClientRect(hwnd, &rect))
		return;

	clientWidth = rect.right - rect.left;
	
	controlWindow = GetDlgItem(hwnd, 3);
	if (NULL != controlWindow)
	{
		HDLAYOUT headerLayout;
		WINDOWPOS headerPos;
					
		headerLayout.prc = &rect;
		headerLayout.pwpos = &headerPos;
					
		if (FALSE != SendMessageW(controlWindow, HDM_LAYOUT, 0, (LPARAM)&headerLayout))
		{
			headerPos.flags |= ((SWP_NOREDRAW | SWP_NOCOPYBITS) & windowPos->flags);
			headerPos.flags &= ~SWP_NOZORDER;
			headerPos.hwndInsertAfter = HWND_TOP;
			SetWindowPos(controlWindow, headerPos.hwndInsertAfter, headerPos.x, headerPos.y, 
						headerPos.cx, headerPos.cy, headerPos.flags);
		}
	}

	if (self->viewHeight != windowPos->cy ||
		0 != (SWP_FRAMECHANGED & windowPos->flags))
	{
		ScrollPosFlags scrollFlags;

		scrollFlags = SPF_FORCE | SPF_RELATIVE;
		if (0 != (SWP_NOREDRAW & windowPos->flags))
			scrollFlags |= SPF_NOREDRAW;

		self->viewHeight = windowPos->cy;
		
		UpdateScrollInfo(hwnd, SIF_RANGE | SIF_POS, TRUE);

		SmoothScrollList_SetScrollPos(hwnd, 0, scrollFlags);
	}
	else
	{
		controlWindow = GetDlgItem(hwnd, 2);
		if (NULL != controlWindow)
		{
			if (FALSE != GetWindowRect(controlWindow, &rect) && 
				(rect.right - rect.left) != clientWidth)
			{
				SetWindowPos(controlWindow, NULL, 0, 0, clientWidth, rect.bottom - rect.top, 
					SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | ((SWP_NOREDRAW | SWP_NOCOPYBITS) & windowPos->flags));
			}
		}
	}
			
	parentWindow = GetAncestor(hwnd, GA_PARENT);
	if (NULL != parentWindow)
	{
		NMHDR hdr; 
		hdr.code = LVN_EX_SIZECHANGED;
		hdr.hwndFrom = hwnd;
		hdr.idFrom = GetWindowLongPtrW(hwnd, GWLP_ID);
		SendMessageW(parentWindow, WM_NOTIFY, hdr.idFrom, (LPARAM)&hdr);
	}
}

static void
SmoothScrollList_OnMouseWheel(HWND hwnd, INT virtualKeys, INT distance, LONG pointer_s)
{
	SmoothScrollList *self;
	int pos;
	unsigned int wheelScroll;
	int scrollLines;

	KillTimer(hwnd, 43);

	self = GetUserData(hwnd);
	if (NULL == self)
		return;
		
	if (FALSE == SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &wheelScroll, 0))
		wheelScroll = 3;

	if (0 == wheelScroll)
		return;

	if (WHEEL_PAGESCROLL == wheelScroll)
	{					
		SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(((distance > 0) ? SB_PAGEUP : SB_PAGEDOWN), 0), 0L);
		SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), 0L);
		return;
	}

	distance += self->wheelCarryover;
	scrollLines = distance * (int)wheelScroll / WHEEL_DELTA;
	self->wheelCarryover = distance - scrollLines * WHEEL_DELTA / (int)wheelScroll;

	pos = scrollLines * (int)self->textHeight;
		
	SmoothScrollList_SetScrollPos(hwnd, -pos, SPF_RELATIVE | SPF_NORMAL);
	MLSkinnedScrollWnd_UpdateBars(hwnd, TRUE);
}

static void
SmoothScrollList_OnVertScroll(HWND hwnd, INT actionLayout, INT trackPosition, HWND scrollBar)
{
	SmoothScrollList *s;
	SCROLLINFO si;
	int pos;
	ScrollPosFlags scrollFlags;
	unsigned int lineHeight;

	KillTimer(hwnd, 43);

	s = GetUserData(hwnd);
	if (NULL == s)
		return;
        						
	si.cbSize =sizeof(si);
	si.fMask = SIF_PAGE | SIF_POS | SIF_TRACKPOS | SIF_RANGE;
	
	if (FALSE == GetScrollInfo(hwnd, SB_VERT, &si))
		return;
					
	scrollFlags = SPF_NORMAL;

	if (si.nPos > (si.nMax - (INT)si.nPage)) 
		si.nPos = si.nMax - si.nPage;

	lineHeight = s->textHeight * 3;
	if (lineHeight > s->itemHeight)
		lineHeight = s->itemHeight;
	if (lineHeight > si.nPage)
		lineHeight = si.nPage;

	switch(actionLayout)
	{
		case SB_TOP:			pos = si.nMin; break;
		case SB_BOTTOM:			pos = si.nMax; break;
		case SB_LINEDOWN:		pos = si.nPos + lineHeight; break;
		case SB_LINEUP:			pos = si.nPos - lineHeight; break;
		case SB_PAGEDOWN:		pos = si.nPos + (si.nPage / s->itemHeight) * s->itemHeight; break;
		case SB_PAGEUP:			pos = si.nPos - (si.nPage / s->itemHeight) * s->itemHeight; break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:		pos = si.nTrackPos;  scrollFlags |= SPF_FORCE; break; 
		case SB_ENDSCROLL:		MLSkinnedScrollWnd_UpdateBars(hwnd, TRUE); return; 
		default:				pos = si.nPos;
	}

	SmoothScrollList_SetScrollPos(hwnd, pos, scrollFlags);
}

static LRESULT
SmoothScrollList_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *measureItem)
{
	LRESULT result;
	HWND parentWindow;
	SmoothScrollList *self;
	unsigned int itemHeight, textHeight;
	BOOL updateScroll;

	if(2 != measureItem->CtlID) 
		return FALSE;
					
	self = GetUserData(hwnd);
	

	updateScroll = FALSE;
	itemHeight = measureItem->itemHeight;

	parentWindow = GetAncestor(hwnd, GA_PARENT);
	if (NULL != parentWindow)
	{
		measureItem->CtlID = GetWindowLongPtrW(hwnd, GWLP_ID);
		result = SendMessageW(parentWindow, WM_MEASUREITEM, measureItem->CtlID, (LPARAM)measureItem);
		itemHeight = measureItem->itemHeight;
	}
	else 
		result = 0;
		
	textHeight = 12;
	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
	if (NULL != hdc)
	{
		HFONT font, fontPrev;
		TEXTMETRIC textMetrics;

		font = (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0L);
		fontPrev = (HFONT)SelectObject(hdc, font);

		if (FALSE != GetTextMetrics(hdc, &textMetrics))
			textHeight = textMetrics.tmHeight;

		SelectObject(hdc, fontPrev);
		ReleaseDC(hwnd, hdc);
	}
	
	if (NULL != self)
	{
		if (self->itemHeight != itemHeight)
		{
			SmoothScrollList_SetScrollPos(hwnd, 0, SPF_NOREDRAW);
			self->itemHeight = itemHeight;
			updateScroll = TRUE;
		}

		if (self->textHeight != textHeight)
		{
			self->textHeight = textHeight;
			updateScroll = TRUE;
		}
	}
	
	if (FALSE != updateScroll)
	{		
		UpdateScrollInfo(hwnd, SIF_RANGE | SIF_POS, TRUE);
	}

	return result;
}

static void
SmoothScrollList_OnSetFont(HWND hwnd, HFONT font, BOOL redraw)
{
	if (0 == (SWS_USESKINFONT & MLSkinnedWnd_GetStyle(hwnd)))
	{
		HWND controlWindow;
				
		controlWindow = GetDlgItem(hwnd,3);
		if (NULL != controlWindow)
			SendMessageW(controlWindow, WM_SETFONT, (WPARAM)font, MAKELPARAM(0, 0L));

		controlWindow = GetDlgItem(hwnd,2);
		if (NULL != controlWindow)
			SendMessageW(controlWindow, WM_SETFONT, (WPARAM)font, MAKELPARAM(0, 0L));
	
		SmoothScrollList_UpdateFontMetrics(hwnd, redraw);
	}
}


static LRESULT
SmoothScrollList_OnGetFont(HWND hwnd)
{
	HWND listWindow;

	listWindow = GetDlgItem(hwnd, 2);
	if (NULL != listWindow) 
		return SendMessageW(listWindow, WM_GETFONT, 0, 0L);
			
	return DefWindowProcW(hwnd, WM_GETFONT, 0, 0L);
}

static void
SmoothScrollList_OnSetRedraw(HWND hwnd, BOOL enableRedraw)
{
	HWND childWindow;

	DefWindowProcW(hwnd, WM_SETREDRAW, enableRedraw, 0L);
	
	childWindow = GetDlgItem(hwnd, 3);
	if (NULL != childWindow)
	{
		SendMessage(childWindow, WM_SETREDRAW, enableRedraw, 0L);
		if (FALSE != enableRedraw)
			InvalidateRect(childWindow, NULL, TRUE);
	}

	childWindow = GetDlgItem(hwnd, 2);
	if (NULL != childWindow)
	{
		SendMessage(childWindow, WM_SETREDRAW, enableRedraw, 0L);
		if (FALSE != enableRedraw)
			InvalidateRect(childWindow, NULL, TRUE);
	}
}


static void 
SmoothScrollList_OnSkinUpdated(HWND hwnd, BOOL notifyChildren, BOOL redraw)
{
	SmoothScrollList_UpdateFontMetrics(hwnd, redraw);
}

static LRESULT
SmoothScrollList_OnDisplaySort(HWND hwnd, int sortIndex, BOOL ascendingOrder)
{
	HWND headerWindow;

	headerWindow = GetDlgItem(hwnd, 3);
	if (NULL == headerWindow)
		return 0;

	return SENDMLIPC(headerWindow, ML_IPC_SKINNEDHEADER_DISPLAYSORT, MAKEWPARAM(sortIndex, ascendingOrder));
}

static LRESULT
SmoothScrollList_OnGetSort(HWND hwnd)
{
	HWND headerWindow;

	headerWindow = GetDlgItem(hwnd, 3);
	if (NULL == headerWindow)
		return 0;

	return SENDMLIPC(headerWindow, ML_IPC_SKINNEDHEADER_GETSORT, 0);
}

static void
SmoothScrollList_OnKeyDown(HWND hwnd, unsigned int vKey, unsigned int keyFlags)
{
	HWND listWindow;
	listWindow = GetDlgItem(hwnd, 2);
	
	if (NULL != listWindow && 
		WS_VISIBLE == ((WS_VISIBLE | WS_DISABLED) & GetWindowLongPtrW(listWindow, GWL_STYLE)))
	{
		SendMessageW(listWindow, WM_KEYDOWN, vKey, (LPARAM)keyFlags);
	}

	DefWindowProcW(hwnd, WM_KEYDOWN, vKey, (LPARAM)keyFlags);
}

static LRESULT
SmoothScrollList_OnMediaLibraryIPC(HWND hwnd, INT msg, INT_PTR param)
{
	switch(msg)
	{
		case ML_IPC_SKINNEDWND_SKINUPDATED:			SmoothScrollList_OnSkinUpdated(hwnd, LOWORD(param), HIWORD(param)); break;
		case ML_IPC_SKINNEDLISTVIEW_DISPLAYSORT: 	return SmoothScrollList_OnDisplaySort(hwnd, LOWORD(param), HIWORD(param));
		case ML_IPC_SKINNEDLISTVIEW_GETSORT:		return SmoothScrollList_OnGetSort(hwnd);
	}
	return 0;
}

static LRESULT CALLBACK SmoothScrollMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) 
	{
		case WM_CREATE:				return SmoothScrollList_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			SmoothScrollList_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED:	SmoothScrollList_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_MOUSEWHEEL:			SmoothScrollList_OnMouseWheel(hwnd, LOWORD(wParam), (short)HIWORD(wParam), (LONG)lParam); return 0;
		case WM_VSCROLL:			SmoothScrollList_OnVertScroll(hwnd, LOWORD(wParam), (short)HIWORD(wParam), (HWND)lParam); return 0;
		case WM_ERASEBKGND: 		return 1;
		case WM_MEASUREITEM:		return SmoothScrollList_OnMeasureItem(hwnd, (MEASUREITEMSTRUCT*)lParam);
		case WM_SETFONT:			SmoothScrollList_OnSetFont(hwnd, (HFONT)wParam, LOWORD(lParam)); return 0; 
		case WM_GETFONT:			return SmoothScrollList_OnGetFont(hwnd);
		case WM_SETREDRAW:			SmoothScrollList_OnSetRedraw(hwnd, (BOOL)wParam); return 0;
	
		case LVM_GETHEADER:
			return (LRESULT)GetDlgItem(hwnd,3);
		case LVM_INSERTCOLUMNA:
		case LVM_INSERTCOLUMNW:
			{
				LVCOLUMNW *listColumn = (LVCOLUMNW*)lParam;
				HWND controlWindow;
				LRESULT result;

				result = -1;

				controlWindow = GetDlgItem(hwnd,3);
				if (NULL != controlWindow)
				{
					HDITEMW headerItem;
																
					if (FALSE == CopyListColumnToHeaderItem(listColumn, &headerItem))
						return -1;

					if (0 == (HDI_FORMAT & headerItem.mask))
					{
						headerItem.mask |= HDI_FORMAT;
						headerItem.fmt = HDF_LEFT;
					}

					result = SendMessageW(controlWindow, 
									(LVM_INSERTCOLUMNW == uMsg) ? HDM_INSERTITEMW : HDM_INSERTITEMA,
									wParam,	(LPARAM)&headerItem);

					if (-1 == result)
						return result;
				}

				controlWindow = GetDlgItem(hwnd, 2);
				if (NULL != controlWindow)
					result = SendMessageW(controlWindow, uMsg, wParam, lParam);

				return result;
			}
			break;
		case LVM_DELETECOLUMN:
			{
				HWND controlWindow;
				controlWindow = GetDlgItem(hwnd,3);
				if (NULL != controlWindow && 
					FALSE ==SendMessageW(controlWindow, HDM_DELETEITEM, wParam, 0L))
				{
					return FALSE;
				}
				
				controlWindow = GetDlgItem(hwnd,2);
				if (NULL != controlWindow)
					return SendMessageW(controlWindow ,uMsg,wParam,lParam);
			}
			return FALSE;
		case LVM_GETCOLUMNW:
		case LVM_GETCOLUMNA:
			{
				LVCOLUMNW *l = (LVCOLUMNW *)lParam;
				HDITEMW h;
				HWND headerWindow;
				
				headerWindow = GetDlgItem(hwnd, 3);
				if (NULL == headerWindow)
					return FALSE;

				if (FALSE == CopyListColumnToHeaderItem(l, &h))
					return FALSE;

				if(!SendMessageW(headerWindow, 
								(LVM_GETCOLUMNW == uMsg) ? HDM_GETITEMW : HDM_GETITEMA, 
								wParam, 
								(LPARAM)&h))
				{
					return FALSE;
				}
				
				if (FALSE == CopyHeaderItemToListColumn(&h, l))
					return FALSE;
			}
			return TRUE;
		case LVM_GETCOLUMNWIDTH:
			{				
				HWND controlWindow;

				controlWindow = GetDlgItem(hwnd,3);
				if (NULL != controlWindow)
				{
					HDITEMW h;
					h.mask = HDI_WIDTH;
					if (FALSE == SendMessageW(controlWindow, HDM_GETITEM, wParam, (LPARAM)&h))
						return 0;

					return h.cxy;
				}

				controlWindow = GetDlgItem(hwnd, 2);
				if (NULL != controlWindow)
					return SendMessageW(controlWindow, uMsg, wParam, lParam);
			}
			break;
		case LVM_SETCOLUMNW:
		case LVM_SETCOLUMNA:
			{
				LVCOLUMNW *l = (LVCOLUMNW *)lParam;
				HWND controlWindow;
				LRESULT result;
				
				controlWindow = GetDlgItem(hwnd, 3);
				if (NULL != controlWindow)
				{
					HDITEMW h;

					if (FALSE == CopyListColumnToHeaderItem(l, &h))
						return FALSE;

					if(!SendMessageW(controlWindow, 
							(LVM_SETCOLUMNW == uMsg) ? HDM_SETITEMW : HDM_SETITEMA, 
							wParam, (LPARAM)&h))
					{
						return FALSE;
					}
				
					if (FALSE == CopyHeaderItemToListColumn(&h, l))
						return FALSE;

					result = TRUE;
				}
				else result = FALSE;

				controlWindow = GetDlgItem(hwnd,2);
				if (NULL != controlWindow)
					result = SendMessageW(controlWindow, uMsg, wParam, lParam);

				return result;
			}
			break;

		case LVM_SETCOLUMNWIDTH:
			{
				HWND controlWindow;
				LRESULT result;

				controlWindow = GetDlgItem(hwnd, 3);
				if (NULL != controlWindow)
				{
					HDITEMW headerItem;

					if (LVSCW_AUTOSIZE == lParam)
						return FALSE;

					if (LVSCW_AUTOSIZE_USEHEADER == lParam)
						return FALSE;

					headerItem.mask = HDI_WIDTH;
					headerItem.cxy = (int)lParam;

					result = SendMessageW(controlWindow, HDM_SETITEMW, (WPARAM)wParam, (LPARAM)&headerItem);
					if (FALSE == result)
						return FALSE;
				}
				else
					result = FALSE;

				controlWindow = GetDlgItem(hwnd,2);
				if (NULL != controlWindow)
					result = SendMessageW(controlWindow, uMsg, wParam, lParam);

				return result;
			}
			break;
		
		case LVM_SETITEMCOUNT:
			{
				LRESULT result;
				HWND controlWindow = GetDlgItem(hwnd,2);
				
				result = (NULL != controlWindow) ? 
							SendMessageW(controlWindow, uMsg, wParam,lParam) : 
							0;

				UpdateScrollInfo(hwnd, SIF_RANGE | SIF_POS, TRUE);
				return result;
			}
			break;
		case LVM_ENSUREVISIBLE:
			return SmoothScrollList_EnsureVisible(hwnd, (int)wParam, (BOOL)lParam);
		
		case WM_EX_UPDATESCROLLINFO:
			return UpdateScrollInfo(hwnd, (UINT)wParam, (BOOL)lParam);
			
		case WM_EX_UNLOCKREDRAW:
			{				
				unsigned long windowStyle;
				unsigned int redrawFlags;
				HRGN regionInvalid;
				RECT rect;

				windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
				if (0 == (WS_VISIBLE & windowStyle))
					SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle | WS_VISIBLE);
			
				redrawFlags = RDW_INVALIDATE | RDW_ALLCHILDREN;
							

				if (0 != (IWF_FRAME & wParam))
				{
					redrawFlags |= RDW_FRAME;
					GetWindowRect(hwnd, &rect);
					MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);
				}
				else
					GetClientRect(hwnd, &rect);

				if (0 != (IWF_ERASE & wParam))
					redrawFlags |= RDW_ERASE;
				
				if (0 != (IWF_UPDATENOW & wParam))
				{
					redrawFlags |= RDW_UPDATENOW;
					if (0 != (IWF_ERASE & wParam))
						redrawFlags |= RDW_ERASENOW;
				}


				
				regionInvalid = CreateRectRgnIndirect(&rect);
				if (NULL != regionInvalid)
				{
					HWND headerWindow;

					headerWindow = GetDlgItem(hwnd, 3);
					if (NULL != headerWindow && 
						0 != (WS_VISIBLE & GetWindowLongPtrW(headerWindow, GWL_STYLE)))
					{
						HRGN regionHeader;
						GetWindowRect(headerWindow, &rect);
						MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rect, 2);
						regionHeader = CreateRectRgnIndirect(&rect);
						if (NULL != regionHeader)
						{
							CombineRgn(regionInvalid, regionInvalid, regionHeader, RGN_DIFF);
							DeleteObject(regionHeader);
						}
					}
				}

				RedrawWindow(hwnd, NULL, regionInvalid, redrawFlags);

				if (NULL != regionInvalid)
					DeleteObject(regionInvalid);
			}
			break;
		case WM_NOTIFY:
			{
				LPNMHDR l=(LPNMHDR)lParam;
				if(l->idFrom == 2) 
				{
					l->idFrom = GetWindowLongPtrW(hwnd,GWLP_ID);
					l->hwndFrom = hwnd; // this is prevents double reflecting
					return SendMessageW(GetParent(hwnd),uMsg,l->idFrom,lParam);
				}
				else if(l->idFrom == 3) 
				{
					switch(l->code) 
					{
						case HDN_ITEMCLICKA:
						case HDN_ITEMCLICKW:
							{
								NMHEADER *nm = (NMHEADER*)lParam;
								HWND hwndParent;
								hwndParent = GetParent(hwnd);
								if (hwndParent)
								{
									wParam = GetWindowLongPtrW(hwnd,GWLP_ID);
									if(nm->iButton == 0) { // left click
										NMLISTVIEW p = {{hwnd, wParam, LVN_COLUMNCLICK},-1,nm->iItem,0};
										return SendMessageW(hwndParent,WM_NOTIFY,wParam,(LPARAM)&p);
									} else if(nm->iButton == 1) { // right click
										NMHDR p = {nm->hdr.hwndFrom,wParam,NM_RCLICK};
										return SendMessageW(hwndParent,WM_NOTIFY,wParam,(LPARAM)&p);
									}
								}
							}
							break;
						case HDN_ITEMCHANGINGA:
						case HDN_ITEMCHANGINGW:
						case HDN_ITEMCHANGEDA:
						case HDN_ITEMCHANGEDW:
							{
								LRESULT result;
								NMHEADER *nm = (NMHEADER*)lParam;

								result = SendMessageW(GetParent(hwnd),uMsg, wParam,lParam);
								if (FALSE != result &&
									(HDN_ITEMCHANGINGW == l->code || HDN_ITEMCHANGINGA == l->code))
								{
									return result;
								}

								if (NULL != nm->pitem && 
									0 != (HDI_WIDTH & nm->pitem->mask)) 
								{
									HWND hwndList;
									hwndList = GetDlgItem(hwnd,2);
									if (hwndList)
									{
										unsigned long windowStyle;
										windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
										if (0 != (WS_VISIBLE & windowStyle))
											SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

										ListView_SetColumnWidth(hwndList, nm->iItem,nm->pitem->cxy);

										if (0 != (WS_VISIBLE & windowStyle))
										{
											windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);
											if (0 == (WS_VISIBLE & windowStyle))
											{
												windowStyle |= WS_VISIBLE;
												SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle);
											}

											InvalidateRect(hwndList, NULL, FALSE);
										}
									}
								}

								return result;
							}
							break;
					}
					return SendMessageW(GetParent(hwnd),uMsg, wParam,lParam);
				}
			}
			break;

		case WM_EX_GETREALLIST: 
			return (LRESULT)GetDlgItem(hwnd, 2);
		case WM_EX_GETCOUNTPERPAGE: 
			return SendMessageW(GetDlgItem(hwnd, 2), LVM_GETCOUNTPERPAGE, 0, 0L) + 1;
				
		case WM_KEYDOWN:		SmoothScrollList_OnKeyDown(hwnd, (unsigned int)wParam, (unsigned int)lParam); return 0;
			
		case WM_ML_IPC:
			return SmoothScrollList_OnMediaLibraryIPC(hwnd, (INT)lParam, (INT_PTR)wParam);

		default:
			if(uMsg >= LVM_FIRST && uMsg < LVM_FIRST + 0x100) 
			{
				HWND hwndList = GetDlgItem(hwnd,2);
				if (hwndList) return ListViewSubclass(hwndList, uMsg, wParam, lParam);
			}
			break;
	}
	return DefWindowProcW(hwnd,uMsg,wParam,lParam);
}

void InitSmoothScrollList() {
	WNDCLASSW wc = {0, };

	if (GetClassInfoW(plugin.hDllInstance, L"SmoothScrollList", &wc)) return;
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = SmoothScrollMsgProc;
	wc.hInstance = plugin.hDllInstance;
	wc.lpszClassName = L"SmoothScrollList";
	RegisterClassW(&wc);
}
