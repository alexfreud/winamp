#ifndef __WASABI_DRAG_H
#define __WASABI_DRAG_H

class ifc_window;

#include <bfc/wasabi_std.h>

class NOVTABLE DragInterface
{
public:
	// (called on dest) when dragged item enters the winder
	virtual int dragEnter(ifc_window *sourceWnd) = 0;
	// (called on dest) during the winder
	virtual int dragOver(int x, int y, ifc_window *sourceWnd) = 0;
	// (called on src)
	virtual int dragSetSticky(ifc_window *wnd, int left, int right, int up, int down) = 0;
	// (called on dest) when dragged item leaves the winder
	virtual int dragLeave(ifc_window *sourceWnd) = 0;
	// (called on dest) here is where we actually drop it
	virtual int dragDrop(ifc_window *sourceWnd, int x, int y) = 0;

	// must be called from within dragDrop();			(receiver)
	virtual const wchar_t *dragGetSuggestedDropTitle(void) = 0;
	// must be called from within your dragEnter, Over, Leave, or Drop
	// return the slot # if you support this form of the data, -1 otherwise
	// nitems can be NULL if you're just checking validity
	virtual int dragCheckData(const wchar_t *type, int *nitems = NULL) = 0;
	// fetches a specific pointer that was stored
	virtual void *dragGetData(int slot, int itemnum) = 0;
	virtual int dragCheckOption(int option) = 0;
};

class DragInterfaceI : public DragInterface
{
public:
	// (called on dest) when dragged item enters the winder
	virtual int dragEnter(ifc_window *sourceWnd) { return 0; }
	// (called on dest) during the winder
	virtual int dragOver(int x, int y, ifc_window *sourceWnd) { return 0; }
	// (called on src)
	virtual int dragSetSticky(ifc_window *wnd, int left, int right, int up, int down) { return 0; }
	// (called on dest) when dragged item leaves the winder
	virtual int dragLeave(ifc_window *sourceWnd) { return 0; }
	// (called on dest) here is where we actually drop it
	virtual int dragDrop(ifc_window *sourceWnd, int x, int y) { return 0; }

	// must be called from within dragDrop();			(receiver)
	virtual const wchar_t *dragGetSuggestedDropTitle(void) { return NULL; }
	// must be called from within your dragEnter, Over, Leave, or Drop
	// return the slot # if you support this form of the data, -1 otherwise
	// nitems can be NULL if you're just checking validity
	virtual int dragCheckData(const wchar_t *type, int *nitems = NULL) { return 0; }
	// fetches a specific pointer that was stored
	virtual void *dragGetData(int slot, int itemnum) { return 0; }
	virtual int dragCheckOption(int option) { return 0; }
};

class DI
{
public:
	DI(ifc_window *rw);
	int dragEnter(ifc_window *sourceWnd);
	int dragOver(int x, int y, ifc_window *sourceWnd);
	int dragSetSticky(ifc_window *wnd, int left, int right, int up, int down);
	int dragLeave(ifc_window *sourceWnd);
	int dragDrop(ifc_window *sourceWnd, int x, int y);
	const wchar_t *dragGetSuggestedDropTitle(void);
	int dragCheckData(const wchar_t *type, int *nitems = NULL);
	void *dragGetData(int slot, int itemnum);
	int dragCheckOption(int option);
private:
	DragInterface *di;
};

#endif
