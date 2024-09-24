#ifndef _LISTSKIN_H
#define _LISTSKIN_H

#include <windows.h>

class ScrollWnd;

class ListSkin
{
public:
	ListSkin(HWND hwnd);
	~ListSkin();

	void updateScrollWnd();
	void disableHorzScroll();
	
  
	HWND m_hwnd;
	HWND m_listwnd;
	HWND m_headerwnd;
	ScrollWnd *m_scrollwnd;
	WNDPROC m_old_wndproc;
	WNDPROC m_old_header_wndproc;
	WNDPROC m_old_mainwndproc;

	// sort 
	BOOL sortShow;
	BOOL sortAscending;
	int	sortIndex;

	// enabled/disabled handling
	int m_enabled;
//  int m_changing_item_sel;
};

#endif