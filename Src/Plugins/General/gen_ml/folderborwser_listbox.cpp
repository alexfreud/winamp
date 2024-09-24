#include "./folderbrowser.h"
#include "./folderbrowser_internal.h"

#include "../nu/CGlobalAtom.h"


static CGlobalAtom NAVMGR_FBLISTBOXW(L"FBLISTBOX");

extern size_t FolderBrowser_GetListBoxColumn(HWND hwndList);
static LRESULT CALLBACK FBListBox_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct _FBMULTISELECT
{
	HWND hOwner;
	INT nStart;
	INT nTrack;
	POINTS ptStart;
} FBMULTISELECT;

static HWND g_hDragSource = NULL;
static FBMULTISELECT g_MultiSelect = {NULL, };

BOOL FolderBrowser_CustomizeListBox(HWND hwndListbox)
{
	LONG_PTR oldProc = GetWindowLongPtrW(hwndListbox, GWLP_WNDPROC);
	if (!oldProc || !SetPropW(hwndListbox, NAVMGR_FBLISTBOXW, (HANDLE)oldProc)) return FALSE;
	SetWindowLongPtrW(hwndListbox, GWLP_WNDPROC, (LONGX86)(LONG_PTR)FBListBox_WindowProc);
	return TRUE;
}

static void ResetMultiSelect(FBMULTISELECT *pms)
{
	pms->hOwner = NULL;
	pms->nTrack = -1;
	pms->nStart = -1;
	pms->ptStart.x = -1;
	pms->ptStart.y = -1;
}

static void EnsureFocused(HWND hwnd)
{
	HWND hFocus = GetFocus();
	if (hwnd != hFocus)
	{
		SetFocus(hwnd);
		HWND hParent = GetParent(hwnd);
		if (NULL != hParent)
			SendMessageW(hParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), LBN_SETFOCUS), (LPARAM)hwnd);
	}
}

static void EmulateSelectionChanged(HWND hwnd, INT caretIndex, INT ctrlId)
{
	HWND hParent = GetParent(hwnd);
	SendMessageW(hwnd, LB_SETCARETINDEX, (WPARAM)caretIndex, TRUE);
	if (NULL != hParent)
		SendMessageW(hParent, WM_COMMAND, MAKEWPARAM(ctrlId, LBN_SELCHANGE), (LPARAM)hwnd);
}

static BOOL FBListBox_OnLButtonDown(HWND hwnd, UINT uFlags, POINTS pts)
{
	g_hDragSource = NULL;
	ResetMultiSelect(&g_MultiSelect);

	INT count = (INT)SendMessageW(hwnd, LB_GETCOUNT, 0, 0L);
	if (count > 0)
	{
		DWORD item = (DWORD)SendMessageW(hwnd, LB_ITEMFROMPOINT, 0, *((LPARAM*)&pts));
		if (HIWORD(item)) 
		{
			if (0 == (MK_SHIFT & uFlags))
			{
				EnsureFocused(hwnd);
				SendMessageW(hwnd, LB_SETSEL, FALSE, (LPARAM)-1);
				EmulateSelectionChanged(hwnd, (INT)SendMessageW(hwnd, LB_GETCARETINDEX, 0, 0L), GetDlgCtrlID(hwnd));
			}
			g_MultiSelect.hOwner = hwnd;
			g_MultiSelect.nStart = LOWORD(item);
			g_MultiSelect.nTrack = item;
			g_MultiSelect.ptStart = pts;
			SetCapture(hwnd);

			return TRUE;
		}
		item = LOWORD(item);
		INT sel = (INT)SendMessageW(hwnd, LB_GETSEL, item, 0L);
		if (sel > 0) 
		{ 
			EnsureFocused(hwnd);
			g_hDragSource = hwnd; 
			return TRUE; 
		}

		RECT rc;
		if (SendMessageW(hwnd, LB_GETITEMRECT, (WPARAM)item, (LPARAM)&rc))
		{
			MEASUREITEMSTRUCT mis;
			ZeroMemory(&mis, sizeof(MEASUREITEMSTRUCT));
			mis.CtlID = GetDlgCtrlID(hwnd);
			mis.CtlType = ODT_LISTBOX;
			mis.itemID = item;
			mis.itemData = (DWORD)SendMessageW(hwnd, LB_GETITEMDATA, (WPARAM)item, 0L);
			if (SendMessageW(GetParent(hwnd), WM_MEASUREITEM, mis.CtlID, (LPARAM)&mis))
			{
				rc.right = rc.left + mis.itemWidth;
				POINT pt;
				POINTSTOPOINT(pt, pts);
				if (PtInRect(&rc, pt))
				{
					EnsureFocused(hwnd);
					if (0 == ((MK_CONTROL | MK_SHIFT) & uFlags)) SendMessageW(hwnd, LB_SETSEL, FALSE, (LPARAM)-1);
					if (MK_SHIFT & uFlags)
					{
						INT anchor = (INT)SendMessageW(hwnd, LB_GETANCHORINDEX, 0, 0L);
						INT start = (anchor < (INT)item) ? anchor : item;
						INT stop = (start == anchor) ? item : anchor;
						SendMessageW(hwnd, LB_SETSEL, FALSE, (LPARAM)-1);
                        SendMessageW(hwnd, LB_SELITEMRANGEEX, (WPARAM)start, (LPARAM)stop);
						SendMessageW(hwnd, LB_SETANCHORINDEX, (WPARAM)anchor, 0L);
					}
					else
					{
						SendMessageW(hwnd, LB_SETSEL, (0 == (MK_CONTROL & uFlags) || 0 == sel), (LPARAM)item);
					}

					EmulateSelectionChanged(hwnd, item, mis.CtlID);
					return TRUE;
				}
			}
		}
	}
	else 
	{
		EnsureFocused(hwnd);
		return TRUE;
	}

	return FALSE;
}

static BOOL FBListBox_OnLButtonUp(HWND hwnd, UINT uFlags, POINTS pts)
{
	ResetMultiSelect(&g_MultiSelect);

	if (hwnd == GetCapture())
		ReleaseCapture();

	INT count = (INT)SendMessageW(hwnd, LB_GETCOUNT, 0, 0L);
	if (count > 0)
	{
		DWORD item = (DWORD)SendMessageW(hwnd, LB_ITEMFROMPOINT, 0, *((LPARAM*)&pts));
		if (HIWORD(item)) return FALSE;
		item = LOWORD(item);
		INT sel = (INT)SendMessageW(hwnd, LB_GETSEL, item, 0L);
		if (sel > 0 && hwnd == g_hDragSource)
		{ 
			if ((INT)SendMessageW(hwnd, LB_GETSELCOUNT, item, 0L) > 0)
			{
				SendMessageW(hwnd, LB_SETSEL, FALSE, (LPARAM)-1);
				SendMessageW(hwnd, LB_SETSEL, TRUE, (LPARAM)item);
				EmulateSelectionChanged(hwnd, item, GetDlgCtrlID(hwnd));
			}
		}
	}

	g_hDragSource = NULL;

	return FALSE;
}

static BOOL FBListBox_OnMouseMove(HWND hwnd, UINT uFlags, POINTS pts)
{
	if (g_hDragSource == hwnd) return TRUE; 
	if (g_MultiSelect.hOwner == hwnd)
	{
		RECT rc;
		if (GetClientRect(hwnd, &rc))
			pts.x = (SHORT)(rc.left + (rc.right - rc.left)/2);

		DWORD item = (DWORD)SendMessageW(hwnd, LB_ITEMFROMPOINT, 0, *((LPARAM*)&pts));
		BOOL bNotify = FALSE;
		
		if (item != g_MultiSelect.nTrack)
		{
			INT nCurrent = LOWORD(item);

			INT nTrack = LOWORD(g_MultiSelect.nTrack);
			if (abs(nCurrent - g_MultiSelect.nStart) < abs(nTrack - g_MultiSelect.nStart))
			{
				INT first = (nTrack < g_MultiSelect.nStart) ? g_MultiSelect.nStart : nTrack;
				INT last = (first == nTrack) ? g_MultiSelect.nStart : nTrack;
				LRESULT r = SendMessageW(hwnd, LB_SELITEMRANGEEX, (WPARAM)first, (LPARAM)last);
				if (!bNotify) bNotify = (LB_ERR != r);
			}

			if (nCurrent != g_MultiSelect.nStart)
			{
				INT first = (nCurrent > g_MultiSelect.nStart) ? g_MultiSelect.nStart : nCurrent;
				INT last = (first == nCurrent) ? g_MultiSelect.nStart : nCurrent;

				LRESULT r = SendMessageW(hwnd, LB_SELITEMRANGEEX, (WPARAM)first, (LPARAM)last);
				if (!bNotify) bNotify = (LB_ERR != r);
			}
			else if (0 == HIWORD(item))
			{
				LRESULT r = SendMessageW(hwnd, LB_SETSEL, TRUE, (LPARAM)nCurrent);
				if (!bNotify) bNotify = (LB_ERR != r);
			}
			g_MultiSelect.nTrack = (INT)item;
		}
		else if (0 != HIWORD(g_MultiSelect.nTrack) && 0 != HIWORD(item) && 
				LOWORD(item) == g_MultiSelect.nStart &&
				(INT)SendMessageW(hwnd, LB_GETSEL, (WPARAM)g_MultiSelect.nStart, 0L) > 0)
		{
			BOOL bReset = TRUE;
			if (1 == (INT)SendMessageW(hwnd, LB_GETCOUNT, 0, 0L))
			{
				RECT ri;
				if (LB_ERR != SendMessageW(hwnd, LB_GETITEMRECT, (WPARAM)g_MultiSelect.nStart, (LPARAM)&ri))
				{
					LONG t = (pts.y < g_MultiSelect.ptStart.y) ? pts.y : g_MultiSelect.ptStart.y;
					LONG b = (t != pts.y) ? pts.y : g_MultiSelect.ptStart.y;
					bReset = !(t < ri.bottom && b > ri.top);
				}
			}
			if (bReset)
			{
				LRESULT r = SendMessageW(hwnd, LB_SETSEL, FALSE, (LPARAM)g_MultiSelect.nStart);
				if (!bNotify) bNotify = (LB_ERR != r);
			}
		}

		if (bNotify)
		{
			HWND hParent = GetParent(hwnd);
			if (NULL != hParent)
					SendMessageW(hParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), LBN_SELCHANGE), (LPARAM)hwnd);
		}
	}
	return FALSE;
}

static BOOL FBListBox_OnMouseWheel(HWND hwnd, UINT vkCode, INT delta, POINTS pts)
{
	if (MK_CONTROL == vkCode)
	{
		PostMessageW(GetParent(hwnd), WM_MOUSEWHEEL, MAKEWPARAM(vkCode, delta), MAKELPARAM(pts.x, pts.y));
		return TRUE;
	}
	return FALSE;
}

static BOOL FBListBox_OnKeyDown(HWND hwnd, UINT vkCode, UINT flags)
{
	UINT uiState = 0;
	switch(vkCode)
	{
		case VK_CONTROL:
		case VK_SHIFT:
			return FALSE;
		case VK_MENU:
			uiState = (UINT)SendMessageW(hwnd, WM_QUERYUISTATE, 0, 0L);
			if (UISF_HIDEACCEL & uiState)
				SendMessageW(hwnd, WM_CHANGEUISTATE, MAKELONG(UIS_CLEAR, UISF_HIDEACCEL), 0L);
			return FALSE;
	}

	uiState = (UINT)SendMessageW(hwnd, WM_QUERYUISTATE, 0, 0L);
	if (UISF_HIDEFOCUS & uiState)
	{
		SendMessageW(hwnd, WM_CHANGEUISTATE, MAKELONG(UIS_CLEAR, UISF_HIDEFOCUS), 0L);
	}

	if (0 != (0x80000000 & GetAsyncKeyState(VK_CONTROL)))
	{
		switch(vkCode)
		{
			case VK_UP:
			case VK_DOWN:
			case VK_PRIOR:
			case VK_NEXT:
			case VK_END:
			case VK_HOME:
				{
					INT count = (INT)SendMessageW(hwnd, LB_GETCOUNT, 0, 0L);
					INT caret = (INT)SendMessageW(hwnd, LB_GETCARETINDEX, 0, 0L);
					INT caretNew = caret;
					INT page = 0;
					if (count > 0)
					{
						RECT rc;
						INT h = (INT)SendMessageW(hwnd, LB_GETITEMHEIGHT, 0, 0L);
						if (h != -1 && GetClientRect(hwnd, &rc))
						{
							page = (rc.bottom - rc.top)/(h) - 1;
							if (page < 1) page = 1;
						}
					}
					switch(vkCode)
					{
						case VK_UP:		caretNew = caret - 1; break;
						case VK_DOWN:	caretNew = caret + 1; break;
						case VK_PRIOR:	caretNew = caret - page; break;
						case VK_NEXT:	caretNew = caret + page; break;
						case VK_END:	caretNew = count - 1; break;
						case VK_HOME:	caretNew = 0; break;
					}
					if (caretNew < 0) caretNew = 0;
					if (caretNew >= count) caretNew = count - 1;
					if (caret != caretNew && -1 != caretNew) 
						SendMessageW(hwnd, LB_SETCARETINDEX, (WPARAM)caretNew, FALSE);
				}
				return TRUE;
			case VK_SPACE:
				{
					INT caret = (INT)SendMessageW(hwnd, LB_GETCARETINDEX, 0, 0L);
					if (-1 != caret)
					{
						INT selected = (INT)SendMessageW(hwnd, LB_GETSEL, caret, 0L);
						SendMessageW(hwnd, LB_SETSEL, (0 == selected), (LPARAM)caret);
						HWND hParent = GetParent(hwnd);
						if (NULL != hParent)
							SendMessageW(hParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), LBN_SELCHANGE), (LPARAM)hwnd);
					}
				}
				return TRUE;
		}
	}
	return FALSE;
}

static BOOL FBListBox_OnNcHitTest(HWND hwnd, POINTS pts, LRESULT *pResult)
{
	if (SIZER_OVERLAP_LEFT > 0 || SIZER_OVERLAP_RIGHT > 0)
	{
		RECT rw;
		if (GetWindowRect(hwnd, &rw))
		{
			if ((SIZER_OVERLAP_RIGHT > 0 && pts.x >= rw.left && pts.x <= (rw.left + SIZER_OVERLAP_RIGHT)) ||
				(SIZER_OVERLAP_LEFT > 0 && pts.x <= rw.right && pts.x >= (rw.right - SIZER_OVERLAP_LEFT)))
			{
				*pResult = HTTRANSPARENT;
				return TRUE;
			}
		}
	}
	return FALSE;
}

static LRESULT CALLBACK FBListBox_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC fnOriginalProc = (WNDPROC)GetPropW(hwnd, NAVMGR_FBLISTBOXW);
	if (!fnOriginalProc) return DefWindowProcW(hwnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
		case WM_NCDESTROY:
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)fnOriginalProc);
			RemovePropW(hwnd, NAVMGR_FBLISTBOXW);
			return CallWindowProcW(fnOriginalProc, hwnd, uMsg, wParam, lParam);
		case WM_LBUTTONDOWN:
			if (FBListBox_OnLButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam))) return 0;
			break;
		case WM_LBUTTONUP:
			if (FBListBox_OnLButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam))) return 0;
			break;
		case WM_MOUSEWHEEL:
			if (FBListBox_OnMouseWheel(hwnd, GET_KEYSTATE_WPARAM(wParam), GET_WHEEL_DELTA_WPARAM(wParam), MAKEPOINTS(lParam))) return 0;
			break;
		case WM_MOUSEMOVE:
			if (FBListBox_OnMouseMove(hwnd, (UINT)wParam, MAKEPOINTS(lParam))) return 0;
			break;
		case WM_KEYDOWN:
			if (FBListBox_OnKeyDown(hwnd, (UINT)wParam, (UINT)lParam)) return 0;
			break;
		case WM_UPDATEUISTATE:
			InvalidateRect(hwnd, NULL, FALSE);
			break;
		case WM_NCHITTEST:
			{
				LRESULT lResult;
				if (FBListBox_OnNcHitTest(hwnd, MAKEPOINTS(lParam), &lResult)) 
					return lResult;
			}
			break;
	}
	return CallWindowProcW(fnOriginalProc, hwnd, uMsg, wParam, lParam);
}