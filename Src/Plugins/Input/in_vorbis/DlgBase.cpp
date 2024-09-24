#include "DlgBase.h"

void DlgBase::MakeComboEdit(UINT id, DWORD s)
{
	HWND w = GetDlgItem(wnd, id);
	RECT r;
	GetChildRect(id, r);
	DestroyWindow(w);
	CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", 0, WS_CHILD | s, r.left - 1, r.top - 1, r.right - r.left, r.bottom - r.top, wnd, (HMENU)id, 0, 0);
}

void DlgBase::GetChildRect(UINT id, RECT& child)
{
	RECT r_parent, r_child;
	GetWindowRect(wnd, &r_parent);
	GetWindowRect(GetDlgItem(wnd, id), &r_child);
	int dx = r_parent.left;
	int dy = r_parent.top;
	if (!(GetWindowLong(wnd, GWL_STYLE)&WS_CHILD))
	{
		dy += GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME);
		dx += GetSystemMetrics(SM_CXDLGFRAME);
	}
	child.left = r_child.left - dx;
	child.right = r_child.right - dx;
	child.top = r_child.top - dy;
	child.bottom = r_child.bottom - dy;
}

void DlgBase::do_sizing(UINT wp, RECT * r)
	{
		UINT dx, dy;
		dx = r->right - r->left;
		dy = r->bottom - r->top;
		if (dx < min_size_x_w)
		{
			switch (wp)
			{
			case WMSZ_BOTTOMLEFT:
			case WMSZ_LEFT:
			case WMSZ_TOPLEFT:
				r->left = r->right - min_size_x_w;
				break;
			case WMSZ_BOTTOMRIGHT:
			case WMSZ_RIGHT:
			case WMSZ_TOPRIGHT:
				r->right = r->left + min_size_x_w;
				break;
			}
		}
		if (dy < min_size_y_w)
		{
			switch (wp)
			{
			case WMSZ_BOTTOMLEFT:
			case WMSZ_BOTTOM:
			case WMSZ_BOTTOMRIGHT:
				r->bottom = r->top + min_size_y_w;
				break;
			case WMSZ_TOPLEFT:
			case WMSZ_TOP:
			case WMSZ_TOPRIGHT:
				r->top = r->bottom - min_size_y_w;
				break;
			}
		}
	}