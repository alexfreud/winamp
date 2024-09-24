#include "./skinnedlistview.h"
#include "../winamp/gen.h"
#include "../winamp/wa_dlg.h"
#include "./skinning.h"
#include "../nu/trace.h"
#include "config.h"

#ifndef LVS_EX_DOUBLEBUFFER		//this will work XP only
#define LVS_EX_DOUBLEBUFFER		0x00010000
#endif

// internal flags
#define LVIF_ENABLED			0x0001
#define LVIF_FOCUSED			0x0002
#define LVIF_HEADERATTACHED		0x0100
#define LVIF_REMOVEREFLECTOR	0x0200

extern "C" winampGeneralPurposePlugin plugin;

SkinnedListView::SkinnedListView(void) : SkinnedScrollWnd(FALSE), listFlags(0), currentItem(-1)
{
	currentColor = SendMessageW(plugin.hwndParent, WM_WA_IPC, 4, IPC_GET_GENSKINBITMAP);
}

SkinnedListView::~SkinnedListView(void)
{
	if (NULL != hwnd && 
		0 != (LVIF_REMOVEREFLECTOR & listFlags))
	{
		HWND hParent = GetParent(hwnd);
		RemoveReflector(hParent);
	}
}

BOOL SkinnedListView::Attach(HWND hwndListView)
{
	HWND hwndParent;
	listFlags = 0x000;
	if(!SkinnedScrollWnd::Attach(hwndListView)) return FALSE;

	SetType(SKINNEDWND_TYPE_LISTVIEW);
	SetMode(SCROLLMODE_LISTVIEW);

	hwndParent = GetParent(hwndListView);
	if (NULL != hwndParent && S_OK == InstallReflector(hwndParent))
		listFlags |= LVIF_REMOVEREFLECTOR;

	TryAttachHeader();

	SendMessageW(hwnd, CCM_SETVERSION, 5, 80); 

	return TRUE;
}

void SkinnedListView::OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw)
{
	if (SWS_USESKINCOLORS & style)
	{
		DisableRedraw();

		currentColor = SendMessageW(plugin.hwndParent, WM_WA_IPC, 4, IPC_GET_GENSKINBITMAP);
		SendMessageW(hwnd, LVM_SETTEXTCOLOR, 0, (LPARAM)WADlg_getColor(WADLG_ITEMFG));
		SendMessageW(hwnd, LVM_SETTEXTBKCOLOR, 0, (LPARAM)WADlg_getColor(WADLG_ITEMBG));
		SendMessageW(hwnd, LVM_SETBKCOLOR, 0, (LPARAM)WADlg_getColor(WADLG_ITEMBG));

		EnableRedraw(SWR_NONE);
	}

	__super::OnSkinChanged(bNotifyChildren, bRedraw);
}

BOOL SkinnedListView::SetStyle(UINT newStyle, BOOL bRedraw)
{
	BOOL succeeded;

	succeeded = __super::SetStyle(newStyle, bRedraw);

	if (hwnd)
	{
		UINT lvStyle(0);
		if(SWLVS_FULLROWSELECT & newStyle) lvStyle |= LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
		if(SWLVS_DOUBLEBUFFER & newStyle) lvStyle |= LVS_EX_DOUBLEBUFFER;
		if (0 != lvStyle)
			ListView_SetExtendedListViewStyleEx(hwnd, lvStyle, lvStyle);

		if (0 != (SWS_USESKINCOLORS & style))
		{
			HWND tooltipWindow;
			tooltipWindow = (HWND)SendMessageW(hwnd, LVM_GETTOOLTIPS, 0, 0L);
			if (NULL != tooltipWindow)
			{
				unsigned int skinStyle;

				skinStyle = SWS_USESKINCOLORS;
				skinStyle |= ((SWS_USESKINFONT | SWS_USESKINCURSORS) & style);
				SkinWindowEx(tooltipWindow, SKINNEDWND_TYPE_TOOLTIP, skinStyle);
			}
		}
	}
	return succeeded;
}

BOOL SkinnedListView::OnCustomDraw(HWND hwndFrom, NMLVCUSTOMDRAW *plvcd, LRESULT *pResult)
{
	static COLORREF rgbBk, rgbFg, rgbBkSel, rgbFgSel;
	static BOOL restoreSelect(FALSE), bFullRowSelect(FALSE); 
	BOOL bSelected;

	switch(plvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:	
			
			listFlags &= ~0x00FF;
			if (IsWindowEnabled(hwnd)) listFlags |= LVIF_ENABLED;
			if (GetFocus() == hwnd || GetForegroundWindow() == hwnd) 
				listFlags |= LVIF_FOCUSED;
						
			if (MLSkinnedWnd_EnableReflection(hwndFrom, FALSE))
			{
				 *pResult = SendMessageW(hwndFrom, WM_NOTIFY, (WPARAM)plvcd->nmcd.hdr.idFrom, (LPARAM)plvcd);
				 MLSkinnedWnd_EnableReflection(hwndFrom, TRUE);
			}
			else *pResult = CDRF_DODEFAULT;
			
			if (plvcd->nmcd.rc.bottom != 0 && plvcd->nmcd.rc.right != 0) *pResult |= CDRF_NOTIFYITEMDRAW;
			bFullRowSelect = ( LVS_REPORT != (LVS_TYPEMASK & GetWindowLongPtrW(hwnd, GWL_STYLE)) ||
							0 != (LVS_EX_FULLROWSELECT & CallPrevWndProc(LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0L)));
			return TRUE;
        
		// Modify item text and or background
		case CDDS_ITEMPREPAINT:
			bSelected = FALSE;
			if(LVIF_ENABLED & listFlags)
			{
				bSelected = (LVIS_SELECTED == CallPrevWndProc(LVM_GETITEMSTATE, plvcd->nmcd.dwItemSpec, LVIS_SELECTED));
				if (bSelected)
				{
					if(LVIF_FOCUSED & listFlags)
					{
						rgbFgSel = WADlg_getColor(WADLG_SELBAR_FGCOLOR);
						rgbBkSel = WADlg_getColor(WADLG_SELBAR_BGCOLOR);
					} 
					else 
					{
						rgbFgSel = WADlg_getColor(WADLG_INACT_SELBAR_FGCOLOR);
						rgbBkSel = WADlg_getColor(WADLG_INACT_SELBAR_BGCOLOR);
					}
					if (bFullRowSelect)
					{
						rgbBk = rgbBkSel;
						rgbFg = rgbFgSel;
					}
				}
				else
				{
					if (plvcd->nmcd.dwItemSpec%2 && (style&SWLVS_ALTERNATEITEMS) && config_use_alternate_colors)
					{
						rgbFg = WADlg_getColor(WADLG_ITEMFG2);
						rgbBk = WADlg_getColor(WADLG_ITEMBG2);
					}
					else
					{
						rgbBk = plvcd->clrTextBk;
						rgbFg = plvcd->clrText;
					}
				}
			}
			else
			{
				#define BLENDER(fg,bg) (RGB((GetRValue(fg)+GetRValue(bg))/2,(GetGValue(fg)+GetGValue(bg))/2,(GetBValue(fg)+GetBValue(bg))/2))
				rgbFg = BLENDER(WADlg_getColor(WADLG_INACT_SELBAR_FGCOLOR),WADlg_getColor(WADLG_WNDBG));
				rgbBk = WADlg_getColor(WADLG_ITEMBG);
			}

			plvcd->clrTextBk = rgbBk;

			if (plvcd->nmcd.dwItemSpec == currentItem)
				plvcd->clrText = currentColor;
			else
				plvcd->clrText = rgbFg;

			if (MLSkinnedWnd_EnableReflection(hwndFrom, FALSE))
			{
				 *pResult = SendMessageW(hwndFrom, WM_NOTIFY, (WPARAM)plvcd->nmcd.hdr.idFrom, (LPARAM)plvcd);
				 MLSkinnedWnd_EnableReflection(hwndFrom, TRUE);
			}
			else *pResult = CDRF_DODEFAULT;

			if (!bFullRowSelect) *pResult |= CDRF_NOTIFYSUBITEMDRAW;

			if (bSelected && 0 == (CDRF_SKIPDEFAULT & *pResult))
			{
				plvcd->nmcd.uItemState &= ~CDIS_SELECTED;
				restoreSelect = TRUE;
				*pResult |= CDRF_NOTIFYPOSTPAINT;
			}
			return TRUE;

		case CDDS_ITEMPOSTPAINT:
			if(restoreSelect)
			{
				plvcd->nmcd.uItemState |= CDIS_SELECTED;
				restoreSelect = FALSE;
			}
			break;
		case (CDDS_SUBITEM | CDDS_ITEMPREPAINT):
			if (restoreSelect && (bFullRowSelect || 0 == plvcd->iSubItem))
			{
				plvcd->clrTextBk = rgbBkSel;
				plvcd->clrText = rgbFgSel;
			}
			else 
			{
				plvcd->clrTextBk = rgbBk;
				plvcd->clrText = rgbFg;
			}

			if (plvcd->nmcd.dwItemSpec == currentItem)
				plvcd->clrText = currentColor;

			break;
	}
	return FALSE;
}

BOOL SkinnedListView::OnReflectedNotify(HWND hwndFrom, INT idCtrl, NMHDR *pnmh, LRESULT *pResult)
{
	switch(pnmh->code)
	{
		case NM_CUSTOMDRAW: 
			return OnCustomDraw(hwndFrom, (NMLVCUSTOMDRAW*)pnmh, pResult);
		case LVN_ITEMCHANGED:
			{
				NMLISTVIEW  *plv = (NMLISTVIEW*)pnmh;
				if (((LVIS_SELECTED | LVIS_FOCUSED) & plv->uNewState) != ((LVIS_SELECTED | LVIS_FOCUSED) & plv->uOldState) && 
					plv->iSubItem == 0 && plv->iItem >= 0 &&
					LVS_REPORT == (LVS_TYPEMASK & GetWindowLongPtrW(hwnd, GWL_STYLE)) &&
					0 == (LVS_EX_FULLROWSELECT & CallPrevWndProc(LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0L)))
				{
					RECT rc;
					ListView_GetItemRect(hwnd, plv->iItem, &rc, LVIR_ICON | ((LVIS_FOCUSED & plv->uNewState) ? LVIR_LABEL : 0));
					if (rc.left != rc.right) InvalidateRect(hwnd, &rc, FALSE);
				}
			}
			break;
	}
	return FALSE;
}

LRESULT SkinnedListView::OnEraseBackground(HDC hdc)
{
	HWND hwndHeader;

	hwndHeader = ListView_GetHeader(hwnd);
	if (hdc && hwndHeader && IsWindowVisible(hwndHeader))
	{
		RECT rc;
		if(GetClientRect(hwndHeader, &rc))
		{
			MapWindowPoints(hwndHeader, hwnd, (POINT*)&rc, 2);
			ExcludeClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
		}
	}
	return __super::OnEraseBackground(hdc);
}

BOOL SkinnedListView::OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult)
{
	HWND hwndHeader;
	switch(msg)
	{
		case ML_IPC_SKINNEDLISTVIEW_DISPLAYSORT: 
			hwndHeader = (HWND)CallPrevWndProc(LVM_GETHEADER, 0, 0L);
			*pResult = (hwndHeader) ? SENDMLIPC(hwndHeader, ML_IPC_SKINNEDHEADER_DISPLAYSORT, (WPARAM)param) : 0L;
			return TRUE;
		case ML_IPC_SKINNEDLISTVIEW_GETSORT:	 
			hwndHeader = (HWND)CallPrevWndProc(LVM_GETHEADER, 0, 0L);
			*pResult = (hwndHeader) ? SENDMLIPC(hwndHeader, ML_IPC_SKINNEDHEADER_GETSORT, 0) : 0L;
			return TRUE;
		case ML_IPC_SKINNEDLISTVIEW_SETCURRENT:
			currentItem = (INT)param;
			InvalidateRect(hwnd, NULL, FALSE);
			UpdateWindow(hwnd);
			return TRUE;
	}
	return __super::OnMediaLibraryIPC(msg, param, pResult);
}

static BOOL
SkinnedListView_OverrideEdgeItemNavigation(HWND hwnd, unsigned int vkCode)
{
	int iItem, iNextItem, iTest;

	iItem = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)(LVNI_ALL | LVNI_FOCUSED));
	if (-1 == iItem)
		return FALSE;
			
	if (VK_RIGHT == vkCode)
	{
		iNextItem = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)iItem, (LPARAM)(LVNI_TORIGHT));
		if (iNextItem != iItem)
			return FALSE;

		iNextItem = iItem;
		for(;;)
		{
			iTest = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)iNextItem, (LPARAM)(LVNI_TOLEFT));
			if (-1 == iTest || iTest == iNextItem)
				break;
			iNextItem = iTest;
		}

		iNextItem = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)iNextItem, (LPARAM)(LVNI_BELOW));
		if (iNextItem == iTest)
			return FALSE;
	}
	else if (VK_LEFT == vkCode)
	{
		iNextItem = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)iItem, (LPARAM)(LVNI_TOLEFT));
		if (iNextItem != iItem)
			return FALSE;

		iNextItem = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)iItem, (LPARAM)(LVNI_ABOVE));
		if (-1 != iNextItem && iItem != iNextItem)
		{
			for(;;)
			{
				iTest = (int)SendMessageW(hwnd, LVM_GETNEXTITEM, (WPARAM)iNextItem, (LPARAM)(LVNI_TORIGHT));
				if (-1 == iTest || iTest == iNextItem)
					break;
				iNextItem = iTest;
			}
		}
	}
	else
		return FALSE;

	if (-1 == iNextItem || iItem == iNextItem)
		return FALSE;
	else
	{
		LVITEM item;
		BOOL ctrlPressed, shiftPressed;

		ctrlPressed = (0 != (0x8000 & GetAsyncKeyState(VK_CONTROL)));
		shiftPressed = (0 != (0x8000 & GetAsyncKeyState(VK_SHIFT)));

		if (FALSE == shiftPressed && FALSE == ctrlPressed)
		{
			item.state = 0;
			item.stateMask = LVIS_SELECTED;
			SendMessageW(hwnd, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&item);
		}

		item.stateMask = LVIS_FOCUSED;
		item.state = 0;
		SendMessageW(hwnd, LVM_SETITEMSTATE, (WPARAM)iItem, (LPARAM)&item);

		item.state = LVIS_FOCUSED;
		if (FALSE == ctrlPressed)
		{
			item.state |= LVIS_SELECTED;
			item.stateMask |= LVIS_SELECTED;
		}

		SendMessageW(hwnd, LVM_SETITEMSTATE, (WPARAM)iNextItem, (LPARAM)&item);
		SendMessageW(hwnd, LVM_ENSUREVISIBLE, (WPARAM)iNextItem, (LPARAM)FALSE);
	}
	return TRUE;						
}

void SkinnedListView::OnKeyDown(UINT vkCode, UINT flags)
{
	switch(vkCode)
	{
		case VK_LEFT:
		case VK_RIGHT:
			{
				unsigned long windowStyle;
				windowStyle = GetWindowLongPtrW(hwnd, GWL_STYLE);

				if (LVS_REPORT == (LVS_TYPEMASK & windowStyle))
				{
					if (0 != IsHorzBarHidden())
					{
						HWND hParent = GetParent(hwnd);
						if (NULL != hParent)
						{
							NMLVKEYDOWN lvkd;
							lvkd.hdr.code = LVN_KEYDOWN;
							lvkd.hdr.hwndFrom = hwnd;
							lvkd.hdr.idFrom = (UINT)(UINT_PTR)GetWindowLongPtr(hwnd, GWLP_ID);
							lvkd.wVKey = vkCode;
							SendMessage(hParent, WM_NOTIFY, (WPARAM)lvkd.hdr.idFrom, (LPARAM)&lvkd);
						}
						vkCode = 0;
					}
				}
				else if (LVS_ICON == (LVS_TYPEMASK & windowStyle))
				{
					if (FALSE != SkinnedListView_OverrideEdgeItemNavigation(hwnd, vkCode))
					{
						vkCode = 0;
					}
				}
			}
			break;
		case VK_SPACE:
			if (0 != (SWLVS_SELALWAYS & style))
			{
				UINT windowStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
				if (0 != (LVS_SINGLESEL & windowStyle) && 
					0 != (0x8000 & GetAsyncKeyState(VK_CONTROL)))
				{
					vkCode = 0;	
				}
			}
			break;
	}
	__super::WindowProc(WM_KEYDOWN, (WPARAM)vkCode, (LPARAM)flags);
}

void SkinnedListView::TryAttachHeader(void)
{
	if (0 != (LVIF_HEADERATTACHED & listFlags)) return;
	DWORD ws = GetWindowLongPtrW(hwnd, GWL_STYLE);
	if (LVS_REPORT == (LVS_TYPEMASK & ws) && 0 == (LVS_NOCOLUMNHEADER & ws))
	{
		HWND hHeader = (HWND)CallPrevWndProc(LVM_GETHEADER, 0, 0L);
		if (hHeader) 
		{
			SkinWindow(hHeader, SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);
			listFlags |= LVIF_HEADERATTACHED;
		}
	}
}

void SkinnedListView::OnLButtonDown(UINT uFlags, POINTS pts)
{
	if (0 != (SWLVS_SELALWAYS & style))
	{
		LVHITTESTINFO ht;
		POINTSTOPOINT(ht.pt, pts);
		INT index = (INT)CallPrevWndProc(LVM_HITTEST, 0, (LPARAM)&ht);
		if (-1 == index)
		{
			if (hwnd != GetFocus() &&
				0 != (LVS_EX_FULLROWSELECT & CallPrevWndProc(LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0L)))
			{
				SetFocus(hwnd);
			}
			return;
		}
	}

	__super::WindowProc(WM_LBUTTONDOWN, (WPARAM)uFlags, *((LPARAM*)&pts));
}

void SkinnedListView::OnRButtonDown(UINT uFlags, POINTS pts)
{
	if (0 != (SWLVS_SELALWAYS & style))
	{
		LVHITTESTINFO ht;
		POINTSTOPOINT(ht.pt, pts);
		INT index = (INT)CallPrevWndProc(LVM_HITTEST, 0, (LPARAM)&ht);
		if (-1 == index)
		{
			return;
		}
	}

	__super::WindowProc(WM_RBUTTONDOWN, (WPARAM)uFlags, *((LPARAM*)&pts));
}

LRESULT SkinnedListView::OnGetDlgCode(UINT vkCode, MSG* pMsg)
{	
	if (NULL != pMsg)
	{
		switch(vkCode)
		{
			case VK_RETURN:
				SendMessage(hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
				return 0;
		}
	}

	LRESULT result = __super::WindowProc(WM_GETDLGCODE, (WPARAM)vkCode, (LPARAM)pMsg);
	if (NULL == pMsg)
	{
		result |= DLGC_WANTMESSAGE;
	}
	return result;
}

LRESULT SkinnedListView::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	switch(uMsg)
	{
		case REFLECTED_NOTIFY: return OnReflectedNotify(((REFLECTPARAM*)lParam)->hwndFrom, (INT)wParam, (NMHDR*)((REFLECTPARAM*)lParam)->lParam, &((REFLECTPARAM*)lParam)->result);
	    // custom handling of this allows us to handle disabled listview controls correctly
	    // so we don't have the part skinned / part OS colouring which happened before
		case WM_ENABLE: InvalidateRect(hwnd, NULL, TRUE);  return 1;
		case WM_SETFONT:
			HWND hwndHeader;
			hwndHeader = ListView_GetHeader(hwnd); 
			if (hwndHeader)
			{
				SendMessageW(hwndHeader, WM_SETFONT, wParam, lParam);
				MLSkinnedHeader_SetHeight(hwndHeader, -1);
			}
			break;
		case WM_LBUTTONDOWN:			OnLButtonDown((UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_RBUTTONDOWN:			OnRButtonDown((UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_KEYDOWN:				OnKeyDown((UINT)wParam, (UINT)lParam); return 0;
		case LVM_INSERTCOLUMNA:
		case LVM_INSERTCOLUMNW:
			{
				LRESULT r = __super::WindowProc(uMsg, wParam, lParam);
				TryAttachHeader();
				return r;
			}
			break;
		case WM_GETDLGCODE:	return OnGetDlgCode((UINT)wParam, (MSG*)lParam);
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}