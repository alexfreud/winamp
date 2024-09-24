#include <windows.h>
#include "main.h"

static void SetWindowRect(HWND w, RECT * r)
{
	SetWindowPos(w, 0, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOCOPYBITS);
}

class DlgBase
{
	public:
	BOOL isDialogMessage(MSG * m) 
	{
		return wnd ? IsDialogMessage(wnd, m) : 0;
	}
protected:
	void endDialog(int x)
	{
		EndDialog(wnd, x);
	}

	void _do_size_x(RECT * r, UINT id, UINT wx, UINT min_x)
	{
		RECT r1 = {r->left, r->top, wx - min_x + r->right, r->bottom};
		SetWindowRect(GetDlgItem(wnd, id), &r1);
	}

	void _do_size_xy(RECT * r, UINT id, UINT wx, UINT wy, UINT min_x, UINT min_y)
	{
		RECT r1 = {r->left, r->top, wx - min_x + r->right, wy - min_y + r->bottom};
		SetWindowRect(GetDlgItem(wnd, id), &r1);
	}

	void _do_align_x_size_y(RECT * r, UINT id, UINT wx, UINT wy, UINT min_x, UINT min_y)
	{
		RECT r1 = {wx - min_x + r->left, r->top, wx - min_x + r->right, wy - min_y + r->bottom};
		SetWindowRect(GetDlgItem(wnd, id), &r1);
	}

	void _do_align_x(RECT * r, UINT id, UINT wx, UINT min_x)
	{
		RECT r1 = {wx - min_x + r->left, r->top, wx - min_x + r->right, r->bottom};
		SetWindowRect(GetDlgItem(wnd, id), &r1);
	}

	void _do_align_xy(RECT * r, UINT id, UINT wx, UINT wy, UINT min_x, UINT min_y)
	{
		RECT r1 = {wx - min_x + r->left, wy - min_y + r->top, wx - min_x + r->right, wy - min_y + r->bottom};
		SetWindowRect(GetDlgItem(wnd, id), &r1);
	}

#define do_size_x(id,r) _do_size_x(r,id,sx,min_size_x)
#define do_size_xy(id,r) _do_size_xy(r,id,sx,sy,min_size_x,min_size_y)
#define do_align_x_size_y(id,r) _do_align_x_size_y(r,id,sx,sy,min_size_x,min_size_y)
#define do_align_xy(id,r) _do_align_xy(r,id,sx,sy,min_size_x,min_size_y)
#define do_align_x(id,r) _do_align_x(r,id,sx,min_size_x)

	HWND wnd;
	UINT min_size_x, min_size_y;
	UINT min_size_x_w, min_size_y_w;

	void do_sizing(UINT wp, RECT * r);
	void MakeComboEdit(UINT id, DWORD s);
	void GetChildRect(UINT id, RECT& child);

	virtual BOOL DlgProc(UINT msg, WPARAM wp, LPARAM lp) { return 0;};
	static BOOL CALLBACK TheDialogProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		DlgBase * p;
		if (msg == WM_INITDIALOG)
		{
			p = (DlgBase*)lp;
			SetWindowLong(wnd, DWL_USER, lp);
			p->wnd = wnd;
			RECT r;
			GetClientRect(wnd, &r);
			p->min_size_x = r.right;
			p->min_size_y = r.bottom;
			GetWindowRect(wnd, &r);
			p->min_size_x_w = r.right - r.left;
			p->min_size_y_w = r.bottom - r.top;
		}
		else p = (DlgBase*)GetWindowLong(wnd, DWL_USER);
		BOOL rv = 0;
		if (p)
		{
			rv = p->DlgProc(msg, wp, lp);
			if (msg == WM_DESTROY)
			{
				p->wnd = 0;
				SetWindowLong(wnd, DWL_USER, 0);
			}
		}
		return rv;
	}
	HWND myCreateDialog(UINT id, HWND parent)
	{
		return CreateDialogParamT(hIns, (char*)id, parent, TheDialogProc, (long)this);
	}
	virtual void myProcessMessage(MSG * msg)
	{
		if (!IsDialogMessage(wnd, msg))
		{
			TranslateMessage(msg);
			DispatchMessage(msg);
		}
	}

	int myDialogBox(UINT id, HWND parent)
	{
		return DialogBoxParamT(hIns, (char*)id, parent, TheDialogProc, (long)this);
	}
	DlgBase() {
		wnd = 0;
		min_size_x = min_size_y = min_size_x_w = min_size_y_w = 0;
	}
	virtual ~DlgBase() {}
};
