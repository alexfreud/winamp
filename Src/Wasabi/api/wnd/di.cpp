#include "precomp.h"
#include "drag.h"
#include <api/wnd/api_window.h>

DI::DI(ifc_window *rw)
	{
		if (rw == NULL) di = NULL;
		else di = rw->getDragInterface();
	}
	int DI::dragEnter(ifc_window *sourceWnd)
	{
		return di ? di->dragEnter(sourceWnd) : 0;
	}
	int DI::dragOver(int x, int y, ifc_window *sourceWnd)
	{
		return di ? di->dragOver(x, y, sourceWnd) : 0;
	}
	int DI::dragSetSticky(ifc_window *wnd, int left, int right, int up, int down)
	{
		return di ? di->dragSetSticky(wnd, left, right, up, down) : 0;
	}
	int DI::dragLeave(ifc_window *sourceWnd)
	{
		return di ? di->dragLeave(sourceWnd) : 0;
	}
	int DI::dragDrop(ifc_window *sourceWnd, int x, int y)
	{
		return di ? di->dragDrop(sourceWnd, x, y) : 0;
	}

	const wchar_t *DI::dragGetSuggestedDropTitle(void)
	{
		return di ? di->dragGetSuggestedDropTitle() : NULL;
	}
	int DI::dragCheckData(const wchar_t *type, int *nitems )
	{
		return di ? di->dragCheckData(type, nitems) : 0;
	}
	void *DI::dragGetData(int slot, int itemnum)
	{
		return di ? di->dragGetData(slot, itemnum) : NULL;
	}
	int DI::dragCheckOption(int option)
	{
		return di ? di->dragCheckOption(option) : NULL;
	}
