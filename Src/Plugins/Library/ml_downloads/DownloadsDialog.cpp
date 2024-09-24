#include "main.h"
#include "api__ml_downloads.h"
#include "RFCDate.h"
#include "Downloaded.h"
#include "DownloadStatus.h"
#include "Defaults.h"
#include "../nu/listview.h"
#include "..\..\General\gen_ml/ml_ipc.h"
#include "..\..\General\gen_ml/menu.h"
#include <vector>
#include "../nu/menushortcuts.h"
#include <commctrl.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <strsafe.h>
#include <algorithm>

HWND downloads_window = 0;
extern int downloads_treeItem;
extern int no_auto_hide;
int groupBtn = 1, enqueuedef = 0, customAllowed = 0;
HMENU g_context_menus2 = NULL;
static viewButtons view;

#ifndef HDF_SORTUP
#define HDF_SORTUP              0x0400
#define HDF_SORTDOWN            0x0200
#endif // !HDF_SORTUP

using namespace Nullsoft::Utility;

enum
{
    COL_TITLE = 0,
	COL_PROGRESS,
	COL_DATE,
    COL_SOURCE,
	COL_SIZE,
	COL_PATH,
    NUM_COLUMNS,
};

int downloadsSourceWidth   = DOWNLOADSSOURCEWIDTHDEFAULT;
int downloadsTitleWidth    = DOWNLOADSTITLEWIDTHDEFAULT;
int downloadsProgressWidth = DOWNLOADSPROGRESSWIDTHDEFAULT;
int downloadsDateWidth     = DOWNLOADSDATEWIDTHDEFAULTS;
int downloadsSizeWidth     = DOWNLOADSSIZEWIDTHDEFAULTS;
int downloadsPathWidth     = DOWNLOADSPATHWIDTHDEFAULTS;


W_ListView downloadList;
int downloadsItemSort = 2; // -1 means no sort active
bool downloadsSortAscending = false;

enum
{
    DOWNLOADSDIALOG_TIMER_UPDATESTATUSBAR = 0,
};

class DownloadListItem
{
public:
	DownloadedFile *f = NULL;
	DownloadToken token = NULL;
	wchar_t *source = 0;
	wchar_t *title  = 0;
	wchar_t *path   = 0;
	wchar_t status[ 20 ] = { 0 };

	DownloadListItem( DownloadedFile *fi )
	{
		f = new DownloadedFile( *fi );
		ZeroMemory( status, sizeof( status ) );
	}

	DownloadListItem( DownloadToken p_token, const wchar_t *p_source, const wchar_t *p_title, const wchar_t *p_path, size_t p_downloaded, size_t p_maxSize ) : token( p_token )
	{
		if ( p_maxSize )
			StringCchPrintf( status, 20, WASABI_API_LNGSTRINGW( IDS_DOWNLOADING_PERCENT ), (int)( p_downloaded / ( p_maxSize / 100 ) ) );
		else
		{
			if ( WAC_API_DOWNLOADMANAGER->IsPending( p_token ) )
				WASABI_API_LNGSTRINGW_BUF( IDS_DOWNLOAD_PENDING, status, 20 );
			else
				WASABI_API_LNGSTRINGW_BUF( IDS_DOWNLOADING, status, 20 );
		}

		source = p_source ? _wcsdup( p_source ) : NULL;
		title  = p_title ? _wcsdup( p_title ) : NULL;
		path   = p_path ? _wcsdup( p_path ) : NULL;
	}

	~DownloadListItem()
	{
		clean();

		if ( f )
			delete f;
	}

	void clean()
	{
		if ( source )
		{
			free( source );
			source = 0;
		}

		if ( title )
		{
			free( title );
			title = 0;
		}

		if ( path )
		{
			free( path );
			path = 0;
		}
	}
};

static std::vector<DownloadListItem*> listContents;

bool GetDownload( int &download )
{
	download = ListView_GetNextItem( downloadList.getwnd(), download, LVNI_ALL | LVNI_SELECTED );
	if ( download == -1 )
		return false;
	else
		return true;
}

void Downloads_Play( bool enqueue = false )
{
	int download = -1;
	AutoLock lock( downloadedFiles );
	while ( GetDownload( download ) )
	{
		if ( !enqueue )
		{
			if ( listContents[ download ]->f )
				mediaLibrary.PlayFile( listContents[ download ]->f->path );
			else if ( listContents[ download ]->path )
				mediaLibrary.PlayFile( listContents[ download ]->path );

			enqueue = true;
		}
		else
		{
			if ( listContents[ download ]->f )
				mediaLibrary.EnqueueFile( listContents[ download ]->f->path );
			else if ( listContents[ download ]->path )
				mediaLibrary.EnqueueFile( listContents[ download ]->path );
		}
	}
}

void DownloadsUpdated( const DownloadStatus::Status &s, DownloadToken token )
{
	listContents.push_back( new DownloadListItem( token, s.source, s.title, s.path, s.downloaded, s.maxSize ) );

	downloadList.SetVirtualCountAsync( (int)listContents.size() );
}

void DownloadsUpdated( DownloadToken token, const DownloadedFile *f )
{
	for ( DownloadListItem *l_content : listContents )
	{
		if ( l_content->token == token )
		{
			l_content->token = 0;
			if ( f )
			{
				l_content->f = new DownloadedFile( *f );

				l_content->clean();
			}
			else
				lstrcpyn( l_content->status, L"Error", 20 );

			break;
		}
	}

	PostMessage( downloadList.getwnd(), LVM_REDRAWITEMS, 0, listContents.size() );
}

void DownloadsUpdated()
{
	for ( DownloadListItem *l_content : listContents )
		delete l_content;

	listContents.clear();

	for ( DownloadedFile &l_download : downloadedFiles.downloadList )
		listContents.push_back( new DownloadListItem( &l_download ) );

	{
		AutoLock lock( downloadStatus.statusLock );
		for ( DownloadStatus::Downloads::iterator itr = downloadStatus.downloads.begin(); itr != downloadStatus.downloads.end(); itr++ )
		{
			listContents.push_back( new DownloadListItem( itr->first, itr->second.source, itr->second.title, itr->second.path, itr->second.downloaded, itr->second.maxSize ) );
		}
	}

	downloadList.SetVirtualCountAsync( (int)listContents.size() );
//	Navigation_ShowService( SERVICE_DOWNLOADS, SHOWMODE_AUTO );
}

static void CleanupDownloads()
{
	{
		AutoLock lock( downloadedFiles );
		DownloadList::DownloadedFileList &downloads = downloadedFiles.downloadList;
		DownloadList::iterator itr, next;
		for ( itr = downloads.begin(); itr != downloads.end();)
		{
			next = itr;
			++next;
			if ( !PathFileExists( itr->path ) )
				downloads.erase( itr );
			else
				itr = next;
		}
	}

//	Navigation_ShowService(SERVICE_DOWNLOADS, SHOWMODE_AUTO);
}

void Downloads_UpdateStatusBar(HWND hwndDlg)
{
	wchar_t status[256]=L"";
	downloadStatus.GetStatusString(status, 256);
	SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), status);
}

void Downloads_Paint(HWND hwndDlg)
{
	int tab[] = { IDC_DOWNLOADLIST | DCW_SUNKENBORDER, };
	dialogSkinner.Draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
}


static HRGN g_rgnUpdate = NULL;
static int offsetX = 0, offsetY = 0;

typedef struct _LAYOUT
{
	INT		id;
	HWND	hwnd;
	INT		x;
	INT		y;
	INT		cx;
	INT		cy;
	DWORD	flags;
	HRGN	rgn;
}
LAYOUT, PLAYOUT;

#define SETLAYOUTPOS(_layout, _x, _y, _cx, _cy) { _layout->x=_x; _layout->y=_y;_layout->cx=_cx;_layout->cy=_cy;_layout->rgn=NULL; }
#define SETLAYOUTFLAGS(_layout, _r)																						\
	{																													\
		BOOL fVis;																										\
		fVis = (WS_VISIBLE & (LONG)GetWindowLongPtr(_layout->hwnd, GWL_STYLE));											\
		if (_layout->x == _r.left && _layout->y == _r.top) _layout->flags |= SWP_NOMOVE;									\
		if (_layout->cx == (_r.right - _r.left) && _layout->cy == (_r.bottom - _r.top)) _layout->flags |= SWP_NOSIZE;	\
		if ((SWP_HIDEWINDOW & _layout->flags) && !fVis) _layout->flags &= ~SWP_HIDEWINDOW;								\
		if ((SWP_SHOWWINDOW & _layout->flags) && fVis) _layout->flags &= ~SWP_SHOWWINDOW;									\
	}

#define LAYOUTNEEEDUPDATE(_layout) ((SWP_NOMOVE | SWP_NOSIZE) != ((SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_SHOWWINDOW) & _layout->flags))

#define GROUP_MIN			0x1
#define GROUP_MAX			0x2
#define GROUP_STATUSBAR		0x1
#define GROUP_MAIN			0x2

static void LayoutWindows(HWND hwnd, BOOL fRedraw, BOOL fUpdateAll = FALSE)
{
	static INT controls[] =
	{
		GROUP_STATUSBAR, IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM, IDC_REMOVE, IDC_CLEANUP, IDC_STATUS,
		GROUP_MAIN, IDC_DOWNLOADLIST
	};

	INT index;
	RECT rc, rg, ri;
	LAYOUT layout[sizeof(controls)/sizeof(controls[0])], *pl;
	BOOL skipgroup;
	HRGN rgn = NULL;

	GetClientRect(hwnd, &rc);

	if (rc.right == rc.left || rc.bottom == rc.top)
		return;

	if ( rc.right > WASABI_API_APP->getScaleX( 4 ) )
		rc.right -= WASABI_API_APP->getScaleX( 4 );

	SetRect( &rg, rc.left, rc.top, rc.right, rc.top );

	pl = layout;
	skipgroup = FALSE;

	InvalidateRect(hwnd, NULL, TRUE);

	for (index = 0; index < sizeof(controls) / sizeof(*controls); index++)
	{
		if (controls[index] >= GROUP_MIN && controls[index] <= GROUP_MAX) // group id
		{
			skipgroup = FALSE;
			switch (controls[index])
			{
				case GROUP_STATUSBAR:
				{
					wchar_t buffer[128] = {0};
					WASABI_API_LNGSTRINGW_BUF(IDC_PLAY, buffer, ARRAYSIZE(buffer));
					LRESULT idealSize = MLSkinnedButton_GetIdealSize(GetDlgItem(hwnd, IDC_PLAY), buffer);

					SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1),
							rc.bottom - WASABI_API_APP->getScaleY(HIWORD(idealSize)),
							rc.right, rc.bottom);
					rc.bottom = rg.top - WASABI_API_APP->getScaleY(3);
					break;
				}
				case GROUP_MAIN:
					SetRect(&rg, rc.left + WASABI_API_APP->getScaleX(1), rc.top, rc.right, rc.bottom);
					break;
			}
			continue;
		}
		if (skipgroup) continue;

		pl->id   = controls[index];
		pl->hwnd = GetDlgItem(hwnd, pl->id);

		if ( !pl->hwnd )
			continue;

		GetWindowRect(pl->hwnd, &ri);
		MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&ri, 2);
		pl->flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS;

		switch (pl->id)
		{
			case IDC_PLAY:
			case IDC_ENQUEUE:
			case IDC_CUSTOM:
			case IDC_REMOVE:
			case IDC_CLEANUP:
				if ( IDC_CUSTOM != pl->id || customAllowed )
				{
					if ( groupBtn && pl->id == IDC_PLAY && enqueuedef == 1 )
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					if ( groupBtn && pl->id == IDC_ENQUEUE && enqueuedef != 1 )
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					if ( groupBtn && ( pl->id == IDC_PLAY || pl->id == IDC_ENQUEUE ) && customAllowed )
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					wchar_t buffer[ 128 ] = { 0 };
					GetWindowTextW( pl->hwnd, buffer, ARRAYSIZE( buffer ) );

					LRESULT idealSize = MLSkinnedButton_GetIdealSize( pl->hwnd, buffer );
					LONG    width     = LOWORD( idealSize ) + WASABI_API_APP->getScaleX( 6 );

					SETLAYOUTPOS( pl, rg.left, rg.bottom - WASABI_API_APP->getScaleY( HIWORD( idealSize ) ), width, WASABI_API_APP->getScaleY( HIWORD( idealSize ) ) );

					pl->flags |= ( ( rg.right - rg.left ) > width ) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;

					if ( SWP_SHOWWINDOW & pl->flags )
						rg.left += ( pl->cx + WASABI_API_APP->getScaleX( 4 ) );
				}
				else
					pl->flags |= SWP_HIDEWINDOW;
				break;
			case IDC_STATUS:
				SETLAYOUTPOS(pl, rg.left, rg.top, rg.right - rg.left, (rg.bottom - rg.top));
				pl->flags |= (pl->cx > 16) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				break;
			case IDC_DOWNLOADLIST:
				pl->flags |= (rg.top < rg.bottom) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS( pl, rg.left, rg.top + WASABI_API_APP->getScaleY( 1 ), rg.right - rg.left + WASABI_API_APP->getScaleY( 1 ), ( rg.bottom - rg.top ) - WASABI_API_APP->getScaleY( 2 ) );
				break;
		}

		SETLAYOUTFLAGS(pl, ri);
		if ( LAYOUTNEEEDUPDATE( pl ) )
		{
			if ( SWP_NOSIZE == ( ( SWP_HIDEWINDOW | SWP_SHOWWINDOW | SWP_NOSIZE ) & pl->flags ) && ri.left == ( pl->x + offsetX ) && ri.top == ( pl->y + offsetY ) && IsWindowVisible( pl->hwnd ) )
			{
				SetRect( &ri, pl->x, pl->y, pl->cx + pl->x, pl->y + pl->cy );
				ValidateRect( hwnd, &ri );
			}

			pl++;
		}
		else if ( ( fRedraw || ( !offsetX && !offsetY ) ) && IsWindowVisible( pl->hwnd ) )
		{
			ValidateRect( hwnd, &ri );
			if ( GetUpdateRect( pl->hwnd, NULL, FALSE ) )
			{
				if ( !rgn )
					rgn = CreateRectRgn( 0, 0, 0, 0 );

				GetUpdateRgn( pl->hwnd, rgn, FALSE );
				OffsetRgn( rgn, pl->x, pl->y );
				InvalidateRgn( hwnd, rgn, FALSE );
			}
		}
	}

	if (pl != layout)
	{
		LAYOUT *pc;
		HDWP hdwp = BeginDeferWindowPos((INT)(pl - layout));
		for(pc = layout; pc < pl && hdwp; pc++)
		{
			hdwp = DeferWindowPos(hdwp, pc->hwnd, NULL, pc->x, pc->y, pc->cx, pc->cy, pc->flags);
		}

		if (hdwp)
			EndDeferWindowPos(hdwp);

		if ( !rgn )
			rgn = CreateRectRgn( 0, 0, 0, 0 );

		if (fRedraw) 
		{
			GetUpdateRgn(hwnd, rgn, FALSE);
			for ( pc = layout; pc < pl && hdwp; pc++ )
			{
				if ( pc->rgn )
				{
					OffsetRgn( pc->rgn, pc->x, pc->y );
					CombineRgn( rgn, rgn, pc->rgn, RGN_OR );
				}
			}

			RedrawWindow(hwnd, NULL, rgn, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
		}

		if (g_rgnUpdate)
		{
			GetUpdateRgn(hwnd, g_rgnUpdate, FALSE);
			for(pc = layout; pc < pl && hdwp; pc++)
			{
				if (pc->rgn) 
				{
					OffsetRgn(pc->rgn, pc->x, pc->y);
					CombineRgn(g_rgnUpdate, g_rgnUpdate, pc->rgn, RGN_OR);
				}
			}
		}

		for(pc = layout; pc < pl && hdwp; pc++) if (pc->rgn) DeleteObject(pc->rgn);
	}

	if ( rgn )
		DeleteObject( rgn );

	ValidateRgn(hwnd, NULL);
}

void Downloads_DisplayChange(HWND hwndDlg)
{
	ListView_SetTextColor(downloadList.getwnd(), dialogSkinner.Color(WADLG_ITEMFG));
	ListView_SetBkColor(downloadList.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
	ListView_SetTextBkColor(downloadList.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
	downloadList.SetFont(dialogSkinner.GetFont());
	LayoutWindows(hwndDlg, TRUE);
}

static void DownloadsDialog_SkinControls(HWND hwnd, const INT *itemList, INT itemCount, UINT skinType, UINT skinStyle)
{
	MLSKINWINDOW skinWindow = {0};
	skinWindow.style = skinStyle;
	skinWindow.skinType = skinType;

	for(INT i = 0; i < itemCount; i++)
	{
		skinWindow.hwndToSkin = GetDlgItem(hwnd, itemList[i]);
		if (NULL != skinWindow.hwndToSkin)
		{
			MLSkinWindow(plugin.hwndLibraryParent, &skinWindow);
		}
	}
}

static void DownloadDialog_InitializeList(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_DOWNLOADLIST);
	if (NULL == hControl) return;

	UINT styleEx = (UINT)GetWindowLongPtr(hControl, GWL_EXSTYLE);
	SetWindowLongPtr(hControl, GWL_EXSTYLE, styleEx & ~WS_EX_NOPARENTNOTIFY);

	styleEx = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
	SendMessage(hControl, LVM_SETEXTENDEDLISTVIEWSTYLE, styleEx, styleEx);
	SendMessage(hControl, LVM_SETUNICODEFORMAT, (WPARAM)TRUE, 0L);

	MLSKINWINDOW skinWindow;
	skinWindow.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_ALTERNATEITEMS;
	skinWindow.skinType = SKINNEDWND_TYPE_LISTVIEW;
	skinWindow.hwndToSkin = hControl;
	MLSkinWindow(plugin.hwndLibraryParent, &skinWindow);
}

bool COL_SOURCE_Sort(const DownloadListItem* item1, const DownloadListItem* item2)
{
	if (item1->f && item2->f)
		return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, (item1->f->source), -1, (item2->f->source), -1));
	else if (!item1->f && !item2->f)
		return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, (item1->source), -1, (item2->source), -1));
	else if (!item1->f)
		return (FALSE == downloadsSortAscending)?0:1;
	else //if (!item2->f)
		return (FALSE == downloadsSortAscending)?1:0;

	//return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
	//										 (item1->f?item1->f->source:item1->source), -1,
	//										 (item2->f?item2->f->source:item2->source), -1));
}

bool COL_TITLE_Sort(const DownloadListItem* item1, const DownloadListItem* item2)
{
	if (item1->f && item2->f)
		return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, (item1->f->title), -1, (item2->f->title), -1));
	else if (!item1->f && !item2->f)
		return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, (item1->title), -1, (item2->title), -1));
	else if (!item1->f)
		return (FALSE == downloadsSortAscending)?0:1;
	else //if (!item2->f)
		return (FALSE == downloadsSortAscending)?1:0;

	//return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
	//										 (item1->f?item1->f->title:item1->title), -1,
	//										 (item2->f?item2->f->title:item2->title), -1));
}

bool COL_PROGRESS_Sort( const DownloadListItem *item1, const DownloadListItem *item2 )
{
	if ( item1->f && item2->f )
		return ( item1->f->downloadStatus > item2->f->downloadStatus );
	else if ( !item1->f && !item2->f )
		return ( item1->token < item2->token );
	else if ( !item1->f )
		return ( FALSE == downloadsSortAscending ) ? 0 : 1;
	else //if (!item2->f)
		return ( FALSE == downloadsSortAscending ) ? 1 : 0;

	//return ((item1->f?item1->f->downloadStatus:-1) < (item2->f?item2->f->downloadStatus:-1));

	//return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
	//										(item1->f?GetDownloadStatus(item1->f->downloadStatus):item1->status), -1,
	//										(item2->f?GetDownloadStatus(item2->f->downloadStatus):item2->status), -1));
}

bool COL_PATH_Sort(const DownloadListItem* item1, const DownloadListItem* item2)
{
	if (item1->f && item2->f)
		return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, (item1->f->path), -1, (item2->f->path), -1));
	else if (!item1->f && !item2->f)
		return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, (item1->path), -1, (item2->path), -1));
	else if (!item1->f)
		return (FALSE == downloadsSortAscending)?0:1;
	else //if (!item2->f)
		return (FALSE == downloadsSortAscending)?1:0;

	//return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
	//										 (item1->f?item1->f->path:item1->path), -1,
	//										 (item2->f?item2->f->path:item2->path), -1));
}

bool COL_DATE_Sort(const DownloadListItem* item1, const DownloadListItem* item2)
{
	if (item1->f && item2->f)
		return item1->f->downloadDate < item2->f->downloadDate;
	else if (!item1->f && !item2->f)
		return item1->token < item2->token;
	else if (!item1->f)
		return (FALSE == downloadsSortAscending)?0:1;
	else //if (!item2->f)
		return (FALSE == downloadsSortAscending)?1:0;
}

bool COL_SIZE_Sort(const DownloadListItem* item1, const DownloadListItem* item2)
{
	if (item1->f && item2->f)
		return item1->f->totalSize < item2->f->totalSize;
	else if (!item1->f && !item2->f)
		return item1->token < item2->token;
	else if (!item1->f)
		return (FALSE == downloadsSortAscending)?0:1;
	else //if (!item2->f)
		return (FALSE == downloadsSortAscending)?1:0;
}

static BOOL Downloads_SortItems(int sortColumn)
{	
	AutoLock lock (downloadedFiles);
	switch (sortColumn)
	{
		case COL_TITLE:
			std::sort(listContents.begin(), listContents.end(), COL_TITLE_Sort);
			if (FALSE == downloadsSortAscending) std::reverse(listContents.begin(), listContents.end());
			return TRUE;
		case COL_PROGRESS:
			std::sort(listContents.begin(), listContents.end(), COL_PROGRESS_Sort);
			if (FALSE == downloadsSortAscending) std::reverse(listContents.begin(), listContents.end());
			return TRUE;
		case COL_DATE:
			std::sort(listContents.begin(), listContents.end(), COL_DATE_Sort);
			if (FALSE == downloadsSortAscending) std::reverse(listContents.begin(), listContents.end());
			return TRUE;
		case COL_SOURCE:
			std::sort(listContents.begin(), listContents.end(), COL_SOURCE_Sort);
			if (FALSE == downloadsSortAscending) std::reverse(listContents.begin(), listContents.end());
			return TRUE;
		case COL_SIZE:
			std::sort(listContents.begin(), listContents.end(), COL_SIZE_Sort);
			if (FALSE == downloadsSortAscending) std::reverse(listContents.begin(), listContents.end());
			return TRUE;
		case COL_PATH:
			std::sort(listContents.begin(), listContents.end(), COL_PATH_Sort);
			if (FALSE == downloadsSortAscending) std::reverse(listContents.begin(), listContents.end());
			return TRUE;
	}
	return FALSE;
}

static void Downloads_SetListSortColumn(HWND hwnd, INT listId, INT index, BOOL fAscending)
{
	HWND hItems = GetDlgItem(hwnd, listId);
	if (NULL == hItems) return;
	
	HWND hHeader = (HWND)SNDMSG(hItems, LVM_GETHEADER, 0, 0L);
	if (NULL == hHeader) return;
		
	HDITEM item;
	item.mask = HDI_FORMAT;
	// reset first (ml req)
	INT count = (INT)SNDMSG(hHeader, HDM_GETITEMCOUNT, 0, 0L);
	for (INT i = 0; i < count; i++)
	{
		if (index != i && FALSE != (BOOL)SNDMSG(hHeader, HDM_GETITEM, i, (LPARAM)&item))
		{
			if (0 != ((HDF_SORTUP | HDF_SORTDOWN) & item.fmt))
			{	
				item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
				SNDMSG(hHeader, HDM_SETITEM, i, (LPARAM)&item);
			}
		}
	}

	if (FALSE != (BOOL)SNDMSG(hHeader, HDM_GETITEM, index, (LPARAM)&item))
	{
		INT fmt = item.fmt & ~(HDF_SORTUP | HDF_SORTDOWN);
		fmt |= (FALSE == fAscending) ? HDF_SORTDOWN : HDF_SORTUP;
		if (fmt != item.fmt)
		{
			item.fmt = fmt;
			SNDMSG(hHeader, HDM_SETITEM, index, (LPARAM)&item);
		}
	}
}

static BOOL Downloads_Sort(HWND hwnd, INT iColumn, bool fAscending)
{
	BOOL result = TRUE;
	downloadsSortAscending = fAscending;
	Downloads_SortItems(iColumn);
	Downloads_SetListSortColumn(hwnd, IDC_DOWNLOADLIST, iColumn, fAscending);

	if (FALSE != result)
	{
		HWND hItems = GetDlgItem(hwnd, IDC_DOWNLOADLIST);
		if (NULL != hItems) 
			InvalidateRect(hItems, NULL, TRUE);
	}

	return TRUE;
}

void Downloads_UpdateButtonText(HWND hwndDlg, int _enqueuedef)
{
	if (groupBtn)
	{
		switch(_enqueuedef)
		{
			case 1:
				SetDlgItemTextW(hwndDlg, IDC_PLAY, view.enqueue);
				customAllowed = FALSE;
			break;

			default:
				// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
				//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
				pluginMessage p = {ML_MSG_VIEW_BUTTON_HOOK_IN_USE, (INT_PTR)_enqueuedef, 0, 0};

				wchar_t *pszTextW = (wchar_t *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
				if (pszTextW && pszTextW[0] != 0)
				{
					// set this to be a bit different so we can just use one button and not the
					// mixable one as well (leaving that to prevent messing with the resources)
					SetDlgItemTextW(hwndDlg, IDC_PLAY, pszTextW);
					customAllowed = TRUE;
				}
				else
				{
					SetDlgItemTextW(hwndDlg, IDC_PLAY, view.play);
					customAllowed = FALSE;
				}
			break;
		}
	}
}

static void Downloads_ManageButtons( HWND hwndDlg )
{
	int has_selection = downloadList.GetSelectedCount();

	const int buttonids[] = { IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM, IDC_REMOVE };
	for ( size_t i = 0; i != sizeof( buttonids ) / sizeof( buttonids[ 0 ] ); i++ )
	{
		HWND controlHWND = GetDlgItem( hwndDlg, buttonids[ i ] );
		EnableWindow( controlHWND, has_selection );
	}
}

void Downloads_Init(HWND hwndDlg)
{
	HWND hLibrary = plugin.hwndLibraryParent;
	downloads_window = hwndDlg;

	if (!view.play)
	{
		SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view);
	}

	HACCEL accel = WASABI_API_LOADACCELERATORSW(IDR_VIEW_DOWNLOAD_ACCELERATORS);
	if (accel)
		WASABI_API_APP->app_addAccelerators(hwndDlg, &accel, 1, TRANSLATE_MODE_CHILD);

	g_context_menus2 = WASABI_API_LOADMENU(IDR_MENU1);
	groupBtn = ML_GROUPBTN_VAL();
	enqueuedef = (ML_ENQDEF_VAL() == 1);

	// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
	//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
	pluginMessage p = {ML_MSG_VIEW_BUTTON_HOOK, (INT_PTR)hwndDlg, (INT_PTR)MAKELONG(IDC_CUSTOM, IDC_ENQUEUE), (INT_PTR)L"ml_downloads"};
	wchar_t *pszTextW = (wchar_t *)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p);
	if (pszTextW && pszTextW[0] != 0)
	{
		// set this to be a bit different so we can just use one button and not the
		// mixable one as well (leaving that to prevent messing with the resources)
		customAllowed = TRUE;
		SetDlgItemTextW(hwndDlg, IDC_CUSTOM, pszTextW);
	}
	else
		customAllowed = FALSE;

	MLSkinWindow2(hLibrary, hwndDlg, SKINNEDWND_TYPE_AUTO, SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	const INT szControls[] = {IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM};
	DownloadsDialog_SkinControls(hwndDlg, szControls, ARRAYSIZE(szControls), SKINNEDWND_TYPE_AUTO,
								 SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | (groupBtn ? SWBS_SPLITBUTTON : 0));

	const INT szControlz[] = {IDC_REMOVE, IDC_CLEANUP, IDC_STATUS};
	DownloadsDialog_SkinControls(hwndDlg, szControlz, ARRAYSIZE(szControlz), SKINNEDWND_TYPE_AUTO,
								 SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	DownloadDialog_InitializeList(hwndDlg);
	Downloads_UpdateStatusBar(hwndDlg);

	downloadList.setwnd(GetDlgItem(hwndDlg, IDC_DOWNLOADLIST));
	downloadList.AddCol(WASABI_API_LNGSTRINGW(IDS_TITLE), downloadsTitleWidth);
	downloadList.AddCol(WASABI_API_LNGSTRINGW(IDS_PROGRESS), downloadsProgressWidth);
	downloadList.AddCol(WASABI_API_LNGSTRINGW(IDS_DATE), downloadsDateWidth);
	downloadList.AddCol(WASABI_API_LNGSTRINGW(IDS_SOURCE), downloadsSourceWidth);
	downloadList.AddCol(WASABI_API_LNGSTRINGW(IDS_SIZE), downloadsSizeWidth);
	downloadList.AddCol(WASABI_API_LNGSTRINGW(IDS_PATH), downloadsPathWidth);

    DownloadsUpdated();

	downloadList.SetVirtualCount((int)listContents.size());
	Downloads_UpdateButtonText(hwndDlg, enqueuedef == 1);
	Downloads_ManageButtons(hwndDlg);
	Downloads_DisplayChange(hwndDlg);
	Downloads_Sort(hwndDlg, downloadsItemSort, downloadsSortAscending);
	SetTimer(hwndDlg, DOWNLOADSDIALOG_TIMER_UPDATESTATUSBAR , 1000, 0);
}

void Downloads_Timer( HWND hwndDlg, UINT timerId )
{
	switch ( timerId )
	{
		case DOWNLOADSDIALOG_TIMER_UPDATESTATUSBAR:
			Downloads_UpdateStatusBar( hwndDlg );
			{
				AutoLock lock( downloadStatus.statusLock );
				for ( DownloadListItem *l_content : listContents )
				{
					if ( l_content->token )
					{
						size_t d = downloadStatus.downloads[ l_content->token ].downloaded;
						size_t s = downloadStatus.downloads[ l_content->token ].maxSize;

						if ( s )
							StringCchPrintf( l_content->status, 20, WASABI_API_LNGSTRINGW( IDS_DOWNLOADING_PERCENT ), (int)( d / ( s / 100 ) ) );
						else
						{
							if ( WAC_API_DOWNLOADMANAGER->IsPending( l_content->token ) )
								WASABI_API_LNGSTRINGW_BUF( IDS_DOWNLOAD_PENDING, l_content->status, 20 );
							else
								WASABI_API_LNGSTRINGW_BUF( IDS_DOWNLOADING, l_content->status, 20 );
						}

						PostMessage( downloadList.getwnd(), LVM_REDRAWITEMS, 0, listContents.size() );
					}
				}
			}
			break;
	}
}

static INT Downloads_GetListSortColumn(HWND hwnd, INT listId, bool *fAscending)
{
	HWND hItems = GetDlgItem(hwnd, listId);
	if (NULL != hItems)
	{
		HWND hHeader = (HWND)SNDMSG(hItems, LVM_GETHEADER, 0, 0L);
		if (NULL != hHeader)
		{
			HDITEM item;
			item.mask = HDI_FORMAT;

			INT count = (INT)SNDMSG(hHeader, HDM_GETITEMCOUNT, 0, 0L);
			for (INT i = 0; i < count; i++)
			{
				if (FALSE != (BOOL)SNDMSG(hHeader, HDM_GETITEM, i, (LPARAM)&item) &&
					0 != ((HDF_SORTUP | HDF_SORTDOWN) & item.fmt))
				{
					if (NULL != fAscending)
					{
						*fAscending = (0 != (HDF_SORTUP & item.fmt));
					}
					return i;
				}
			}
		}
	}
	return -1;
}

void Downloads_Destroy( HWND hwndDlg )
{
	downloads_window       = 0;
	downloadsSourceWidth   = downloadList.GetColumnWidth( COL_SOURCE );
	downloadsTitleWidth    = downloadList.GetColumnWidth( COL_TITLE );
	downloadsProgressWidth = downloadList.GetColumnWidth( COL_PROGRESS );
	downloadsPathWidth     = downloadList.GetColumnWidth( COL_PATH );
	downloadsDateWidth     = downloadList.GetColumnWidth( COL_DATE );
	downloadsSizeWidth     = downloadList.GetColumnWidth( COL_SIZE );

	for ( DownloadListItem *l_content : listContents )
		delete l_content;

	listContents.clear();

	downloadList.setwnd( NULL );

	bool fAscending;
	downloadsItemSort = Downloads_GetListSortColumn( hwndDlg, IDC_DOWNLOADLIST, &fAscending );
	downloadsSortAscending = ( -1 != downloadsItemSort ) ? ( FALSE != fAscending ) : true;

	int activeDownloads  = 0;
	int historyDownloads = 0;
	{
		Nullsoft::Utility::AutoLock historylock( downloadedFiles.downloadedLock );
		Nullsoft::Utility::AutoLock statuslock( downloadStatus.statusLock );
		historyDownloads = (int)downloadedFiles.downloadList.size();
		activeDownloads  = (int)downloadStatus.downloads.size();
	}

	if ( !activeDownloads && !historyDownloads && !no_auto_hide )
	{
		HNAVITEM hItem = MLNavCtrl_FindItemById( plugin.hwndLibraryParent, downloads_treeItem );
		if ( hItem )
		{
			MLNavCtrl_DeleteItem( plugin.hwndLibraryParent, hItem );
			downloads_treeItem = 0;
		}
	}
}

void Downloads_Remove( bool del = false, HWND parent = NULL )
{
	int d = -1;
	int r = 0;
	while ( GetDownload( d ) )
	{
		int download = d - r;
		DownloadListItem *item = listContents[ download ];
		if ( item->f )
		{
			AutoLock lock( downloadedFiles );
			int j = 0;
			for ( DownloadList::iterator i = downloadedFiles.begin(); i != downloadedFiles.end(); ++i )
			{
				if ( !_wcsicmp( i->path, item->f->path ) )
				{
					if ( del )
					{
						if ( !downloadedFiles.RemoveAndDelete( j ) )
							MessageBox( parent, WASABI_API_LNGSTRINGW( IDS_DELETEFAILED ), downloadedFiles.downloadList[ j ].path, 0 );
					}
					else
						downloadedFiles.Remove( j );

					delete item;
					listContents.erase( listContents.begin() + download );
					r++;
					dirty++;

					break;
				}

				++j;
			}
		}
		else if ( item->token )
		{
			AutoLock lock( downloadStatus.statusLock );
			downloadStatus.downloads[ item->token ].killswitch = 1;
			delete item;
			listContents.erase( listContents.begin() + download );
			r++;
		}
		else
		{
			delete item;
			listContents.erase( listContents.begin() + download );
			r++;
		}
	}

	downloadList.SetVirtualCountAsync( (int)listContents.size() );
	downloadList.UnselectAll();
	//	Navigation_ShowService(SERVICE_DOWNLOADS, SHOWMODE_AUTO);
}

void Downloads_Delete( HWND parent )
{
	wchar_t message[ 256 ] = { 0 };
	int c = downloadList.GetSelectedCount();
	
	if ( !c )
		return;
	else if ( c == 1 )
		WASABI_API_LNGSTRINGW_BUF( IDS_PERM_DELETE_ARE_YOU_SURE, message, 256 );
	else
		StringCchPrintf( message, 256, WASABI_API_LNGSTRINGW( IDS_PERM_DELETE_THESE_ARE_YOU_SURE ), c );

	if ( MessageBox( NULL, message, WASABI_API_LNGSTRINGW( IDS_DELETION ), MB_ICONWARNING | MB_YESNO ) == IDNO )
		return;

	Downloads_Remove( true, parent );
}

void Downloads_CleanUp(HWND hwndDlg)
{
	wchar_t titleStr[64] = {0};
	if ( MessageBox( hwndDlg, WASABI_API_LNGSTRINGW( IDS_CLEAR_ALL_FINISHED_DOWNLOADS ), WASABI_API_LNGSTRINGW_BUF( IDS_CLEAN_UP_LIST, titleStr, 64 ), MB_ICONWARNING | MB_YESNO ) == IDNO )
		return;

	{
		AutoLock lock( downloadedFiles );
		downloadedFiles.downloadList.clear();
	}

	dirty++;

	DownloadsUpdated();
}

void Downloads_InfoBox( HWND parent )
{
	int download = -1;
	if ( GetDownload( download ) )
	{
		const wchar_t *fn;
		if ( listContents[ download ]->f )
			fn = listContents[ download ]->f->path;
		else
			fn = listContents[ download ]->path;

		if ( fn )
		{
			infoBoxParamW p = { parent, fn };
			SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&p, IPC_INFOBOXW );
		}
	}
}

void Downloads_SelectAll()
{
	int l = downloadList.GetCount();
	for ( int i = 0; i < l; i++ )
		downloadList.SetSelected( i );
}

static void exploreItemFolder( HWND hwndDlg )
{
	if ( downloadList.GetSelectionMark() >= 0 )
	{
		int download = -1;
		while ( GetDownload( download ) )
		{
			wchar_t *file;
			if ( listContents[ download ]->f )
				file = listContents[ download ]->f->path;
			else
				file = listContents[ download ]->path;

			WASABI_API_EXPLORERFINDFILE->AddFile( file );
		}
		WASABI_API_EXPLORERFINDFILE->ShowFiles();
	}
}

void Downloads_Cancel()
{
	int l_selected_count = downloadList.GetSelectedCount();
	for ( int i = -1; i < l_selected_count; ++i )
	{
		if ( GetDownload( i ) )
		{
			dirty++;
			if ( !listContents[ i ]->f )
				WAC_API_DOWNLOADMANAGER->CancelDownload( listContents[ i ]->token );

			break;  // Workaround for 5.9.1 to avoid crash if cancel of many downloads in same time
		}
	}
}

int we_are_drag_and_dropping = 0;

static void Downloads_OnColumnClick(HWND hwnd, NMLISTVIEW *plv)
{
	bool fAscending;
	INT iSort = Downloads_GetListSortColumn(hwnd, IDC_DOWNLOADLIST, &fAscending);
	fAscending = (-1 != iSort && iSort == plv->iSubItem) ? (!fAscending) : true;
	Downloads_Sort(hwnd, plv->iSubItem, fAscending);
}

LRESULT DownloadList_Notify( LPNMHDR l, HWND hwndDlg )
{
	switch ( l->code )
	{
		case LVN_COLUMNCLICK:
			Downloads_OnColumnClick( hwndDlg, (NMLISTVIEW *)l );
			break;
		case NM_DBLCLK:
			Downloads_Play( ( ( !!( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ) ^ ML_ENQDEF_VAL() ) );
			break;
		case LVN_BEGINDRAG:
			we_are_drag_and_dropping = 1;
			SetCapture( hwndDlg );
			break;
		case LVN_ITEMCHANGED:
			Downloads_ManageButtons( hwndDlg );
			break;
		case LVN_GETDISPINFO:
			NMLVDISPINFO *lpdi = (NMLVDISPINFO *)l;
			size_t item = lpdi->item.iItem;

			if ( item < 0 || item >= listContents.size() )
				return 0;

			//if (FALSE == downloadsSortAscending) item = listContents.size() - item - 1;

			DownloadListItem *l = listContents[ item ];

			if ( lpdi->item.mask & LVIF_TEXT )
			{
				lpdi->item.pszText[ 0 ] = 0;
				switch ( lpdi->item.iSubItem )
				{
					case COL_TITLE:
						if ( !l->token && l->f )
						{
							wchar_t *l_title = L"";

							if ( l->f->title != NULL )
								l_title = l->f->title;

							lstrcpyn( lpdi->item.pszText, l_title, lpdi->item.cchTextMax );
						}
						else
						{
							if ( l->title ) lstrcpyn( lpdi->item.pszText, l->title, lpdi->item.cchTextMax );
						}
						break;
					case COL_PROGRESS:
						if ( !l->token && l->f )
						{
							switch ( l->f->downloadStatus )
							{
								case DownloadedFile::DOWNLOAD_SUCCESS:
									WASABI_API_LNGSTRINGW_BUF( IDS_DOWNLOAD_SUCCESS, lpdi->item.pszText, lpdi->item.cchTextMax );
									break;
								case DownloadedFile::DOWNLOAD_FAILURE:
									WASABI_API_LNGSTRINGW_BUF( IDS_DOWNLOAD_FAILURE, lpdi->item.pszText, lpdi->item.cchTextMax );
									break;
								case DownloadedFile::DOWNLOAD_CANCELED:
									WASABI_API_LNGSTRINGW_BUF( IDS_DOWNLOAD_CANCELED, lpdi->item.pszText, lpdi->item.cchTextMax );
									break;
							}
						}
						else lstrcpyn( lpdi->item.pszText, l->status, lpdi->item.cchTextMax );
						break;
					case COL_DATE:
					{
						if ( !l->token && l->f && l->f->downloadDate )
						{
							wchar_t tmp[ 128 ] = { 0 };
							MakeDateString( l->f->downloadDate, tmp, 128 );
							lstrcpyn( lpdi->item.pszText, tmp, lpdi->item.cchTextMax );
						}
						else
						{
							WASABI_API_LNGSTRINGW_BUF( IDS_N_A, lpdi->item.pszText, lpdi->item.cchTextMax );
						}
						break;
					}
					case COL_SOURCE:
						if ( !l->token && l->f )
						{
							wchar_t *l_source = L"";

							if ( l->f->source != NULL )
								l_source = l->f->source;

							lstrcpyn( lpdi->item.pszText, l_source, lpdi->item.cchTextMax );
						}
						else
							lstrcpyn( lpdi->item.pszText, l->source, lpdi->item.cchTextMax );
						break;
					case COL_SIZE:
					{
						if ( !l->token && l->f && l->f->totalSize > 0 )
							WASABI_API_LNG->FormattedSizeString( lpdi->item.pszText, lpdi->item.cchTextMax, l->f->totalSize );
						else
							WASABI_API_LNGSTRINGW_BUF( IDS_N_A, lpdi->item.pszText, lpdi->item.cchTextMax );
						break;
					}
					case COL_PATH:
						if ( !l->token && l->f )
						{
							wchar_t *l_path = L"";

							if ( l->f->path != NULL )
								l_path = l->f->path;

							lstrcpyn( lpdi->item.pszText, l_path, lpdi->item.cchTextMax );
						}
						else
						{
							if ( l->path )
								lstrcpyn( lpdi->item.pszText, l->path, lpdi->item.cchTextMax );
						}

						break;
				}
			}

			break;
	}

	return 0;
}

void listbuild( wchar_t **buf, int &buf_size, int &buf_pos, const wchar_t *tbuf )
{
	if ( !*buf )
	{
		*buf = (wchar_t *)calloc( 4096, sizeof( wchar_t ) );
		if ( *buf )
		{
			buf_size = 4096;
			buf_pos = 0;
		}
		else
		{
			buf_size = buf_pos = 0;
		}
	}
	int newsize = buf_pos + lstrlenW( tbuf ) + 1;
	if ( newsize < buf_size )
	{
		size_t old_buf_size = buf_size;
		buf_size = newsize + 4096;
		wchar_t *new_buf = (wchar_t *)realloc( *buf, ( buf_size + 1 ) * sizeof( wchar_t ) );
		if ( new_buf )
		{
			*buf = new_buf;
		}
		else
		{
			new_buf = (wchar_t*)calloc( ( buf_size + 1 ), sizeof( wchar_t ) );
			if ( new_buf )
			{
				memcpy( new_buf, *buf, ( old_buf_size * sizeof( wchar_t ) ) );
				free( *buf );
				*buf = new_buf;
			}
			else buf_size = (int)old_buf_size;
		}
	}

	StringCchCopyW( *buf + buf_pos, buf_size, tbuf );
	buf_pos = newsize;
}

wchar_t *getSelectedList()
{
	wchar_t *path     = NULL;
	int      buf_pos  = 0;
	int      buf_size = 0;
	int      download = -1;

	while ( GetDownload( download ) )
	{
		if ( listContents[ download ]->f )
			listbuild( &path, buf_size, buf_pos, listContents[ download ]->f->path );
	}

	if ( path )
		path[ buf_pos ] = 0;

	return path;
}

static void SwapPlayEnqueueInMenu( HMENU listMenu )
{
	int playPos = -1, enqueuePos = -1;
	MENUITEMINFOW playItem = { sizeof( MENUITEMINFOW ), 0, }, enqueueItem = { sizeof( MENUITEMINFOW ), 0, };

	int numItems = GetMenuItemCount( listMenu );

	for ( int i = 0; i < numItems; i++ )
	{
		UINT id = GetMenuItemID( listMenu, i );
		if ( id == IDC_PLAY )
		{
			playItem.fMask = MIIM_ID;
			playPos = i;
			GetMenuItemInfoW( listMenu, i, TRUE, &playItem );
		}
		else if ( id == IDC_ENQUEUE )
		{
			enqueueItem.fMask = MIIM_ID;
			enqueuePos        = i;
			GetMenuItemInfoW( listMenu, i, TRUE, &enqueueItem );
		}
	}

	playItem.wID = IDC_ENQUEUE;
	enqueueItem.wID = IDC_PLAY;
	SetMenuItemInfoW( listMenu, playPos, TRUE, &playItem );
	SetMenuItemInfoW( listMenu, enqueuePos, TRUE, &enqueueItem );
}

static void SyncMenuWithAccelerators( HWND hwndDlg, HMENU menu )
{
	HACCEL szAccel[ 24 ] = { 0 };
	INT c = WASABI_API_APP->app_getAccelerators( hwndDlg, szAccel, sizeof( szAccel ) / sizeof( szAccel[ 0 ] ), FALSE );
	AppendMenuShortcuts( menu, szAccel, c, MSF_REPLACE );
}

void UpdateMenuItems( HWND hwndDlg, HMENU menu )
{
	bool swapPlayEnqueue = false;
	if ( ML_ENQDEF_VAL() )
	{
		SwapPlayEnqueueInMenu( menu );
		swapPlayEnqueue = true;
	}

	SyncMenuWithAccelerators( hwndDlg, menu );
	if ( swapPlayEnqueue )
		SwapPlayEnqueueInMenu( menu );
}

static int IPC_LIBRARY_SENDTOMENU = 0;
static librarySendToMenuStruct s = { 0 };

static void DownloadList_RightClick(HWND hwndDlg, HWND listHwnd, POINTS pts)
{
	POINT pt;
	POINTSTOPOINT(pt, pts);

	RECT controlRect, headerRect;
	if (FALSE == GetClientRect(listHwnd, &controlRect))
		SetRectEmpty(&controlRect);
	else
		MapWindowPoints(listHwnd, HWND_DESKTOP, (POINT*)&controlRect, 2);

	if ( -1 == pt.x && -1 == pt.y )
	{
		RECT itemRect;
		int selected = downloadList.GetNextSelected();
		if ( selected != -1 ) // if something is selected we'll drop the menu from there
		{
			downloadList.GetItemRect( selected, &itemRect );
			ClientToScreen( listHwnd, (POINT *)&itemRect );
		}
		else // otherwise we'll drop it from the top-left corner of the listview, adjusting for the header location
		{
			GetWindowRect( listHwnd, &itemRect );

			HWND hHeader = (HWND)SNDMSG( listHwnd, LVM_GETHEADER, 0, 0L );
			RECT headerRect;
			if ( ( WS_VISIBLE & GetWindowLongPtr( hHeader, GWL_STYLE ) ) && GetWindowRect( hHeader, &headerRect ) )
			{
				itemRect.top += ( headerRect.bottom - headerRect.top );
			}
		}

		pt.x = itemRect.left;
		pt.y = itemRect.top;
	}

	HWND hHeader = (HWND)SNDMSG(listHwnd, LVM_GETHEADER, 0, 0L);
	if ( 0 == ( WS_VISIBLE & GetWindowLongPtr( hHeader, GWL_STYLE ) ) || FALSE == GetWindowRect( hHeader, &headerRect ) )
	{
		SetRectEmpty( &headerRect );
	}

	if ( FALSE != PtInRect( &headerRect, pt ) )
	{
		return;
	}

	LVHITTESTINFO hitTest;
	hitTest.pt = pt;
	MapWindowPoints( HWND_DESKTOP, listHwnd, &hitTest.pt, 1 );

	int index = ( downloadList.GetNextSelected() != -1 ? ListView_HitTest( listHwnd, &hitTest ) : -1 );

	HMENU baseMenu = WASABI_API_LOADMENU( IDR_MENU1 );

	if ( baseMenu == NULL )
		return;

	HMENU menu = GetSubMenu( baseMenu, 0 );
	if ( menu != NULL )
	{
		if ( ( index == -1 ) || ( index != -1 ) && listContents[ index ]->f )
			DeleteMenu( menu, ID_DOWNLOADS_CANCELDOWNLOAD, MF_BYCOMMAND );

		UINT enableExtras = MF_ENABLED;
		if ( ( index == -1 ) || ( index != -1 ) && !listContents[ index ]->f
			 || ( index != -1 ) && ( listContents[ index ]->f->downloadStatus != 1 ) )
			enableExtras = ( MF_GRAYED | MF_DISABLED );

		UINT enableViewExtras = MF_ENABLED;
		if ( index == -1 )
			enableViewExtras = ( MF_GRAYED | MF_DISABLED );

		EnableMenuItem( menu, IDC_PLAY, MF_BYCOMMAND | enableExtras );
		EnableMenuItem( menu, IDC_ENQUEUE, MF_BYCOMMAND | enableExtras );
		EnableMenuItem( menu, IDC_REMOVE, MF_BYCOMMAND | enableViewExtras );
		EnableMenuItem( menu, IDC_DELETE, MF_BYCOMMAND | enableExtras );
		EnableMenuItem( menu, IDC_INFOBOX, MF_BYCOMMAND | enableExtras );
		EnableMenuItem( menu, ID_DOWNLOADS_EXPLORERITEMFOLDER, MF_BYCOMMAND | enableExtras );
		EnableMenuItem( menu, 2, MF_BYPOSITION | enableExtras );

		{ // send-to menu shit...
			ZeroMemory( &s, sizeof( s ) );
			IPC_LIBRARY_SENDTOMENU = (int)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, ( WPARAM ) & "LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE );
			if ( IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)0, IPC_LIBRARY_SENDTOMENU ) == 0xffffffff )
			{
				s.mode        = 1;
				s.hwnd        = hwndDlg;
				s.data_type   = ML_TYPE_FILENAMESW;
				s.ctx[ 1 ]    = 1;
				s.build_hMenu = GetSubMenu( menu, 2 );
			}
		}

		UpdateMenuItems( hwndDlg, menu );

		int r = Menu_TrackPopup( plugin.hwndLibraryParent, menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, pt.x, pt.y, hwndDlg, NULL );
		if ( !SendMessage( hwndDlg, WM_COMMAND, r, 0 ) )
		{
			s.menu_id = r; // more send to menu shit...
			if ( s.mode == 2 && SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU ) == 0xffffffff )
			{
				s.mode = 3;
				wchar_t *path = getSelectedList();
				if ( path )
				{
					s.data = path;
					SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU );
					free( path );
				}
			}
		}

		if ( s.mode )
		{ // yet more send to menu shit...
			s.mode = 4;
			SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_LIBRARY_SENDTOMENU ); // cleanup
		}
	}

	DestroyMenu( baseMenu );
}

static void Downloads_ContextMenu( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND sourceWindow = (HWND)wParam;
	if ( sourceWindow == downloadList.getwnd() )
		DownloadList_RightClick( hwndDlg, sourceWindow, MAKEPOINTS( lParam ) );
}

enum
{
	BPM_ECHO_WM_COMMAND=0x1, // send WM_COMMAND and return value
	BPM_WM_COMMAND = 0x2, // just send WM_COMMAND
};

BOOL Downloads_ButtonPopupMenu( HWND hwndDlg, int buttonId, HMENU menu, int flags = 0 )
{
	RECT r;
	HWND buttonHWND = GetDlgItem( hwndDlg, buttonId );
	GetWindowRect( buttonHWND, &r );
	UpdateMenuItems( hwndDlg, menu );
	MLSkinnedButton_SetDropDownState( buttonHWND, TRUE );
	UINT tpmFlags = TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN;
	if ( !( flags & BPM_WM_COMMAND ) )
		tpmFlags |= TPM_RETURNCMD;
	int x = Menu_TrackPopup( plugin.hwndLibraryParent, menu, tpmFlags, r.left, r.top, hwndDlg, NULL );
	if ( ( flags & BPM_ECHO_WM_COMMAND ) && x )
		SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( x, 0 ), 0 );
	MLSkinnedButton_SetDropDownState( buttonHWND, FALSE );
	return x;
}

static void Downloads_Play( HWND hwndDlg, HWND from, UINT idFrom )
{
	HMENU listMenu = GetSubMenu( g_context_menus2, 0 );
	int count = GetMenuItemCount( listMenu );
	if ( count > 2 )
	{
		for ( int i = 2; i < count; i++ )
		{
			DeleteMenu( listMenu, 2, MF_BYPOSITION );
		}
	}

	Downloads_ButtonPopupMenu( hwndDlg, idFrom, listMenu, BPM_WM_COMMAND );
}

static BOOL WINAPI DownloadDialog_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITMENUPOPUP: // yet yet more send to menu shit...
		if (wParam && (HMENU)wParam == s.build_hMenu && s.mode==1)
		{
			if (SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU)==0xffffffff)
				s.mode=2;
		}
			break;

		case WM_USER+543:
			Navigation_Update();
			break;			

		case WM_CONTEXTMENU:
			Downloads_ContextMenu(hwndDlg, wParam, lParam);
			return TRUE;

		case WM_NOTIFYFORMAT:
			return NFR_UNICODE;

		case WM_INITDIALOG:
			Downloads_Init(hwndDlg);
			break;

		case WM_NOTIFY:
		{
			LPNMHDR l = (LPNMHDR)lParam;
			if (l->idFrom == IDC_DOWNLOADLIST)
				return (BOOL)DownloadList_Notify(l,hwndDlg);
		}
			break;

		case WM_DESTROY:
			Downloads_Destroy(hwndDlg);
			return 0;

		case WM_DISPLAYCHANGE:
			Downloads_DisplayChange(hwndDlg);
			return 0;

		case WM_TIMER:
			Downloads_Timer(hwndDlg, (UINT)wParam);
			break;

		case WM_MOUSEMOVE:
			if (we_are_drag_and_dropping && GetCapture() == hwndDlg)
			{
				POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				ClientToScreen(hwndDlg, &p);
				mlDropItemStruct m;
				ZeroMemory(&m, sizeof(mlDropItemStruct));
				m.type = ML_TYPE_FILENAMESW;
				m.p = p;
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDRAG);
			}
			break;

		case WM_LBUTTONUP:
			if (we_are_drag_and_dropping && GetCapture() == hwndDlg)
			{
				we_are_drag_and_dropping = 0;
				ReleaseCapture();
				POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				ClientToScreen(hwndDlg, &p);
				mlDropItemStruct m = {0};
				m.type = ML_TYPE_FILENAMESW;
				m.p = p;
				m.flags = ML_HANDLEDRAG_FLAG_NOCURSOR;
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDRAG);
				if (m.result > 0)
				{
					m.flags = 0;
					m.result = 0;
					wchar_t* path = getSelectedList();
					if(path)
					{
						m.data = path;
						SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDROP);
						free(path);
					}
				}
			}
			break;

		case WM_PAINT:
			{
				int tab[] = { IDC_DOWNLOADLIST|DCW_SUNKENBORDER};
				dialogSkinner.Draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
			}
			return 0;

		case WM_WINDOWPOSCHANGED:
			if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & ((WINDOWPOS*)lParam)->flags) || 
				(SWP_FRAMECHANGED & ((WINDOWPOS*)lParam)->flags))
			{
				LayoutWindows(hwndDlg, !(SWP_NOREDRAW & ((WINDOWPOS*)lParam)->flags));
			}
			return 0;

		case WM_USER + 0x200:
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 1); // yes, we support no - redraw resize
			return TRUE;

		case WM_USER + 0x201:
			offsetX = (short)LOWORD(wParam);
			offsetY = (short)HIWORD(wParam);
			g_rgnUpdate = (HRGN)lParam;
			return TRUE;

		case WM_APP + 104:
		{
			Downloads_UpdateButtonText(hwndDlg, (int)wParam);
			LayoutWindows(hwndDlg, TRUE);
			return 0;
		}

		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case IDC_PLAY:
				case IDC_ENQUEUE:
				case IDC_CUSTOM:
					if ( HIWORD( wParam ) == MLBN_DROPDOWN )
					{
						Downloads_Play( hwndDlg, (HWND)lParam, LOWORD( wParam ) );
					}
					else
					{
						bool action;
						if ( LOWORD( wParam ) == IDC_PLAY )
						{
							action = ( HIWORD( wParam ) == 1 ) ? ML_ENQDEF_VAL() == 1 : 0;
						}
						else if ( LOWORD( wParam ) == IDC_ENQUEUE )
						{
							action = ( HIWORD( wParam ) == 1 ) ? ML_ENQDEF_VAL() != 1 : 1;
						}
						else
							break;

						Downloads_Play( action );
					}
					break;
				case IDC_REMOVE:
					Downloads_Remove();
					break;
				case IDC_DELETE:
					Downloads_Delete( hwndDlg );
					break;
				case IDC_CLEANUP:
					Downloads_CleanUp( hwndDlg );
					break;
				case IDC_INFOBOX:
					Downloads_InfoBox( hwndDlg );
					break;
				case IDC_SELECTALL:
					Downloads_SelectAll();
					break;
				case ID_DOWNLOADS_EXPLORERITEMFOLDER:
					exploreItemFolder( hwndDlg );
					break;
				case ID_DOWNLOADS_CANCELDOWNLOAD:
					Downloads_Cancel();
					break;
				default:
					return 0;
			}
			return 1;
	}
	return 0;
}

HWND CALLBACK DownloadDialog_Create( HWND hParent )
{
	return WASABI_API_CREATEDIALOGPARAMW( IDD_DOWNLOADS, hParent, DownloadDialog_DlgProc, 0 );
}