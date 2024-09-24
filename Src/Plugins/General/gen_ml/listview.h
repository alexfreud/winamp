/*
** Copyright (C) 2003 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
*/

#ifndef _LISTVIEW_H_
#define _LISTVIEW_H_

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

class W_ListView 
{
public:
	W_ListView()
	{
		m_hwnd=NULL; 
		m_col=0; 
		m_allowfonts=1;
		m_font=NULL; 
#ifndef GEN_ML_EXPORTS
		m_libraryparent=NULL;
#endif
	}
	W_ListView(HWND hwnd) 
	{
		m_hwnd=NULL; 
		m_col=0; 
		m_allowfonts=1;
		m_font=NULL; 
#ifndef GEN_ML_EXPORTS
		m_libraryparent=NULL;
#endif
		setwnd(hwnd);
	}
	~W_ListView() 
	{
		if (m_font)	DeleteFont(m_font);
		m_font=0;
	}

	void refreshFont();

#ifndef GEN_ML_EXPORTS
	void setLibraryParentWnd(HWND hwndParent)
	{
		m_libraryparent=hwndParent;
	}// for Winamp Font getting stuff
#endif
	void setallowfonts(int allow=1);
	void setwnd(HWND hwnd);
	void AddCol(char *text, int w);
	int GetCount(void)
	{
		return ListView_GetItemCount(m_hwnd);
	}
	int GetParam(int p);
	void DeleteItem(int n)
	{
		ListView_DeleteItem(m_hwnd,n);
	}
	void Clear(void)
	{
		ListView_DeleteAllItems(m_hwnd);
	}
	int GetSelected(int x)
	{
		return(ListView_GetItemState(m_hwnd, x, LVIS_SELECTED) & LVIS_SELECTED)?1:0;
	}

	int GetSelectedCount()
	{
		return ListView_GetSelectedCount(m_hwnd);
	}

	int GetSelectionMark()
	{
		return ListView_GetSelectionMark(m_hwnd);
	}
	void SetSelected(int x)
	{
		ListView_SetItemState(m_hwnd,x,LVIS_SELECTED,LVIS_SELECTED);
	}
	int InsertItem(int p, char *text, int param);
	void GetItemRect(int i, RECT *r)
	{
		ListView_GetItemRect(m_hwnd, i, r, LVIR_BOUNDS);
	}
	void SetItemText(int p, int si, char *text);
	void SetItemParam(int p, int param);

	void GetText(int p, int si, char *text, int maxlen)
	{
		ListView_GetItemText(m_hwnd, p, si, text, maxlen);
	}
	int FindItemByParam(int param)
	{
		LVFINDINFO fi={LVFI_PARAM,0,param};
		return ListView_FindItem(m_hwnd,-1,&fi);
	}
	int FindItemByPoint(int x, int y)
	{
		int l=GetCount();
		for (int i=0;i<l;i++)
		{
			RECT r;
			GetItemRect(i, &r);
			if (r.left<=x && r.right>=x && r.top<=y && r.bottom>=y)	return i;
		}
		return -1;
	}
	int GetColumnWidth(int col);
	HWND getwnd(void)
	{
		return m_hwnd;
	}

protected:
	HWND m_hwnd;
	HFONT m_font;
	int m_col;
	int m_allowfonts;
#ifndef GEN_ML_EXPORTS
	HWND m_libraryparent;
#endif
};

#endif//_LISTVIEW_H_

