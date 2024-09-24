#include "main.h"

#include "../gen_ml/ml.h"
#include "../gen_ml/ml_ipc_0313.h"
#include "../Winamp/gen.h"

#define WM_EX_GETREALLIST		(WM_USER + 0x01)
#define WM_EX_UNLOCKREDRAW		(WM_USER + 0x02)
#define WM_EX_UPDATESCROLLINFO	(WM_USER + 0x03)
#define WM_EX_GETCOUNTPERPAGE	(WM_USER + 0x04)

#define LVN_EX_SIZECHANGED		(LVN_LAST)

#define IWF_NO				0x0000
#define IWF_ALL				0x00FF
#define IWF_CONTAINER		0x0001
#define IWF_LISTVIEW			0x0002
#define IWF_HEADER			0x0004
#define IWF_UPDATENOW		0x1000

BOOL
CopyListColumnToHeaderItem(const LVCOLUMNW *column, HDITEMW *item)
{
	if (NULL == column || NULL == item)
		return FALSE;

	item->mask = 0;
	if (0 != (LVCF_FMT & column->mask))
	{
		item->mask |= HDI_FORMAT;
		item->fmt = 0;

		switch((LVCFMT_JUSTIFYMASK & column->fmt))
		{
			case LVCFMT_RIGHT:
				item->fmt |= HDF_RIGHT;
				break;
			case LVCFMT_CENTER:
				item->fmt |= HDF_CENTER;
				break;
			default:
				item->fmt |= HDF_LEFT;
				break;
		}

		if (0 != (LVCFMT_IMAGE & column->fmt))
			item->fmt |= HDF_IMAGE;

		if (0 != (LVCFMT_BITMAP_ON_RIGHT & column->fmt))
			item->fmt |= HDF_BITMAP_ON_RIGHT;
	}
				
	if (0 != (LVCF_WIDTH & column->mask))
	{
		item->mask |= HDI_WIDTH;
		item->cxy = column->cx;
	}

	if (0 != (LVCF_TEXT & column->mask))
	{
		item->mask |= HDI_TEXT;
		item->pszText = column->pszText;
		item->cchTextMax = column->cchTextMax;
	}

	if (0 != (LVCF_IMAGE & column->mask))
	{
		item->mask |= HDI_IMAGE;
		item->iImage = column->iImage;
	}

	if (0 != (LVCF_ORDER & column->mask))
	{
		item->mask |= HDI_ORDER;
		item->iOrder = column->iOrder;
	}

	return TRUE;
}

BOOL
CopyHeaderItemToListColumn(const HDITEMW *item, LVCOLUMNW *column)
{
	if (NULL == column || NULL == item)
		return FALSE;

	column->mask = 0;
	if (0 != (HDI_FORMAT& item->mask))
	{
		column->mask |= LVCF_FMT ;
		column->fmt = 0;

		switch((HDF_JUSTIFYMASK & item->fmt))
		{
			case HDF_RIGHT:
				column->fmt |= LVCFMT_RIGHT;
				break;
			case HDF_CENTER:
				column->fmt |= LVCFMT_CENTER;
				break;
			default:
				column->fmt |= LVCFMT_LEFT;
				break;
		}

		if (0 != (HDF_IMAGE & item->fmt))
			column->fmt |= LVCFMT_IMAGE;

		if (0 != (HDF_BITMAP_ON_RIGHT & item->fmt))
			column->fmt |= LVCFMT_BITMAP_ON_RIGHT;
	}
				
	if (0 != (HDI_WIDTH & item->mask))
	{
		column->mask |= LVCF_WIDTH;
		column->cx = item->cxy;
	}

	if (0 != (HDI_TEXT & item->mask))
	{
		column->mask |= LVCF_TEXT;
		column->pszText = item->pszText;
		column->cchTextMax = item->cchTextMax;
	}

	if (0 != (HDI_IMAGE & item->mask))
	{
		column->mask |= LVCF_IMAGE;
		column->iImage = item->iImage;
	}

	if (0 != (HDI_ORDER & item->mask))
	{
		column->mask |= LVCF_ORDER;
		column->iOrder = item->iOrder;
	}

	return TRUE;
}

static BOOL UpdateScrollInfo(HWND hwnd, UINT fMask, BOOL bRedraw) 
{
	HWND hwndList;
	SCROLLINFO si;
	BOOL bUpdateBars(FALSE);

	hwndList = GetDlgItem(hwnd,2);

	si.cbSize= sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

	if (!hwndList || !GetScrollInfo(hwnd, SB_VERT, &si)) 
		return FALSE;

	if (SIF_RANGE & fMask)
	{
		RECT rc, rv;
	
		UINT nPage;
		INT nMax;
		BOOL bScrollVisible;
		HDLAYOUT headerLayout;
		WINDOWPOS headerPos;

		HWND hwndHeader = GetDlgItem(hwnd, 3);
		if (!hwndHeader) 
			return FALSE;

		GetClientRect(hwnd, &rc);
		ListView_GetViewRect(hwndList,&rv);
	
		headerLayout.prc = &rc;
		headerLayout.pwpos = &headerPos;
					
		SendMessageW(hwndHeader, HDM_LAYOUT, 0, (LPARAM)&headerLayout);
		
		bScrollVisible = (si.nMax > 0 && si.nPage > 0 && (INT)si.nPage < si.nMax);
        
		nMax = rv.bottom - rv.top;
		nPage = (rc.bottom - rc.top);

		if (si.nMax != nMax || si.nPage != nPage)
		{
			si.nMin = 0;
			si.nMax = nMax;
			si.nPage = nPage;
			SetScrollInfo(hwnd, SB_VERT, &si, FALSE);
			bUpdateBars = TRUE;
			if (bScrollVisible != (si.nMax > 0 && si.nPage > 0 && (INT)si.nPage < si.nMax))
			{
				MLSkinnedScrollWnd_UpdateBars(hwnd, bRedraw);
				GetClientRect(hwnd, &rc);

				headerLayout.prc = &rc;
				headerLayout.pwpos = &headerPos;
					
				SendMessageW(hwndHeader, HDM_LAYOUT, 0, (LPARAM)&headerLayout);

				headerPos.flags |= SWP_NOREDRAW;
				SetWindowPos(hwndHeader, headerPos.hwndInsertAfter, headerPos.x, headerPos.y, 
						headerPos.cx, headerPos.cy, headerPos.flags);
				
				SetWindowPos(hwndList, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
						SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);

				if (FALSE != bRedraw)
					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

				bUpdateBars = FALSE;
			}
		}
		
	}

	if (SIF_POS & fMask)
	{
		POINT p;
		if (FALSE != SendMessageW(hwndList, LVM_GETORIGIN, (WPARAM)0, (LPARAM)&p))
		{
			if (p.y < si.nMin)
				p.y = si.nMin;
			
			if (p.y > (si.nMax - (int)si.nPage))
				p.y = si.nMax - si.nPage;
			
			if (p.y != si.nPos)
			{
				si.nPos = p.y;
				si.fMask = SIF_POS;
				SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
				bUpdateBars = TRUE;
			}
		}
	}
	if (bUpdateBars) 
		MLSkinnedScrollWnd_UpdateBars(hwnd, bRedraw);

	return TRUE;
}

static void
UpdateFontMetrics(HWND hwnd, BOOL redraw)
{
	HWND controlWindow;
	RECT headerRect;
	unsigned int windowStyle;


	windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	if(0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtrW(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);

	
	controlWindow = GetDlgItem(hwnd, 3);
	if (NULL != controlWindow)
	{	
		MLSkinnedHeader_SetHeight(controlWindow, -1);
		if (FALSE == GetWindowRect(controlWindow, &headerRect))
			SetRectEmpty(&headerRect);
	}
	else
		SetRectEmpty(&headerRect);


	controlWindow = GetDlgItem(hwnd, 2);
	if (NULL != controlWindow)
	{
		
		RECT rect;
		if (FALSE != GetClientRect(hwnd, &rect))
		{
			SetWindowPos(controlWindow, NULL, 
						 rect.left, rect.top + headerRect.bottom - headerRect.top, 
 						 rect.right - rect.left, (rect.bottom - rect.top) - (headerRect.bottom - headerRect.top), 
						 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS);  
		}
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
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
	}
}

static HHOOK hook = NULL;
static HWND hwndToMonitor = NULL;

typedef struct UpdateSelection
{
	BOOL active;
	int iFrom;
	int iTo;
}UpdateSelection;


typedef struct ListViewData
{
	WNDPROC prevWindowProc;
	UpdateSelection updateSelection;
} ListViewData;

#define LVCWP_NORMAL			0
#define LVCWP_UPDATESCROLLINFO	(1 << 0)

static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *pMsg = (MSG*)lParam;
	if (pMsg->hwnd == hwndToMonitor)
	{
		switch(pMsg->message)
		{
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
				{
					LRESULT result = CallNextHookEx(hook, nCode, wParam, lParam);
					UnhookWindowsHookEx(hook);
					hwndToMonitor = NULL;
					hook = NULL;
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
								if ((si.nPos > si.nMin && pt.y < rw.top) || (si.nPos < (si.nMax - (INT)si.nPage) && pt.y > rw.bottom))
								{
									SendMessageW(hwndParent, WM_SETREDRAW, FALSE, 0L);
									LRESULT result = CallNextHookEx(hook, nCode, wParam, lParam);
									PostMessageW(hwndParent, WM_EX_UPDATESCROLLINFO, SIF_POS, FALSE);
									PostMessageW(hwndParent, WM_EX_UNLOCKREDRAW, IWF_CONTAINER | IWF_LISTVIEW | IWF_UPDATENOW, 0L);
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
static void
ListView_ScheduleSelectionUpdate(HWND hwnd, int iFrom, int iTo)
{
	ListViewData *listView;
	listView = (ListViewData*)(LONG_PTR)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (NULL == listView)
		return;

	listView->updateSelection.iFrom = iFrom;
	listView->updateSelection.iTo = iTo;
	listView->updateSelection.active = TRUE;
}

static void
ListView_UpdateSelection(HWND hwnd, int iFrom, int iTo)
{
	HWND parentWindow;
	LVITEM item;
	int start, stop, i, lim;
	
	start = (int)SendMessageW(hwnd, LVM_GETSELECTIONMARK, 0, 0L);
	stop = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, iFrom - 1, LVNI_FOCUSED);
	if (stop < start)
	{
		int tmp = start;
		start = stop;
		stop = tmp;
	}
					
	item.state = 0;
	item.stateMask = LVIS_SELECTED;

	if (start != iFrom)
	{
		if (start < iFrom) 
		{
			i = start; 
			lim = iFrom;
		}
		else 
		{
			i = iFrom; 
			lim = start; 
		}

		i--;
		for(;;)
		{
			i = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)i, 
								 (LPARAM)(LVNI_ALL | LVNI_SELECTED));
			if (-1 == i || i >= lim)
				break;
			
			SendMessageW(hwnd, LVM_SETITEMSTATE, i, (LPARAM)&item);
		}
	}
	if (stop < iTo)  
	{
		i = stop + 1; 
		lim = iTo; 

		i--;
		for(;;)
		{
			i = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)i, 
								 (LPARAM)(LVNI_ALL | LVNI_SELECTED));
			if (-1 == i || i > lim)
				break;
			
			SendMessageW(hwnd, LVM_SETITEMSTATE, i, (LPARAM)&item);
		}
	}

	parentWindow = GetAncestor(hwnd, GA_PARENT);
	if (NULL != parentWindow)
	{
		HWND notifyWindow;
		
		SendMessageW(parentWindow, WM_SETREDRAW, TRUE, 0L);
		RedrawWindow(parentWindow, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
		
		notifyWindow =  GetAncestor(parentWindow, GA_PARENT);
		if (NULL != notifyWindow)
		{
			NMLVODSTATECHANGE stateChange;
			stateChange.hdr.idFrom = GetWindowLongPtrW(parentWindow, GWLP_ID);
			stateChange.hdr.hwndFrom = parentWindow;
			stateChange.hdr.code = LVN_ODSTATECHANGED;
			stateChange.iFrom = start;
			stateChange.iTo = stop;
			stateChange.uNewState = LVIS_SELECTED;
			stateChange.uOldState = 0;
			SendMessageW(notifyWindow, WM_NOTIFY, stateChange.hdr.idFrom, (LPARAM)&stateChange);
		}
	}
}
static LRESULT
ListView_CallWindowProc(ListViewData *listView, HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam,
						unsigned int flags)
{
	LRESULT result;
	
	result = CallWindowProcW(listView->prevWindowProc, hwnd, uMsg, wParam, lParam);
	
	if (FALSE != listView->updateSelection.active)
	{
		listView->updateSelection.active = FALSE;
		ListView_UpdateSelection(hwnd, listView->updateSelection.iFrom, listView->updateSelection.iTo);
	}

	if (0 != (LVCWP_UPDATESCROLLINFO & flags))
	{
		HWND hwndParent;
		hwndParent = GetAncestor(hwnd, GA_PARENT);
		if (NULL != hwndParent)
			UpdateScrollInfo(hwndParent, SIF_POS, TRUE);
	}
	
	return result;	
}

static LRESULT WINAPI 
ListView_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ListViewData *listView;
	listView = (ListViewData*)(LONG_PTR)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	
	if (NULL == listView || 
		NULL == listView->prevWindowProc)
	{
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}
	
	switch(uMsg)
	{
		case WM_DESTROY:
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, NULL);
			CallWindowProcW(listView->prevWindowProc, hwnd, uMsg, wParam, lParam);
			free(listView);
			return 0;
		case WM_KEYDOWN:
			switch(wParam)
			{
				case VK_PRIOR:
				case VK_UP:
				case VK_HOME:
				case VK_NEXT:
				case VK_END:
				case VK_DOWN:
				case VK_LEFT:
				case VK_RIGHT:
					return ListView_CallWindowProc(listView, hwnd, uMsg, wParam, lParam, LVCWP_UPDATESCROLLINFO);
			}
			break;
		case WM_TIMER:
			if (43 == wParam) 
				return ListView_CallWindowProc(listView, hwnd, uMsg, wParam, lParam, LVCWP_UPDATESCROLLINFO);
			break;
		case WM_HSCROLL:
		case WM_VSCROLL:
		case WM_MOUSEWHEEL: 
		case LVM_ENSUREVISIBLE:
			return ListView_CallWindowProc(listView, hwnd, uMsg, wParam, lParam, LVCWP_UPDATESCROLLINFO);
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			{
				LVHITTESTINFO ht;
				ht.pt.x = GET_X_LPARAM(lParam);
				ht.pt.y = GET_Y_LPARAM(lParam);
				if (-1 == SendMessageW(hwnd, LVM_HITTEST, 0, (LPARAM)&ht) || 0 == (LVHT_ONITEM  & ht.flags))
				{
					hwndToMonitor = hwnd;
					hook = SetWindowsHookEx(WH_MSGFILTER, HookProc, NULL, GetCurrentThreadId()); 
				}
			}
			break;
	}

	return ListView_CallWindowProc(listView, hwnd, uMsg, wParam, lParam, LVCWP_NORMAL);
}


static LRESULT
HeaderIconList_OnCreate(HWND hwnd, CREATESTRUCT *createStruct)
{
	HWND hwndList, hwndHeader;
	MLSKINWINDOW m;
	RECT rc;
	DWORD style;

	m.skinType = SKINNEDWND_TYPE_SCROLLWND;
	m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
	m.hwndToSkin = hwnd;
	MLSkinWindow(g_hwnd, &m);
	
	SetScrollRange(hwnd, SB_VERT, 0, 0, FALSE);
	MLSkinnedScrollWnd_UpdateBars(hwnd, FALSE);

	GetClientRect(hwnd, &rc);

	style = WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | HDS_BUTTONS | HDS_FULLDRAG;
	hwndHeader = CreateWindowExW(WS_EX_NOPARENTNOTIFY, WC_HEADERW, L"", style, 0, 0, rc.right - rc.left, 20, hwnd, (HMENU)3,0,0);
	if (NULL != hwndHeader)
	{
		m.hwndToSkin = hwndHeader;
		m.skinType = SKINNEDWND_TYPE_HEADER;
		m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
		MLSkinWindow(g_hwnd, &m);
	}
	
	style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_CHILD | WS_TABSTOP | WS_VISIBLE |
					 LVS_ICON | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_NOLABELWRAP;
	
	hwndList = CreateWindowExW(WS_EX_NOPARENTNOTIFY, WC_LISTVIEWW,L"",style,0, 20, rc.right - rc.left, rc.bottom - rc.top - 20, hwnd,(HMENU)2,0,0);
	if (NULL != hwndList)
	{
		ListViewData *listView;
		listView = (ListViewData*)calloc(1, sizeof(ListViewData));
		if (NULL != listView)
		{
			listView->prevWindowProc = (WNDPROC)(LONG_PTR)SetWindowLongPtrW(hwndList, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ListView_WindowProc);
			SetWindowLongPtrW(hwndList,GWLP_USERDATA, (LONGX86)(LONG_PTR)listView);
		}
		
		SendMessageW(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_INFOTIP, LVS_EX_INFOTIP);

		if (NULL != hwndHeader)
			SetWindowPos(hwndHeader, hwndList, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		
		m.skinType = SKINNEDWND_TYPE_LISTVIEW;
		m.hwndToSkin = hwndList;
		m.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER;
		MLSkinWindow(g_hwnd, &m);
		MLSkinnedScrollWnd_SetMode(hwndList, SCROLLMODE_STANDARD);
		MLSkinnedScrollWnd_ShowHorzBar(hwndList, FALSE);
		MLSkinnedScrollWnd_ShowVertBar(hwndList, FALSE);
	}

	UpdateFontMetrics(hwnd, FALSE);

	return 0;
}

static void
HeaderIconList_OnDestroy(HWND hwnd)
{
}

// TODO need to finish off the butting up to edge handling
static void
HeaderIconList_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	RECT rc;
	HWND hwndHeader, hwndList;
	hwndHeader = GetDlgItem(hwnd, 3);
	hwndList = GetDlgItem(hwnd, 2);
	BOOL bRedraw;

	bRedraw = (0 == (SWP_NOREDRAW & windowPos->flags));

	if ((SWP_NOSIZE | SWP_NOMOVE) == ((SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED) & windowPos->flags))
		return;

	hwndHeader = GetDlgItem(hwnd, 3);
	hwndList = GetDlgItem(hwnd, 2);
	if (hwndHeader &&  hwndList)
	{
		HDLAYOUT headerLayout;
		WINDOWPOS headerPos;
		
		GetClientRect(hwnd, &rc);

		/*SCROLLINFO si;
		si.cbSize= sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		BOOL got = GetScrollInfo(hwnd, SB_VERT, &si);
		BOOL bScrollVisible = (si.nMax > 0 && si.nPage > 0 && (INT)si.nPage <= si.nMax);*/

		headerLayout.prc = &rc;
		headerLayout.pwpos = &headerPos;
					
		if (FALSE != SendMessageW(hwndHeader, HDM_LAYOUT, 0, (LPARAM)&headerLayout))
		{
			headerPos.flags |= ((SWP_NOREDRAW | SWP_NOCOPYBITS) & windowPos->flags);
			SetWindowPos(hwndHeader, headerPos.hwndInsertAfter, headerPos.x, headerPos.y, 
						headerPos.cx, headerPos.cy, headerPos.flags);
		}

		SetWindowPos(hwndList, NULL, rc.left, rc.top, rc.right - rc.left/* + (bScrollVisible ? 16 : 0)*/, rc.bottom - rc.top,
					SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);

		UpdateScrollInfo(hwnd, SIF_RANGE | SIF_POS, bRedraw);
	}
	
	NMHDR hdr; 
	hdr.code = LVN_EX_SIZECHANGED;
	hdr.hwndFrom = hwnd;
	hdr.idFrom = GetWindowLongPtrW(hwnd,GWLP_ID);
	SendMessageW(GetParent(hwnd), WM_NOTIFY, hdr.idFrom, (LPARAM)&hdr);
}

static void
HeaderIconList_OnVertScroll(HWND hwnd, INT actionLayout, INT trackPosition, HWND scrollBar)
{
	SCROLLINFO si={sizeof(si),SIF_TRACKPOS | SIF_POS | SIF_RANGE | SIF_PAGE,0};
	if (GetScrollInfo(hwnd,SB_VERT,&si))
	{
		int pos(0), itemHeight;
		BOOL bRedraw(TRUE);
		HWND hwndList;
		
		hwndList = GetDlgItem(hwnd, 2);
		if (!hwndList) 
			return;

		itemHeight = HIWORD(SendMessageW(hwndList, LVM_GETITEMSPACING, 0, 0L));
		
		if (si.nPos > (si.nMax - (INT)si.nPage)) 
			si.nPos = si.nMax - si.nPage;

		switch(actionLayout)
		{
			case SB_TOP:			pos = si.nMin; break;
			case SB_BOTTOM:			pos = si.nMax; break;
			case SB_LINEDOWN:		pos = si.nPos + itemHeight/2; break;
			case SB_LINEUP:			pos = si.nPos - itemHeight/2; break;
			case SB_PAGEDOWN:		pos = si.nPos + si.nPage - (((INT)si.nPage > itemHeight && 0 == ((INT)si.nPage%itemHeight)) ? itemHeight : 0);break;
			case SB_PAGEUP:			pos = si.nPos - si.nPage - (((INT)si.nPage > itemHeight && 0 == ((INT)si.nPage%itemHeight)) ? itemHeight : 0);break;
			case SB_THUMBPOSITION:	
			case SB_THUMBTRACK:		
				{
					POINT pt;
					if (!SendMessageW(hwndList, LVM_GETORIGIN, 0, (LPARAM)&pt)) 
						return;
					si.nPos = pt.y;
					pos = si.nTrackPos;
					bRedraw = FALSE;
				}
				break; 
			case SB_ENDSCROLL:		MLSkinnedScrollWnd_UpdateBars(hwnd, TRUE); return;
			default:				pos = si.nPos;
		}
	
		if (pos < si.nMin) pos = si.nMin;
		if (pos > (si.nMax - (INT)si.nPage)) pos = si.nMax - si.nPage;
		if (pos != si.nPos)
		{
			BOOL br;
			if (!bRedraw)
			{
				UpdateWindow(GetDlgItem(hwnd, 3));
				UpdateWindow(hwnd);
				SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0L);
			}
			br = (BOOL)SendMessageW(hwndList, LVM_SCROLL, 0, pos - si.nPos);
			if (!bRedraw) SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0L);
			if (br)
			{
				si.fMask = SIF_POS;
				si.nPos = pos;
				SetScrollInfo(hwnd, SB_VERT, &si, FALSE);

				if (!bRedraw) 
					InvalidateRect(hwndList, NULL, TRUE);
			}
		}
	}
}

static void
HeaderIconList_OnSetFont(HWND hwnd, HFONT font, BOOL redraw)
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
	
		UpdateFontMetrics(hwnd, redraw);
	}
}

static LRESULT
HeaderIconList_OnGetFont(HWND hwnd)
{
	HWND listWindow;

	listWindow = GetDlgItem(hwnd, 2);
	if (NULL != listWindow) 
		return SendMessageW(listWindow, WM_GETFONT, 0, 0L);
			
	return DefWindowProcW(hwnd, WM_GETFONT, 0, 0L);
}

static void
HeaderIconList_OnSetRedraw(HWND hwnd, BOOL enableRedraw)
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
HeaderIconList_OnSkinUpdated(HWND hwnd, BOOL notifyChildren, BOOL redraw)
{
	UpdateFontMetrics(hwnd, redraw);
}

static LRESULT
HeaderIconList_OnDisplaySort(HWND hwnd, int sortIndex, BOOL ascendingOrder)
{
	HWND headerWindow;

	headerWindow = GetDlgItem(hwnd, 3);
	if (NULL == headerWindow)
		return 0;

	return SENDMLIPC(headerWindow, ML_IPC_SKINNEDHEADER_DISPLAYSORT, MAKEWPARAM(sortIndex, ascendingOrder));
}

static LRESULT
HeaderIconList_OnGetSort(HWND hwnd)
{
	HWND headerWindow;

	headerWindow = GetDlgItem(hwnd, 3);
	if (NULL == headerWindow)
		return 0;

	return SENDMLIPC(headerWindow, ML_IPC_SKINNEDHEADER_GETSORT, 0);
}


static LRESULT
HeaderIconList_OnMediaLibraryIPC(HWND hwnd, INT msg, INT_PTR param)
{
	switch(msg)
	{
		case ML_IPC_SKINNEDWND_SKINUPDATED:			HeaderIconList_OnSkinUpdated(hwnd, LOWORD(param), HIWORD(param)); break;
		case ML_IPC_SKINNEDLISTVIEW_DISPLAYSORT: 	return HeaderIconList_OnDisplaySort(hwnd, LOWORD(param), HIWORD(param));
		case ML_IPC_SKINNEDLISTVIEW_GETSORT:		return HeaderIconList_OnGetSort(hwnd); 
	}
	return 0;
}

static LRESULT CALLBACK HeaderIconList(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) 
	{
		case WM_CREATE:				return HeaderIconList_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:			HeaderIconList_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED:	HeaderIconList_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_VSCROLL:			HeaderIconList_OnVertScroll(hwnd, LOWORD(wParam), (short)HIWORD(wParam), (HWND)lParam); return 0;
		case WM_ERASEBKGND: 		return 1;
		case WM_SETFONT:			HeaderIconList_OnSetFont(hwnd, (HFONT)wParam, LOWORD(lParam)); return 0; 
		case WM_GETFONT:			return HeaderIconList_OnGetFont(hwnd);
		case WM_SETREDRAW:			HeaderIconList_OnSetRedraw(hwnd, (BOOL)wParam); return 0;
		case WM_TIMER:
			if (43 == wParam)
			{
				KillTimer(hwnd, 43);
				PostMessageW(hwnd, WM_EX_UPDATESCROLLINFO, SIF_POS, FALSE);
				PostMessageW(hwnd, WM_EX_UNLOCKREDRAW, IWF_CONTAINER | IWF_LISTVIEW | IWF_UPDATENOW, 0L);
			}
			break;

		case WM_EX_UPDATESCROLLINFO:
			return UpdateScrollInfo(hwnd, (UINT)wParam, (BOOL)lParam);

		case WM_EX_UNLOCKREDRAW:
			
			SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0L);
			if (IWF_CONTAINER & wParam) { InvalidateRect(hwnd, NULL, FALSE); if (IWF_UPDATENOW & wParam) UpdateWindow(hwnd); }
			if (IWF_LISTVIEW & wParam) 
			{
				HWND hwndList = GetDlgItem(hwnd, 2);
				if(hwndList) { InvalidateRect(hwndList, NULL, TRUE);  if (IWF_UPDATENOW & wParam) UpdateWindow(hwndList); }
			}
			if (IWF_HEADER & wParam) 
			{
				HWND hwndHeader = GetDlgItem(hwnd, 3);
				if(hwndHeader) { InvalidateRect(hwndHeader, NULL, TRUE);  if (IWF_UPDATENOW & wParam) UpdateWindow(hwndHeader); }
			}
			break;
		case LVM_GETHEADER:
			return (LRESULT)GetDlgItem(hwnd,3);
		case LVM_DELETECOLUMN:
			{
				HWND headerWindow;
				headerWindow = GetDlgItem(hwnd,3);
				if (NULL != headerWindow)
					return SendMessageW(headerWindow, HDM_DELETEITEM, wParam, 0L);
			}
			return FALSE;
		case LVM_GETCOLUMNWIDTH:
			{
				HWND headerWindow;
				headerWindow = GetDlgItem(hwnd,3);
				if (NULL != headerWindow)
				{
					HDITEMW headerItem;
					headerItem.mask = HDI_WIDTH;
					if (FALSE != SendMessageW(headerWindow, HDM_GETITEM, wParam, (LPARAM)&headerItem))
						return headerItem.cxy;
				}
			}
			return 0;
		case LVM_INSERTCOLUMNA:
		case LVM_INSERTCOLUMNW:
			{
				LVCOLUMNW *listColumn = (LVCOLUMNW*)lParam;
				HDITEMW headerItem;
				HWND headerWindow;
				headerWindow = GetDlgItem(hwnd,3);
				if (NULL == headerWindow)
					return -1;

				if (FALSE == CopyListColumnToHeaderItem(listColumn, &headerItem))
					return -1;

				if (0 == (HDI_FORMAT & headerItem.mask))
				{
					headerItem.mask |= HDI_FORMAT;
					headerItem.fmt = HDF_LEFT;
				}

				return SendMessageW(headerWindow, 
							(LVM_INSERTCOLUMNW == uMsg) ? HDM_INSERTITEMW : HDM_INSERTITEMA,
							wParam,	(LPARAM)&headerItem);
			}
			break;
		case LVM_SETITEMCOUNT:
			{
				HWND hwndList = GetDlgItem(hwnd, 2);
				if (hwndList) 
				{
					LRESULT lr = ListView_WindowProc(hwndList, uMsg, wParam, lParam);
					UpdateScrollInfo(hwnd, SIF_RANGE | SIF_POS, TRUE);
					return lr;
				}
			}
			break;
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

		case LVM_SETCOLUMNW:
		case LVM_SETCOLUMNA:
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
								(LVM_SETCOLUMNW == uMsg) ? HDM_SETITEMW : HDM_SETITEMA, 
								wParam, (LPARAM)&h))
				{
					return FALSE;
				}
				
				if (FALSE == CopyHeaderItemToListColumn(&h, l))
					return FALSE;
			}
			return TRUE;
		case LVM_SETCOLUMNWIDTH:
			{
				HWND headerWindow;
				HDITEMW headerItem;

				headerWindow = GetDlgItem(hwnd, 3);
				if (NULL == headerWindow)
					return FALSE;

				if (LVSCW_AUTOSIZE == lParam)
					return FALSE;

				if (LVSCW_AUTOSIZE_USEHEADER == lParam)
					return FALSE;

				headerItem.mask = HDI_WIDTH;
				headerItem.cxy = (int)lParam;

				return SendMessageW(headerWindow, HDM_SETITEMW, (WPARAM)wParam, (LPARAM)&headerItem);
			}
			break;
		case WM_NOTIFY:
			{
				BOOL bPost(FALSE);
				LPNMHDR l=(LPNMHDR)lParam;
				if(l->idFrom == 2) 
				{
					switch(l->code)
					{
						case LVN_ODFINDITEMA:
						case LVN_ODFINDITEMW:
							bPost = TRUE;
							break;
						case LVN_ODSTATECHANGED:
							{
								NMLVODSTATECHANGE *stateChange;
								stateChange = (NMLVODSTATECHANGE*)lParam;
								
								if (0 != (LVIS_SELECTED & (stateChange->uNewState ^ stateChange->uOldState)))
								{
									
									SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0L);
									ListView_ScheduleSelectionUpdate(l->hwndFrom, stateChange->iFrom,stateChange->iTo); 
								}

								return 0;
							}
							break;
					}
					l->idFrom = GetWindowLong(hwnd,GWL_ID);
					l->hwndFrom = hwnd;
					LRESULT lr = SendMessageW(GetParent(hwnd),uMsg,l->idFrom,lParam);
					if (bPost)
					{
						UpdateWindow(GetDlgItem(hwnd, 3));
						SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0L);
						PostMessageW(hwnd, WM_EX_UPDATESCROLLINFO, SIF_POS, TRUE);
						PostMessageW(hwnd, WM_EX_UNLOCKREDRAW, IWF_CONTAINER | IWF_LISTVIEW, 0L);
					}
					return lr;
				}
				else if(l->idFrom == 3) {
					switch(l->code) {
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
					}
					return SendMessageW(GetParent(hwnd),uMsg, wParam,lParam);
				}
			}
			break;
		case WM_EX_GETREALLIST: 
			return (LRESULT)GetDlgItem(hwnd, 2);
		case WM_EX_GETCOUNTPERPAGE: 
			{
				HWND hwndList;
				RECT rc;
				hwndList = GetDlgItem(hwnd, 2);
				if (hwndList)
				{
					GetClientRect(hwndList, &rc);
					OffsetRect(&rc, -rc.left, -rc.top);
					DWORD spacing = (DWORD)SendMessageW(hwndList, LVM_GETITEMSPACING, 0, 0L);
					if (LOWORD(spacing) && HIWORD(spacing))
					{					
						return (rc.right/LOWORD(spacing)) * (rc.bottom /HIWORD(spacing) + ((rc.bottom%HIWORD(spacing)) ? 1 : 0));
					}
					
				}
			}
			return 0;

		case WM_ML_IPC:
			return HeaderIconList_OnMediaLibraryIPC(hwnd, (INT)lParam, (INT_PTR)wParam);

		default:
			if(uMsg >= LVM_FIRST && uMsg < LVM_FIRST + 0x100) 
			{
				HWND hwndList = GetDlgItem(hwnd,2);
				if (hwndList) return ListView_WindowProc(hwndList, uMsg, wParam, lParam);
			}
			break;
	}
	return DefWindowProcW(hwnd,uMsg,wParam,lParam);
}


void InitHeaderIconList() {
	WNDCLASSW wc = {0, };

	if (GetClassInfoW(plugin.hDllInstance, L"HeaderIconList", &wc)) return;
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = HeaderIconList;
	wc.hInstance = plugin.hDllInstance;
	wc.lpszClassName = L"HeaderIconList";
	RegisterClassW(&wc);
}