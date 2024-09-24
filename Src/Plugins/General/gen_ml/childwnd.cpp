#include "main.h"
#include "childwnd.h"
#include "resource.h"

typedef struct _CHILDREMOVERGN
{
	HWND hwndParent;
	HRGN rgnUpdate;
	HRGN rgnChild;
} CHILDREMOVERGN;

static BOOL useDeferWndPos = TRUE;


void childresize_init(HWND hwndDlg, ChildWndResizeItem *list, int num)
{
	RECT r;
	int x;

	useDeferWndPos = (GetVersion() < 0x80000000);

	GetClientRect(hwndDlg, &r);
	
	for (x = 0; x < num; x ++)
	{
		RECT r2;
		GetWindowRect(GetDlgItem(hwndDlg,list[x].id), &r2);
		MapWindowPoints(HWND_DESKTOP, hwndDlg, (LPPOINT)&r2, 2);
		
		list[x].rinfo.left	= (0xF000 & list[x].type) ? (r.right - r2.left) : r2.left;
		list[x].rinfo.top	= (0x0F00 & list[x].type) ? (r.bottom -r2.top) : r2.top;
		list[x].rinfo.right	= (0x00F0 & list[x].type) ? (r.right - r2.right) : r2.right;
		list[x].rinfo.bottom= (0x000F & list[x].type) ? (r.bottom - r2.bottom) : r2.bottom;
		
		list[x].type |= 0xF0000;
	} 

}

BOOL CALLBACK childresize_enumRemoveRegion(HWND hwnd, LPARAM lParam)
{
	if (IsWindowVisible(hwnd) && GetParent(hwnd) == ((CHILDREMOVERGN*)lParam)->hwndParent) 
	{
		RECT r;
		GetWindowRect(hwnd, &r);
		MapWindowPoints(HWND_DESKTOP, ((CHILDREMOVERGN*)lParam)->hwndParent, (LPPOINT)&r, 2);
		
		SetRectRgn(((CHILDREMOVERGN*)lParam)->rgnChild, r.left, r.top, r.right, r.bottom);
		CombineRgn(((CHILDREMOVERGN*)lParam)->rgnUpdate, ((CHILDREMOVERGN*)lParam)->rgnUpdate, ((CHILDREMOVERGN*)lParam)->rgnChild, RGN_DIFF);
	}
	
	return TRUE;
}

void childresize_resize_to_rectlist(HWND hwndDlg, ChildWndResizeItem *list, int num, RECT *rectout)
{
	RECT r;
	int x;
	GetClientRect(hwndDlg,&r);
	
	for (x = 0; x < num; x ++) if (list[x].type&0xf0000)
		{
			RECT r2;
			if (list[x].type&0xF000)  r2.left=r.right-list[x].rinfo.left;
			else r2.left=list[x].rinfo.left;

			if (list[x].type&0x0F00)  r2.top=r.bottom-list[x].rinfo.top;
			else r2.top=list[x].rinfo.top;

			if (list[x].type&0x00F0)  r2.right=r.right-list[x].rinfo.right;
			else r2.right=list[x].rinfo.right;

			if (list[x].type&0x000F)  r2.bottom=r.bottom-list[x].rinfo.bottom;
			else r2.bottom=list[x].rinfo.bottom;

			*rectout = r2;
			rectout++;
		}
}

void childresize_resize_from_rectlist(HWND hwndDlg, ChildWndResizeItem *list, int num, RECT *rectin)
{
	RECT r, *pr;
	CHILDREMOVERGN crr;
	int x;
	HDWP hdwp;
	HWND h;

	GetClientRect(hwndDlg,&r);

	crr.hwndParent = hwndDlg;
	crr.rgnUpdate = CreateRectRgnIndirect(&r);
	crr.rgnChild = CreateRectRgn(0,0,0,0);

	EnumChildWindows(hwndDlg,&childresize_enumRemoveRegion,(LPARAM)&crr);

	
	hdwp = (useDeferWndPos) ? BeginDeferWindowPos(num) : NULL;
	for (pr = rectin, x = 0; x < num && hdwp; x ++) 
	{
		if (0xF0000 & list[x].type)
		{
			h = GetDlgItem(hwndDlg,list[x].id);
			if (h && pr) hdwp = DeferWindowPos(hdwp, h, NULL, pr->left, pr->top, pr->right - pr->left, pr->bottom - pr->top, SWP_NOZORDER | SWP_NOACTIVATE);
			pr++;
		}
	}
	if (hdwp) EndDeferWindowPos(hdwp);
	else
	{
		for (pr = rectin, x = 0; x < num; x ++) 
		{
			if (0xF0000 & list[x].type)
			{
				h = GetDlgItem(hwndDlg,list[x].id);
				if (h && pr) SetWindowPos(h, NULL, pr->left, pr->top, pr->right - pr->left, pr->bottom - pr->top, SWP_NOZORDER | SWP_NOACTIVATE);
				pr++;
			}
		}
	}
	EnumChildWindows(hwndDlg,&childresize_enumRemoveRegion,(LPARAM)&crr);
	InvalidateRgn(hwndDlg, crr.rgnUpdate, TRUE);
	DeleteObject(crr.rgnUpdate);
	DeleteObject(crr.rgnChild);
}

void childresize_resize(HWND hwndDlg, ChildWndResizeItem *list, int num)
{
	RECT rc, rw;
	CHILDREMOVERGN crr;
	int x, y, cx, cy;
	DWORD flags;
	HDWP hdwp;
	ChildWndResizeItem *pi;

	GetClientRect(hwndDlg,&rc);
	crr.hwndParent = hwndDlg;
	crr.rgnUpdate = CreateRectRgnIndirect(&rc);
	crr.rgnChild = CreateRectRgn(0,0,0,0);

	EnumChildWindows(hwndDlg,&childresize_enumRemoveRegion,(LPARAM)&crr);

	hdwp = (useDeferWndPos) ? BeginDeferWindowPos(num) : NULL;

	for (pi = list + num - 1; pi >= list && (!useDeferWndPos || hdwp); pi--) 
	{
		HWND hwnd = GetDlgItem(hwndDlg, pi->id);
		if (hwnd && (0xF0000 & pi->type))
		{
			x = (0xF000 & pi->type) ? (rc.right - pi->rinfo.left) : pi->rinfo.left;
			y = (0x0F00 & pi->type) ? (rc.bottom - pi->rinfo.top) : pi->rinfo.top;
			cx = ((0x00F0 & pi->type) ? (rc.right - pi->rinfo.right) : pi->rinfo.right) - x;
			cy = ((0x000F & pi->type) ? (rc.bottom - pi->rinfo.bottom) : pi->rinfo.bottom) - y;
			flags = SWP_NOZORDER | SWP_NOACTIVATE;

			GetWindowRect(hwnd, &rw);
			MapWindowPoints(HWND_DESKTOP, hwndDlg, (LPPOINT)&rw, 2);
			if (rw.left == x && rw.top == y) flags |= SWP_NOMOVE;
			if (rw.right == (x + cx) && rw.bottom == (y + cy)) flags |= SWP_NOSIZE;

			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & flags))
			{
				if (useDeferWndPos) hdwp = DeferWindowPos(hdwp, hwnd, NULL, x, y, cx, cy, flags);
				else SetWindowPos( hwnd, NULL, x, y, cx, cy, flags);
			}
		}
	}
	if (hdwp) EndDeferWindowPos(hdwp);

	EnumChildWindows(hwndDlg,&childresize_enumRemoveRegion,(LPARAM)&crr);
	InvalidateRgn(hwndDlg, crr.rgnUpdate, TRUE);

	DeleteObject(crr.rgnUpdate);
	DeleteObject(crr.rgnChild);
}

void childresize_resize2(HWND hwndDlg, ChildWndResizeItem *list, int num, BOOL fRedraw, HRGN rgnUpdate)
{
	RECT rc, rw;
	CHILDREMOVERGN crr;
	int x, y, cx, cy;
	DWORD flags;
	HDWP hdwp;
	ChildWndResizeItem *pi;

	GetClientRect(hwndDlg,&rc);
	crr.hwndParent = hwndDlg;
	crr.rgnUpdate = CreateRectRgnIndirect(&rc);
	crr.rgnChild = CreateRectRgn(0,0,0,0);

//	EnumChildWindows(hwndDlg,&childresize_enumRemoveRegion,(LPARAM)&crr);

	hdwp = (useDeferWndPos) ? BeginDeferWindowPos(num) : NULL;

	for (pi = list + num - 1; pi >= list && (!useDeferWndPos || hdwp); pi--) 
	{
		HWND hwnd = GetDlgItem(hwndDlg, pi->id);
		if (hwnd && (0xF0000 & pi->type))
		{
			x = (0xF000 & pi->type) ? (rc.right - pi->rinfo.left) : pi->rinfo.left;
			y = (0x0F00 & pi->type) ? (rc.bottom - pi->rinfo.top) : pi->rinfo.top;
			cx = ((0x00F0 & pi->type) ? (rc.right - pi->rinfo.right) : pi->rinfo.right) - x;
			cy = ((0x000F & pi->type) ? (rc.bottom - pi->rinfo.bottom) : pi->rinfo.bottom) - y;
			flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW;

			GetWindowRect(hwnd, &rw);
			MapWindowPoints(HWND_DESKTOP, hwndDlg, (LPPOINT)&rw, 2);
			if (rw.left == x && rw.top == y) flags |= SWP_NOMOVE;
			if (rw.right == (x + cx) && rw.bottom == (y + cy)) flags |= SWP_NOSIZE;

			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & flags))
			{
				if (useDeferWndPos) hdwp = DeferWindowPos(hdwp, hwnd, NULL, x, y, cx, cy, flags);
				else SetWindowPos( hwnd, NULL, x, y, cx, cy, flags);
			}
			else
			{
				SetRectRgn(crr.rgnChild, rw.left, rw.top, rw.right, rw.bottom);
				CombineRgn(crr.rgnUpdate, crr.rgnUpdate, crr.rgnChild, RGN_DIFF);
			}
		}
	}
	if (hdwp) EndDeferWindowPos(hdwp);

//	EnumChildWindows(hwndDlg,&childresize_enumRemoveRegion,(LPARAM)&crr);

	if (fRedraw)
	{
		InvalidateRgn(hwndDlg, crr.rgnUpdate, TRUE);
		RedrawWindow(hwndDlg, NULL, crr.rgnUpdate, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ERASE | RDW_ALLCHILDREN);
		//RedrawWindow(hwndDlg, NULL, crr.rgnUpdate, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_FRAME | RDW_ALLCHILDREN);
	}
	DeleteObject(crr.rgnUpdate);
	DeleteObject(crr.rgnChild);
}