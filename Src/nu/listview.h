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

#ifndef LVS_EX_DOUBLEBUFFER  //this will work XP only
#define LVS_EX_DOUBLEBUFFER		0x00010000
#endif

class W_ListView
{
public:
	W_ListView();
	W_ListView( HWND hwndView );
	W_ListView( HWND hwndDlg, int resourceId );
	~W_ListView();

	void InvertSelection();
	void SetTextColors( COLORREF foregroundColor, COLORREF backgroundColor );
	void SetFont( HFONT newFont );
	void setwnd( HWND hwnd );
	void AddCol( const wchar_t *text, int w );
	void AddCol( const char *text, int w );
	void AddAutoCol( LPTSTR text );
	void AddImageCol( int w );

	void JustifyColumn( int column, int justificationFlag )
	{
		LVCOLUMN col;
		col.mask = LVCF_FMT;
		col.fmt  = justificationFlag;

		ListView_SetColumn( m_hwnd, column, &col );
	}

	void SetColumnWidth( int column, int width )
	{
		ListView_SetColumnWidth( m_hwnd, column, width );
	}

	int GetCount( void )
	{
		return ListView_GetItemCount( m_hwnd );
	}

	int GetParam( int p );

	void DeleteItem( int n )
	{
		ListView_DeleteItem( m_hwnd, n );
	}

	void RefreshItem( int item )
	{
		ListView_RedrawItems( m_hwnd, item, item );
	}

	void RefreshAll()
	{
		ListView_RedrawItems( m_hwnd, 0, GetCount() );
	}

	void Clear( void )
	{
		ListView_DeleteAllItems( m_hwnd );
	}

	int GetSelected( int x )
	{
		return( ListView_GetItemState( m_hwnd, x, LVIS_SELECTED ) & LVIS_SELECTED ) ? 1 : 0;
	}

	int GetSelectedCount()
	{
		return ListView_GetSelectedCount( m_hwnd );
	}

	int GetNextSelected( int start = -1 )
	{
		return ListView_GetNextItem( m_hwnd, start, LVNI_ALL | LVNI_SELECTED );
	}

	int GetSelectionMark()
	{
		return ListView_GetSelectionMark( m_hwnd );
	}

	void SetSelected( int x )
	{
		ListView_SetItemState( m_hwnd, x, LVIS_SELECTED, LVIS_SELECTED );
	}

	void SelectAll()
	{
		ListView_SetItemState( m_hwnd, -1, LVIS_SELECTED, LVIS_SELECTED );
	}

	void UnselectAll()
	{
		ListView_SetItemState( m_hwnd, -1, 0, LVIS_SELECTED );
	}

	void Unselect( int x )
	{
		ListView_SetItemState( m_hwnd, x, 0, LVIS_SELECTED );
	}

	void EditItem( int x )
	{
		SetFocus( m_hwnd );
		ListView_EditLabel( m_hwnd, x );
	}

	int AppendItem( LPCWSTR text, LPARAM param );
	int InsertItem( int p, const wchar_t *text, LPARAM param );
	int InsertItem( int p, const char *text, LPARAM param );

	void GetItemRect( int i, RECT *r )
	{
		ListView_GetItemRect( m_hwnd, i, r, LVIR_BOUNDS );
	}

	void SetItemText( int p, int si, const wchar_t *text );
	void SetItemText( int p, int si, const char *text );
	void SetItemParam( int p, int param );

	void GetText( int p, int si, char *text, int maxlen );
	void GetText( int p, int si, wchar_t *text, int maxlen );

	size_t GetTextLength( int p, int si )
	{
		LVITEM lvItem;
		lvItem.cchTextMax = 0;
		lvItem.pszText    = 0;
		lvItem.iSubItem   = si;
		lvItem.iItem      = p;

		return SendMessage( m_hwnd, LVM_GETITEMTEXT, p, (LPARAM)&lvItem );
	}

	int FindItemByParam( int param )
	{
		LVFINDINFO fi = { LVFI_PARAM,0,param };

		return ListView_FindItem( m_hwnd, -1, &fi );
	}

	int FindItemByPoint( int x, int y );

	void SetVirtualCount( int count, DWORD flags = 0 )
	{
		ListView_SetItemCountEx( m_hwnd, count, flags );
	}

	void SetVirtualCountAsync( int count, DWORD flags = 0 )
	{
		if ( m_hwnd )
			PostMessage( m_hwnd, LVM_SETITEMCOUNT, count, flags );
	}

	int GetColumnWidth( int col );

	void AutoColumnWidth( int col )
	{
		ListView_SetColumnWidth( m_hwnd, col, LVSCW_AUTOSIZE_USEHEADER );
	}

	void AutoSizeColumn( int col )
	{
		ListView_SetColumnWidth( m_hwnd, col, LVSCW_AUTOSIZE );
	}

	HWND getwnd( void )
	{
		return m_hwnd;
	}

	void ScrollTo( int index )
	{
		ListView_EnsureVisible( m_hwnd, index, FALSE );
	}

	void SetDoubleBuffered( bool buffered = true )
	{
		ListView_SetExtendedListViewStyleEx( m_hwnd, LVS_EX_DOUBLEBUFFER, buffered ? LVS_EX_DOUBLEBUFFER : 0 );
	}

	bool ColumnExists( int columnNum )
	{
		LVCOLUMN col;
		col.mask = LVCF_WIDTH;

		return ListView_GetColumn( m_hwnd, columnNum, &col );
	}

	void ForceUnicode()
	{
		SendMessage( m_hwnd, CCM_SETUNICODEFORMAT, TRUE, 0 );
	}

protected:
	HWND  m_hwnd;
	HFONT m_font;
	int   m_col;

};

#endif//_LISTVIEW_H_

