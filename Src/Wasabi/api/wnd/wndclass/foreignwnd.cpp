#include "precomp.h"
#include "foreignwnd.h"

PtrListQuickSorted<ForeignWndProc, ForeignWndProcComparator> ForeignWnd::foreignwndprocs;

LRESULT CALLBACK foreignWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ForeignWndProc *wp = ForeignWnd::foreignwndprocs.findItem((const wchar_t *)hwnd);
	if (wp)
	{
		if (uMsg == WM_SHOWWINDOW) // more ?
			wp->wnd->wndProc(hwnd, uMsg, wParam, lParam);
		return CallWindowProc(wp->oldWindowProc, hwnd, uMsg, wParam, lParam);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

ForeignWnd::ForeignWnd(OSWINDOWHANDLE wndhandle, OSMODULEHANDLE module, int subclass)
{
	thishwnd = wndhandle;
	setOSModuleHandle(module);

#ifdef EXPERIMENTAL_INDEPENDENT_AOT
	DebugString("ForeignWnd::ForeignWnd(OSWINDOWHANDLE wndhandle): There might be problems with GWL_USERDATA assumptions when EXPERIMENTAL_INDEPENDENT_AOT is on, lone should audit\n");
#endif

	// access protected basewnd member
	hwnd = wndhandle;

	setForeignWnd(1);

	if (subclass)
	{
		oldWindowProc = (WNDPROC) GetWindowLongPtrW(wndhandle, GWLP_WNDPROC);
		wndprocentry = new ForeignWndProc;
		wndprocentry->wnd = this;
		wndprocentry->hwnd = thishwnd;
		wndprocentry->oldWindowProc = oldWindowProc;
		foreignwndprocs.addItem(wndprocentry);
		SetWindowLongPtrW(thishwnd, GWLP_WNDPROC, (LONG_PTR)foreignWindowProc);
	}
	else
	{
		oldWindowProc = NULL;
		wndprocentry = NULL;
	}
}

ForeignWnd::~ForeignWnd()
{
	if (wndprocentry && oldWindowProc)
	{
		foreignwndprocs.removeItem(wndprocentry);
		delete wndprocentry;
		wndprocentry = NULL;
		SetWindowLongPtrW(thishwnd, GWLP_WNDPROC, (LONG_PTR)oldWindowProc);
		oldWindowProc = NULL;
	}
}

