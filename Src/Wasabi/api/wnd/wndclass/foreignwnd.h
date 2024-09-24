#ifndef __FOREIGNWND_H
#define __FOREIGNWND_H

#ifdef _WIN32
#include <api/wnd/basewnd.h>

class ForeignWnd;

class ForeignWndProc
{
public:
	ForeignWnd *wnd;
	HWND hwnd;
	WNDPROC oldWindowProc;
};

class ForeignWndProcComparator
{
public:
	// comparator for sorting
	static int compareItem(ForeignWndProc *p1, ForeignWndProc* p2)
	{
		return CMP3(p1->hwnd, p2->hwnd);
	}
	// comparator for searching
	static int compareAttrib(const wchar_t *attrib, ForeignWndProc *item)
	{
		return CMP3((HWND)attrib, item->hwnd);
	}
};

class ForeignWnd : public BaseWnd
{
public:
	// takes over an existing OSWINDOWHANDLE and wraps a BaseWnd around it
	// but does not changes the windowproc, nor does it inserts the wnd
	// into the system list. It does not either (under windows anyway)
	// sets the userdata windowlong to the object pointer.
	// if subclass is true, we subclas the windowproc and process events
	// as if the window was a real rootwnd

	ForeignWnd(OSWINDOWHANDLE w, OSMODULEHANDLE m, int subclass = 0);

	virtual ~ForeignWnd();

	static PtrListQuickSorted<ForeignWndProc, ForeignWndProcComparator> foreignwndprocs;

private:
	WNDPROC oldWindowProc;
	HWND thishwnd;
	ForeignWndProc *wndprocentry;
};
#else
#warning port me
#endif
#endif
