/*
** Copyright  (C) 2003 Nullsoft, Inc.
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

#include <windows.h>
#include <commctrl.h>

#include "listview.h"

void W_ListView::AddImageCol( int w )
{
	LVCOLUMN lvc = { 0, };
	lvc.mask     = LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
	lvc.fmt      = LVCFMT_IMAGE;
	lvc.iSubItem = m_col;

	if ( w )
		lvc.cx = w;

	ListView_InsertColumn( m_hwnd, m_col, &lvc );
	++m_col;
}

int W_ListView::GetColumnWidth( int col )
{
	if ( col < 0 || col >= m_col )
		return 0;

	return ListView_GetColumnWidth( m_hwnd, col );
}

int W_ListView::GetParam( int p )
{
	LVITEM lvi = { 0, };
	lvi.mask  = LVIF_PARAM;
	lvi.iItem = p;

	ListView_GetItem( m_hwnd, &lvi );

	return (int)(INT_PTR)lvi.lParam;
}

void W_ListView::SetItemParam( int p, int param )
{
	LVITEM lvi = { 0, };
	lvi.iItem  = p;
	lvi.mask   = LVIF_PARAM;
	lvi.lParam = param;

	ListView_SetItem( m_hwnd, &lvi );
}

void W_ListView::SetFont( HFONT newFont )
{
	if ( m_font )
	{
		if ( m_hwnd )
			SetWindowFont( m_hwnd, NULL, FALSE );

		DeleteFont( m_font );
	}

	m_font = NULL;
	if ( m_hwnd )
	{
		SetWindowFont( m_hwnd, newFont, FALSE );
		InvalidateRect( m_hwnd, NULL, TRUE );
	}
}

int W_ListView::FindItemByPoint( int x, int y )
{
	int l = GetCount();
	for ( int i = 0; i < l; i++ )
	{
		RECT r;
		GetItemRect( i, &r );
		if ( r.left <= x && r.right >= x && r.top <= y && r.bottom >= y )
			return i;
	}
	return -1;
}

W_ListView::W_ListView()
{
	m_hwnd = NULL;
	m_col  = 0;
	m_font = NULL;
}

W_ListView::W_ListView( HWND hwnd )
{
	m_hwnd = NULL;
	m_col  = 0;
	m_font = NULL;

	if ( IsWindow( hwnd ) )
	{
		m_hwnd = hwnd;
		ListView_SetExtendedListViewStyleEx( m_hwnd, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP );
	}
}

W_ListView::W_ListView( HWND hwndDlg, int resourceId )
{
	m_hwnd = NULL;
	m_col  = 0;
	m_font = NULL;

	m_hwnd = GetDlgItem( hwndDlg, resourceId );
	if ( IsWindow( m_hwnd ) )
	{
		ListView_SetExtendedListViewStyleEx( m_hwnd, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP );
	}
}

W_ListView::~W_ListView()
{
	if ( m_font )
		DeleteFont( m_font );

	m_font = 0;
}

void W_ListView::SetTextColors(COLORREF foregroundColor, COLORREF backgroundColor)
{
	ListView_SetTextColor(m_hwnd, foregroundColor);
	ListView_SetTextBkColor(m_hwnd, backgroundColor);
}

void W_ListView::InvertSelection()
{
	int n = GetCount();
	for (int i = 0; i < n; i++)
	{
		if (GetSelected(i))
			Unselect(i);
		else
			SetSelected(i);
	}
}

/* unicode / ansi trouble spots go below this line */
void W_ListView::AddAutoCol(LPTSTR text)
{
	LVCOLUMN lvc = {0};
	lvc.mask     = LVCF_TEXT;
	lvc.pszText  = text;

	ListView_InsertColumn(m_hwnd, m_col, &lvc);
	m_col++;
}

void W_ListView::AddCol(const wchar_t *text, int w)
{
	LVCOLUMN lvc = {0};
	lvc.mask     = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText  = (LPTSTR)text;

	if (w) 
		lvc.cx = w;

	SendMessageW(m_hwnd, LVM_INSERTCOLUMNW, (WPARAM)m_col, (LPARAM)&lvc);
	m_col++;
}

void W_ListView::AddCol(const char *text, int w)
{
	LVCOLUMN lvc = {0};
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	lvc.pszText = (LPTSTR) text;
	if (w) lvc.cx = w;
	SendMessageA(m_hwnd, LVM_INSERTCOLUMNA, (WPARAM)m_col, (LPARAM)&lvc);
	m_col++;
}

int W_ListView::AppendItem(LPCWSTR text, LPARAM param)
{
	LVITEM lvi = {0};
	lvi.mask       = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem      = GetCount();
	lvi.pszText    = (LPTSTR) text;
	lvi.cchTextMax = wcslen(text);
	lvi.lParam     = param;

	return (int)(INT_PTR)SendMessageW(m_hwnd, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
}

int W_ListView::InsertItem(int p, const wchar_t *text, LPARAM param)
{
	LVITEM lvi = {0};
	lvi.mask       = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem      = p;
	lvi.pszText    = (LPTSTR) text;
	lvi.cchTextMax = wcslen(text);
	lvi.lParam     = param;

	return (int)(INT_PTR)SendMessageW(m_hwnd, LVM_INSERTITEMW, 0, (LPARAM)&lvi);
}

int W_ListView::InsertItem(int p, const char *text, LPARAM param)
{
	LVITEM lvi = {0};
	lvi.mask       = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem      = p;
	lvi.pszText    = (LPTSTR) text;
	lvi.cchTextMax = lstrlenA(text);
	lvi.lParam     = param;

	return (int)(INT_PTR)SendMessageA(m_hwnd, LVM_INSERTITEMA, 0, (LPARAM)&lvi);
}

void W_ListView::SetItemText(int p, int si, const wchar_t *text)
{
	LVITEM lvi = {0};
	lvi.iItem      = p;
	lvi.iSubItem   = si;
	lvi.mask       = LVIF_TEXT;
	lvi.pszText    = (LPTSTR)text;
	lvi.cchTextMax = wcslen(text);

	SendMessageW(m_hwnd, LVM_SETITEMW, 0, (LPARAM)&lvi);
}

void W_ListView::SetItemText(int p, int si, const char *text)
{
	LVITEM lvi = {0};
	lvi.iItem      = p;
	lvi.iSubItem   = si;
	lvi.mask       = LVIF_TEXT;
	lvi.pszText    = (LPTSTR)text;
	lvi.cchTextMax = lstrlenA(text);

	SendMessageA(m_hwnd, LVM_SETITEMA, 0, (LPARAM)&lvi);
}

void W_ListView::setwnd (HWND hwnd)
{
	m_hwnd = hwnd;
	if (IsWindow(hwnd))
	{
		ListView_SetExtendedListViewStyleEx(hwnd, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
#if defined(_UNICODE) || defined(UNICODE)
		SendMessageW(hwnd, CCM_SETUNICODEFORMAT, TRUE, 0);
#endif
	}
}

void W_ListView::GetText(int p, int si, wchar_t *text, int maxlen)
{
	LVITEM lvi = {0};
	lvi.iItem      = p;
	lvi.iSubItem   = si;
	lvi.mask       = LVIF_TEXT;
	lvi.pszText    = (LPTSTR)text;
	lvi.cchTextMax = maxlen;

	SendMessageW(m_hwnd, LVM_GETITEMTEXTW, p,  (LPARAM)&lvi);
}

void W_ListView::GetText(int p, int si, char *text, int maxlen)
{
	LVITEM lvi = {0};
	lvi.iItem      = p;
	lvi.iSubItem   = si;
	lvi.mask       = LVIF_TEXT;
	lvi.pszText    = (LPTSTR)text;
	lvi.cchTextMax = maxlen;

	SendMessageA(m_hwnd, LVM_GETITEMTEXTA, p,  (LPARAM)&lvi);
}