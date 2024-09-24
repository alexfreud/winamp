#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <shellapi.h>

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "nu/listview.h"

#include "resource.h"
#include "Playlist.h"
#include "replicant/nu/AutoChar.h"
#include "../../General/gen_ml/ml_ipc.h"
#include "SendTo.h"
#include "api__ml_playlists.h"
#include "../../General/gen_ml/ml_ipc_0313.h"
#include "nu/menushortcuts.h"
#include "ml_local/api_mldb.h"
#include "ml_pmp/pmp.h"
#include "replicant/nswasabi/ReferenceCounted.h"
#include "replicant/nx/win/nxstring.h"

void playlist_UpdateButtonText( HWND hwndDlg, int enqueuedef );
BOOL playlist_ButtonPopupMenu( HWND hwndDlg, int buttonId, HMENU menu, int flags = 0 );

static std::vector<GUID> playlistGUIDs;
using namespace Nullsoft::Utility;
SendToMenu sendTo;
static W_ListView m_playlistslist;

int        root_is_drag_and_dropping = 0;
HINSTANCE  cloud_hinst               = 0;
static int last_item1                = -1;
int        IPC_GET_CLOUD_HINST       = -1;
int        IPC_GET_CLOUD_ACTIVE      = -1;
int        cloud_avail               = 0;
int        normalimage               = 0;
int        cloudImage                = 0;

static void AutoSizePlaylistColumns()
{
	m_playlistslist.AutoSizeColumn( 2 );
	m_playlistslist.AutoSizeColumn( 3 );

	RECT channelRect;
	GetClientRect( m_playlistslist.getwnd(), &channelRect );

	ListView_SetColumnWidth( m_playlistslist.getwnd(), 0, channelRect.right - m_playlistslist.GetColumnWidth( 1 ) - m_playlistslist.GetColumnWidth( 2 ) - m_playlistslist.GetColumnWidth( 3 ) );
}

static bool opened = false, loaded = false;

void RefreshPlaylistsList()
{
	if ( opened )
	{
		playlistGUIDs.clear();

		AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

		size_t count = AGAVE_API_PLAYLISTS->GetCount();
		playlistGUIDs.reserve( count );

		for ( size_t i = 0; i < count; i++ )
			playlistGUIDs.push_back( AGAVE_API_PLAYLISTS->GetGUID( i ) );

		ListView_SetItemCount( m_playlistslist.getwnd(), playlistGUIDs.size() );
		ListView_RedrawItems( m_playlistslist.getwnd(), 0, playlistGUIDs.size() - 1 );
	}
}

void ImportPlaylist( const wchar_t *srcFilename, bool callback = false )
{
	wchar_t l_src_filename[ MAX_PATH ] = { 0 };
	lstrcpynW( l_src_filename, srcFilename, MAX_PATH );

	wchar_t  filename[ MAX_PATH ] = { 0 };
	wchar_t *filenameptr          = ( !g_config->ReadInt( L"external", 0 ) ? createPlayListDBFileName( filename ) : 0 );
	size_t   numItems             = AGAVE_API_PLAYLISTMANAGER->Copy( filename, l_src_filename );

	// get the filename of the imported playlist
	PathRemoveExtensionW( l_src_filename );
	PathStripPathW( l_src_filename );

	// just incase we've had external playlists added / imported
	// we spin through and abort trying to re-add any that match
	if ( g_config->ReadInt( L"external", 0 ) )
	{
		AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

		size_t count = AGAVE_API_PLAYLISTS->GetCount();
		for ( size_t i = 0; i != count; i++ )
		{
			PlaylistInfo info( i );
			if ( info.Valid() )
			{
				if ( !lstrcmpiW( srcFilename, info.GetFilename() ) )
				{
					wchar_t titleStr[ 96 ] = { 0 };
					MessageBox( currentView, WASABI_API_LNGSTRINGW( IDS_EXTERNAL_ALREADY_ADDED ), WASABI_API_LNGSTRINGW_BUF( IDS_PL_FILE_MNGT, titleStr, 96 ), MB_OK | MB_ICONWARNING );

					return;
				}
			}
		}
	}

	if ( l_src_filename[ 0 ] )
		AddPlaylist( ( !callback ? 1 : 2 ), l_src_filename, ( !g_config->ReadInt( L"external", 0 ) ? filenameptr : srcFilename ), 1, g_config->ReadInt( L"cloud", 1 ), numItems );
	else
		AddPlaylist( ( !callback ? 1 : 2 ), WASABI_API_LNGSTRINGW( IDS_IMPORTED_PLAYLIST ), ( !g_config->ReadInt( L"external", 0 ) ? filenameptr : srcFilename ), 1, g_config->ReadInt( L"cloud", 1 ), numItems );
}

void playlists_ImportExternalPrompt( HWND hwndDlg )
{
	// TODO decide if better to show the message on all changes or only once and
	//		then just leave the user to it in the future otherwise leave as it is
	//if (!g_config->ReadInt("external_prompt", 0) && g_config->ReadInt("external", 0))
	if ( !g_config->ReadInt( L"external", 0 ) )
	{
		wchar_t titleStr[ 96 ] = { 0 };
		if ( MessageBox( hwndDlg, WASABI_API_LNGSTRINGW( IDS_EXTERNAL_CHECKED ), WASABI_API_LNGSTRINGW_BUF( IDS_PL_FILE_MNGT, titleStr, 96 ), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 ) == IDYES )
		{
			g_config->WriteInt( L"external", ( IsDlgButtonChecked( hwndDlg, IDC_EXTERNAL ) == BST_CHECKED ) );
		}
		else
		{
			CheckDlgButton( hwndDlg, IDC_EXTERNAL, g_config->ReadInt( L"external", 0 ) );
		}

		g_config->WriteInt( L"external_prompt", 1 );
	}
	else
	{
		g_config->WriteInt( L"external", ( IsDlgButtonChecked( hwndDlg, IDC_EXTERNAL ) == BST_CHECKED ) );
	}
}

UINT_PTR CALLBACK Playlist_OFNHookProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_INITDIALOG )
	{
		int cloud = playlists_CloudAvailable();
		HWND cloudWnd = GetDlgItem( hwndDlg, IDC_CLOUD );
		if ( IsWindow( cloudWnd ) )
		{
			ShowWindow( cloudWnd, cloud );
			CheckDlgButton( hwndDlg, IDC_CLOUD, AddToCloud() );
		}

		CheckDlgButton( hwndDlg, IDC_EXTERNAL, g_config->ReadInt( L"external", 0 ) );

		if ( !cloud )
		{
			HWND external = GetDlgItem( hwndDlg, IDC_EXTERNAL );
			if ( IsWindow( external ) && IsWindow( cloudWnd ) )
			{
				RECT r = { 0 }, cl = { 0 };
				GetWindowRect( external, &r );
				GetWindowRect( cloudWnd, &cl );
				ScreenToClient( hwndDlg, (LPPOINT)&r );
				ScreenToClient( hwndDlg, (LPPOINT)&cl );
				SetWindowPos( external, NULL, cl.left, r.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_SHOWWINDOW );
			}
		}
	}
	else if ( uMsg == WM_COMMAND )
	{
		switch ( LOWORD( wParam ) )
		{
			case IDC_CLOUD:
				playlists_AddToCloudPrompt( hwndDlg );
				return 1;

			case IDC_EXTERNAL:
				playlists_ImportExternalPrompt( hwndDlg );
				return 1;
		}
	}

	return 0;
}

void Playlist_importFromFile( HWND dlgparent )
{
	wchar_t oldCurPath[ MAX_PATH ] = { 0 };
	wchar_t newCurPath[ MAX_PATH ] = { 0 };
	bool skipRes = false;

	GetCurrentDirectoryW( MAX_PATH, oldCurPath );

retry:
	wchar_t temp[ 1024 ]   = { 0 };
	wchar_t filter[ 1024 ] = { 0 };
	AGAVE_API_PLAYLISTMANAGER->GetFilterList( filter, 1024 );

	OPENFILENAMEW l = { sizeof( l ), 0 };
	l.hwndOwner       = dlgparent;
	l.lpstrFilter     = filter;
	l.lpstrFile       = temp;
	l.nMaxFile        = 1024;
	l.lpstrTitle      = WASABI_API_LNGSTRINGW( IDS_IMPORT_PLAYLIST );
	l.lpstrDefExt     = L"m3u";
	l.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();
	l.lpfnHook        = Playlist_OFNHookProc;
	l.Flags           = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLESIZING | OFN_ENABLETEMPLATE;
	l.lpTemplateName  = MAKEINTRESOURCE( IDD_IMPORT_PLFLD );
	l.hInstance       = ( !skipRes ? WASABI_API_LNG_HINST : WASABI_API_ORIG_HINST );

	if ( GetOpenFileNameW( &l ) )
	{
		GetCurrentDirectoryW( MAX_PATH, newCurPath );
		WASABI_API_APP->path_setWorkingPath( newCurPath );

		ImportPlaylist( temp );
	}
	else
	{
		// deals with the extra child dialog not being present in language packs
		// so we re-spin and try to load the native version before just failing
		DWORD res = CommDlgExtendedError();
		if ( res == CDERR_NOTEMPLATE || res == CDERR_FINDRESFAILURE )
		{
			if ( !skipRes )
			{
				skipRes = true;
				goto retry;
			}
		}
	}

	SetCurrentDirectoryW( oldCurPath );
}

void Playlists_ReplaceBadPathChars( LPWSTR pszPath )
{
	if ( NULL == pszPath )
		return;

	while ( L'\0' != *pszPath )
	{
		switch ( *pszPath )
		{
			case L'?':
			case L'/':
			case L'\\':
			case L':':
			case L'*':
			case L'\"':
			case L'<':
			case L'>':
			case L'|':
				*pszPath = L'_';
				break;
			default:
				if ( *pszPath < 32 )
					*pszPath = L'_';
				break;
		}

		pszPath = CharNextW( pszPath );
	}
}

void Playlist_export( HWND dlgparent, const wchar_t *name, const wchar_t *srcm3u )
{
	wchar_t oldCurPath[ MAX_PATH ] = { 0 };
	GetCurrentDirectoryW( MAX_PATH, oldCurPath );

	wchar_t temp[ MAX_PATH ] = { 0 };
	OPENFILENAMEW l = { sizeof( OPENFILENAMEW ), 0 };
	l.hwndOwner       = dlgparent;
	l.hInstance       = plugin.hDllInstance;

	lstrcpynW( temp, name, MAX_PATH );
	Playlists_ReplaceBadPathChars( temp );

	l.nFilterIndex    = g_config->ReadInt( L"filter", 3 );
	l.lpstrFilter     = (LPCWSTR)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 3, IPC_GET_PLAYLIST_EXTLISTW );
	l.lpstrFile       = temp;
	l.nMaxFile        = MAX_PATH;
	l.lpstrTitle      = WASABI_API_LNGSTRINGW( IDS_EXPORT_PLAYLIST );
	l.lpstrDefExt     = L"m3u";
	l.lpstrInitialDir = WASABI_API_APP->path_getWorkingPath();
	l.Flags           = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	if ( GetSaveFileNameW( &l ) )
	{
		wchar_t newCurPath[ MAX_PATH ] = { 0 };
		GetCurrentDirectoryW( MAX_PATH, newCurPath );
		WASABI_API_APP->path_setWorkingPath( newCurPath );

		AGAVE_API_PLAYLISTMANAGER->Copy( temp, srcm3u );
	}

	g_config->WriteInt( L"filter", l.nFilterIndex );
	SetCurrentDirectoryW( oldCurPath );
}

void importPlaylistFolder( const wchar_t *path, int dorecurs )
{
	wchar_t tmppath[ MAX_PATH ] = { 0 };
	PathCombineW( tmppath, path, L"*" );

	WIN32_FIND_DATAW d;
	HANDLE h = FindFirstFileW( tmppath, &d );
	if ( h == INVALID_HANDLE_VALUE )
		return;

	do
	{
		wchar_t l_playlist_folder[ MAX_PATH ] = { 0 };
		PathCombineW( l_playlist_folder, path, d.cFileName );

		if ( d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && lstrcmpW( d.cFileName, L"." ) && lstrcmpW( d.cFileName, L".." ) && dorecurs )
		{
			importPlaylistFolder( l_playlist_folder, dorecurs );
			continue;
		}

		if ( AGAVE_API_PLAYLISTMANAGER->CanLoad( l_playlist_folder ) )
		{
			ImportPlaylist( l_playlist_folder, true );
		}
	} while ( FindNextFileW( h, &d ) != 0 );

	if ( h != INVALID_HANDLE_VALUE )
		FindClose( h );
}

void Shell_Free( void *p )
{
	IMalloc *m;
	SHGetMalloc( &m );

	m->Free( p );
}

static INT_PTR CALLBACK browseCheckBoxProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_INITDIALOG )
	{
		int cloud = playlists_CloudAvailable();
		HWND cloudWnd = GetDlgItem( hwndDlg, IDC_CLOUD );
		if ( IsWindow( cloudWnd ) )
		{
			ShowWindow( cloudWnd, cloud );
			CheckDlgButton( hwndDlg, IDC_CLOUD, AddToCloud() );
		}

		if ( g_config->ReadInt( L"importplfoldrecurs", 1 ) )
			CheckDlgButton( hwndDlg, IDC_CHECK1, BST_CHECKED );

		CheckDlgButton( hwndDlg, IDC_CLOUD,    AddToCloud() );
		CheckDlgButton( hwndDlg, IDC_EXTERNAL, g_config->ReadInt( L"external", 0 ) );
	}

	if ( uMsg == WM_COMMAND )
	{
		if ( LOWORD( wParam ) == IDC_CHECK1 )
		{
			g_config->WriteInt( L"importplfoldrecurs", !!IsDlgButtonChecked( hwndDlg, IDC_CHECK1 ) );
		}
		else if ( LOWORD( wParam ) == IDC_CLOUD )
		{
			playlists_AddToCloudPrompt( hwndDlg );
		}
		else if ( LOWORD( wParam ) == IDC_EXTERNAL )
		{
			playlists_ImportExternalPrompt( hwndDlg );
		}
	}

	return 0;
}

BOOL CALLBACK browseEnumProc( HWND hwnd, LPARAM lParam )
{
	wchar_t cl[ 32 ] = { 0 };
	GetClassNameW( hwnd, cl, ARRAYSIZE( cl ) );
	if ( !lstrcmpiW( cl, WC_TREEVIEW ) )
	{
		PostMessage( hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection( hwnd ) );

		return FALSE;
	}

	return TRUE;
}

int CALLBACK WINAPI _bcp( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	switch ( uMsg )
	{
		case BFFM_INITIALIZED:
		{
			SetWindowText( hwnd, WASABI_API_LNGSTRINGW( IDS_IMPORT_PLAYLIST_FROM_FOLDER ) );
			SendMessageW( hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)WASABI_API_APP->path_getWorkingPath() );

			HWND h2 = FindWindowEx( hwnd, NULL, NULL, L"__foo" );
			if ( h2 )
				ShowWindow( h2, SW_HIDE );

			HWND h = WASABI_API_CREATEDIALOGW( IDD_BROWSE_PLFLD, hwnd, browseCheckBoxProc );
			SetWindowPos( h, 0, 4, 4, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
			ShowWindow( h, SW_SHOWNA );

			// this is not nice but it fixes the selection not working correctly on all OSes
			EnumChildWindows( hwnd, browseEnumProc, 0 );
		}
	}

	return 0;
}

void Playlist_importFromFolders( HWND dlgparent )
{
	BROWSEINFOW bi               = { 0 };
	wchar_t     name[ MAX_PATH ] = { 0 };

	bi.hwndOwner      = dlgparent;
	bi.pszDisplayName = name;
	bi.lpszTitle      = L"__foo";
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn           = _bcp;

	ITEMIDLIST *idlist = SHBrowseForFolderW( &bi );
	if ( idlist )
	{
		wchar_t path[ MAX_PATH ] = { 0 };
		SHGetPathFromIDListW( idlist, path );
		WASABI_API_APP->path_setWorkingPath( path );

		Shell_Free( idlist );
		MLNavCtrl_BeginUpdate( plugin.hwndLibraryParent, 0 );
		importPlaylistFolder( path, g_config->ReadInt( L"importplfoldrecurs", 1 ) );

		AGAVE_API_PLAYLISTS->Flush(); // REVIEW: save immediately? or only at the end?
		MLNavCtrl_EndUpdate( plugin.hwndLibraryParent );
	}
}

static void playlists_Save( HWND parent )
{
	for ( size_t i = 0; i < playlistGUIDs.size(); i++ )
	{
		if ( !m_playlistslist.GetSelected( i ) )
			continue;

		AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

		PlaylistInfo info( playlistGUIDs[ i ] );

		wchar_t str[ MAX_PATH ] = { 0 };
		if ( PathIsFileSpecW( info.GetFilename() ) )
			PathCombineW( str, g_path, info.GetFilename() );
		else
			lstrcpynW( str, info.GetFilename(), MAX_PATH );

		Playlist_export( parent, info.GetName(), str );
	}
}

void playlists_Import( HWND hwndDlg, LPARAM lParam )
{
	RECT r;
	HMENU menu = GetSubMenu( g_context_menus, 2 );

	GetWindowRect( (HWND)lParam, &r );

	int x = Menu_TrackPopup( plugin.hwndLibraryParent, menu, TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD, r.left, r.top, hwndDlg, NULL );
	switch ( x )
	{
		case IDC_IMPORT_PLAYLIST_FROM_FILE:
			Playlist_importFromFile( hwndDlg );
			RefreshPlaylistsList();
			break;
		case IDC_IMPORT_WINAMP_PLAYLIST:
			Playlist_importFromWinamp();
			RefreshPlaylistsList();
			break;
		case ID_PLAYLISTSIMPORT_IMPORTPLAYLISTSFROMFOLDER:
			Playlist_importFromFolders( hwndDlg );
			RefreshPlaylistsList();
			break;
	}
}

void playlists_Add( HWND parent, bool callback )
{
	WASABI_API_DIALOGBOXPARAMW( ( playlists_CloudAvailable() ? IDD_ADD_CLOUD_PLAYLIST : IDD_ADD_PLAYLIST ), parent, AddPlaylistDialogProc, callback );
}

void DeletePlaylist( GUID _guid, HWND parent, bool confirm )
{
	wchar_t titleStr[ 32 ] = { 0 };
	if ( confirm && MessageBox( parent, WASABI_API_LNGSTRINGW( IDS_CONFIRM_DELETION ), WASABI_API_LNGSTRINGW_BUF( IDS_CONFIRMATION, titleStr, 32 ), MB_YESNO | MB_ICONQUESTION ) != IDYES )
	{
		SetFocus( parent );

		return;
	}

	AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );
	PlaylistInfo info( _guid );

	wchar_t gs[ MAX_PATH + 1 ] = { 0 }, gs2[ MAX_PATH ] = { 0 };
	if ( PathIsFileSpecW( info.GetFilename() ) )
		PathCombineW( gs, g_path, info.GetFilename() );
	else
		lstrcpynW( gs, info.GetFilename(), MAX_PATH );

	wchar_t l_node[ MAX_PATH ];
	wchar_t l_dir[ MAX_PATH ];
	wchar_t l_fname[ MAX_PATH ];
	wchar_t l_ext[ MAX_PATH ];

	lstrcpynW( gs2, gs, MAX_PATH );
	_wsplitpath( gs2, l_node, l_dir, l_fname, l_ext );
	_wmakepath( gs2, l_node, l_dir, L"", L"" );

	AGAVE_API_PLAYLISTS->RemovePlaylist( info.GetIndex() );
	// changed in 5.58 to resolve the issue reported at
	// http://forums.winamp.com/showthread.php?l_plugin_message=2652001#post2652001
	// delete the file after the removal and not before which
	// fixes issues if removing the currently viewed playlist
	//DeleteFileW(gs);

	// changed in 5.64 to use SHFileOperation(..) instead of DeleteFile(..)
	// so we're able to recover external playlists incase people messup...
	SHFILEOPSTRUCTW fileOp = { 0 };
	fileOp.hwnd   = parent;
	fileOp.wFunc  = FO_DELETE;
	fileOp.pFrom  = gs;
	fileOp.fFlags = ( lstrcmpi( g_path, gs2 ) ? FOF_ALLOWUNDO : 0 ) | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_SIMPLEPROGRESS | FOF_NORECURSION | FOF_NOERRORUI | FOF_SILENT;

	SHFileOperationW( &fileOp );
	SetFocus( parent );
}

static void playlists_Delete( HWND parent )
{
	if ( !m_playlistslist.GetSelectedCount() || m_playlistslist.GetSelectionMark() == -1 )
		return;

	wchar_t titleStr[ 32 ] = { 0 };
	if ( MessageBox( parent, WASABI_API_LNGSTRINGW( IDS_CONFIRM_DELETION ), WASABI_API_LNGSTRINGW_BUF( IDS_CONFIRMATION, titleStr, 32 ), MB_YESNO | MB_ICONQUESTION ) != IDYES )
		return;

	MLNavCtrl_BeginUpdate( plugin.hwndLibraryParent, 0 );
	for ( int i = playlistGUIDs.size() - 1; i >= 0; i-- )
	{
		if ( !m_playlistslist.GetSelected( i ) )
			continue;

		DeletePlaylist( playlistGUIDs[ i ], parent, false );
	}

	AGAVE_API_PLAYLISTS->Flush(); // REVIEW: save immediately? or only at the end?
	MLNavCtrl_EndUpdate( plugin.hwndLibraryParent );
}

static void playlists_Play( int enqueue )
{
	int deleted = 0;
	for ( size_t i = 0; i < playlistGUIDs.size(); i++ )
	{
		if ( !m_playlistslist.GetSelected( i ) )
			continue;

		AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

		PlaylistInfo info( playlistGUIDs[ i ] );

		wchar_t str[ MAX_PATH ] = { 0 };
		const wchar_t *fn;

		if ( PathIsFileSpecW( info.GetFilename() ) )
		{
			PathCombineW( str, g_path, info.GetFilename() );
			fn = str;
		}
		else
		{
			fn = info.GetFilename();
		}

		if ( !enqueue && !deleted )
		{
			SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE );
			deleted = 1;
		}

		enqueueFileWithMetaStructW s = { 0 };
		s.filename = fn;
		s.ext      = NULL;
		s.length   = -1;

		SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&s, IPC_PLAYFILEW );
	}

	if ( !enqueue )
		SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_STARTPLAY );
}

static void playlists_ManageButtons( HWND hwndDlg )
{
	int has_selection = m_playlistslist.GetSelectedCount();

	const int buttonids[] = { IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM, IDC_VIEWLIST, IDC_SAVE };
	for ( size_t i = 0; i != sizeof( buttonids ) / sizeof( buttonids[ 0 ] ); i++ )
	{
		HWND controlHWND = GetDlgItem( hwndDlg, buttonids[ i ] );
		EnableWindow( controlHWND, has_selection );
	}
}

static void playlists_ViewList()
{
	int t = m_playlistslist.GetSelectionMark();
	if ( t >= 0 )
	{
		AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

		PlaylistInfo info( playlistGUIDs[ t ] );
		//if ( info.treeId == 0 ) // not created yet
		//{
		//	// TODO: make a treeid for it
		//}
		mediaLibrary.SelectTreeItem( info.treeId );
	}
}

static void playlists_Paint( HWND hwndDlg )
{
	int tab[] = { IDC_PLAYLIST_LIST | DCW_SUNKENBORDER, };
	dialogSkinner.Draw( hwndDlg, tab, 1 );
}

LRESULT playlists_cloud_listview( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_NOTIFY )
	{
		LPNMHDR l = (LPNMHDR)lParam;
		switch ( l->code )
		{
			case TTN_SHOW:
			{
				LVHITTESTINFO lvh = { 0 };
				GetCursorPos( &lvh.pt );
				ScreenToClient( hwnd, &lvh.pt );
				ListView_SubItemHitTest( hwnd, &lvh );

				if ( cloud_avail && lvh.iItem != -1 && lvh.iSubItem == 1 )
				{
					LPTOOLTIPTEXTW tt = (LPTOOLTIPTEXTW)lParam;
					RECT r = { 0 };
					if ( lvh.iSubItem )
						ListView_GetSubItemRect( hwnd, lvh.iItem, lvh.iSubItem, LVIR_BOUNDS, &r );
					else
					{
						ListView_GetItemRect( hwnd, lvh.iItem, &r, LVIR_BOUNDS );
						r.right = r.left + ListView_GetColumnWidth( hwnd, 1 );
					}

					MapWindowPoints( hwnd, HWND_DESKTOP, (LPPOINT)&r, 2 );
					SetWindowPos( tt->hdr.hwndFrom, HWND_TOPMOST, r.right, r.top + 2, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE );

					return 1;
				}
			}
			break;

			case TTN_NEEDTEXTW:
			{
				LVHITTESTINFO lvh = { 0 };
				GetCursorPos( &lvh.pt );
				ScreenToClient( hwnd, &lvh.pt );
				ListView_SubItemHitTest( hwnd, &lvh );

				static wchar_t tt_buf1[ 256 ] = { L"" };
				if ( cloud_avail && lvh.iItem != -1 && lvh.iSubItem == 1 )
				{
					LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO)lParam;

					if ( last_item1 == lvh.iItem )
					{
						lpnmtdi->lpszText = tt_buf1;

						return 0;
					}

					if ( lvh.iItem < 0 || lvh.iItem >= (int)playlistGUIDs.size() )
						return 0;

					AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

					PlaylistInfo info( playlistGUIDs[ lvh.iItem ] );
					if ( info.Valid() )
					{
						WASABI_API_LNGSTRINGW_BUF( ( !info.GetCloud() ? IDS_UPLOAD_TO_CLOUD : IDS_AVAILABLE_IN_CLOUD ), tt_buf1, ARRAYSIZE( tt_buf1 ) );
					}
					else
					{
						WASABI_API_LNGSTRINGW_BUF( IDS_UPLOAD_TO_CLOUD, tt_buf1, ARRAYSIZE( tt_buf1 ) );
					}

					last_item1        = lvh.iItem;
					lpnmtdi->lpszText = tt_buf1;

					// bit of a fiddle but it allows for multi-line tooltips
					//SendMessage(l->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 0);
				}
				else
					return CallWindowProcW( (WNDPROC)GetPropW( hwnd, L"cloud_list_proc" ), hwnd, uMsg, wParam, lParam );
			}

			return 0;
		}
	}

	return CallWindowProcW( (WNDPROC)GetPropW( hwnd, L"cloud_list_proc" ), hwnd, uMsg, wParam, lParam );
}

static void playlists_InitDialog( HWND hwndDlg )
{
	HACCEL accel = WASABI_API_LOADACCELERATORSW( IDR_VIEW_PLS_ACCELERATORS );
	if ( accel )
		WASABI_API_APP->app_addAccelerators( hwndDlg, &accel, 1, TRANSLATE_MODE_CHILD );

	if ( !view.play )
		SENDMLIPC( plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view );

	opened      = true;
	loaded      = false;

	cloud_avail = playlists_CloudAvailable();
	groupBtn    = g_config->ReadInt( L"groupbtn", 1 );
	enqueuedef  = ( g_config->ReadInt( L"enqueuedef", 0 ) == 1 );

	// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
	//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
	pluginMessage l_plugin_message = { ML_MSG_VIEW_BUTTON_HOOK, (INT_PTR)hwndDlg, (INT_PTR)MAKELONG( IDC_CUSTOM, IDC_ENQUEUE ), (INT_PTR)L"ml_playlists_root" };
	wchar_t *pszTextW = (wchar_t *)SENDMLIPC( plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&l_plugin_message );
	if ( pszTextW && pszTextW[ 0 ] != 0 )
	{
		// set this to be a bit different so we can just use one button and not the
		// mixable one as well (leaving that to prevent messing with the resources)
		customAllowed = TRUE;
		SetDlgItemTextW( hwndDlg, IDC_CUSTOM, pszTextW );
	}
	else
		customAllowed = FALSE;

	/* skin dialog */
	MLSKINWINDOW skinWindow = { 0 };
	skinWindow.skinType   = SKINNEDWND_TYPE_DIALOG;
	skinWindow.style      = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	skinWindow.hwndToSkin = hwndDlg;

	MLSkinWindow( plugin.hwndLibraryParent, &skinWindow );

	/* skin listview */
	HWND hwndList = skinWindow.hwndToSkin = GetDlgItem( hwndDlg, IDC_PLAYLIST_LIST );
	skinWindow.skinType = SKINNEDWND_TYPE_LISTVIEW;
	skinWindow.style    = SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS | SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;

	MLSkinWindow( plugin.hwndLibraryParent, &skinWindow );
	MLSkinnedScrollWnd_ShowHorzBar( hwndList, FALSE );

	/* skin buttons */
	skinWindow.skinType = SKINNEDWND_TYPE_BUTTON;
	skinWindow.style    = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | ( groupBtn ? SWBS_SPLITBUTTON : 0 );

	const int buttonidz[] = { IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM };
	for ( size_t i = 0; i != sizeof( buttonidz ) / sizeof( buttonidz[ 0 ] ); i++ )
	{
		skinWindow.hwndToSkin = GetDlgItem( hwndDlg, buttonidz[ i ] );
		if ( IsWindow( skinWindow.hwndToSkin ) )
			MLSkinWindow( plugin.hwndLibraryParent, &skinWindow );
	}

	/* skin buttons */
	skinWindow.style    = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;

	const int buttonids[] = { IDC_VIEWLIST, IDC_CREATENEWPL, IDC_SAVE };
	for ( size_t i = 0; i != sizeof( buttonids ) / sizeof( buttonids[ 0 ] ); i++ )
	{
		skinWindow.hwndToSkin = GetDlgItem( hwndDlg, buttonids[ i ] );
		if ( IsWindow( skinWindow.hwndToSkin ) )
			MLSkinWindow( plugin.hwndLibraryParent, &skinWindow );
	}

	/* skin dropdown buttons */
	skinWindow.style      = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWBS_DROPDOWNBUTTON;
	skinWindow.hwndToSkin = GetDlgItem( hwndDlg, IDC_IMPORT );

	MLSkinWindow( plugin.hwndLibraryParent, &skinWindow );


	HIMAGELIST imageList = ImageList_Create( 15, 15, ILC_COLOR24, 3, 0 );
	if ( imageList != NULL )
	{
		HIMAGELIST prevList = (HIMAGELIST)SNDMSG( hwndList, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)imageList );
		if ( prevList != NULL )
			ImageList_Destroy( prevList );
	}

	m_playlistslist.setwnd( hwndList );
	m_playlistslist.AddCol( WASABI_API_LNGSTRINGW( IDS_PLAYLIST_TITLE ), 400 );

	int width = 27;
	MLCloudColumn_GetWidth( plugin.hwndLibraryParent, &width );
	m_playlistslist.AddCol( L"", ( cloud_avail ? width : 0 ) );

	m_playlistslist.AddCol( WASABI_API_LNGSTRINGW( IDS_ITEMS ), 50 );
	m_playlistslist.AutoColumnWidth( 3 );
	m_playlistslist.JustifyColumn( 3, LVCFMT_RIGHT );
	m_playlistslist.AddCol( WASABI_API_LNGSTRINGW( IDS_TIME ), 75 );
	m_playlistslist.AutoSizeColumn( 4 );
	m_playlistslist.JustifyColumn( 4, LVCFMT_RIGHT );

	MLSkinnedHeader_SetCloudColumn( ListView_GetHeader( hwndList ), ( cloud_avail ? 1 : -1 ) );

	if ( !GetPropW( hwndList, L"cloud_list_proc" ) )
		SetPropW( hwndList, L"cloud_list_proc", (HANDLE)SetWindowLongPtrW( hwndList, GWLP_WNDPROC, (LONG_PTR)playlists_cloud_listview ) );

	playlist_UpdateButtonText( hwndDlg, enqueuedef == 1 );
	playlists_ManageButtons( hwndDlg );
	RefreshPlaylistsList();

	SetWindowRedraw( m_playlistslist.getwnd(), FALSE );
}

void playlists_Destroy( HWND hwndDlg )
{
	opened = false;
	WASABI_API_APP->app_removeAccelerators( hwndDlg );
	m_playlistslist.setwnd( NULL );
	playlistGUIDs.clear();
}

BOOL playlists_GetDisplayInfo( NMLVDISPINFO *lpdi )
{
	size_t item = lpdi->item.iItem;

	if ( item < 0 || item >= playlistGUIDs.size() )
		return 0;

	AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

	PlaylistInfo info( playlistGUIDs[ item ] );
	if ( lpdi->item.mask & LVIF_TEXT && info.Valid() )
	{
		switch ( lpdi->item.iSubItem )
		{
			case 0:
			{
				// TODO: this is going to be slow, we should investigate caching the title
				lstrcpyn( lpdi->item.pszText, info.GetName(), lpdi->item.cchTextMax );
				break;
			}
			case 1:
			{
				StringCchPrintf( lpdi->item.pszText, lpdi->item.cchTextMax, L"%d", info.GetCloud() );
				break;
			}
			case 2:
			{
				StringCchPrintf( lpdi->item.pszText, lpdi->item.cchTextMax, L"%d", info.GetSize() );
				break;
			}
			case 3:
			{
				wchar_t str[ 64 ] = { 0 };
				FormatLength( str, info.GetLength(), 64 );
				lstrcpyn( lpdi->item.pszText, str, lpdi->item.cchTextMax );
			}
			break;
		}
	}
	return 0;
}

void Playlists_RenameSelected( HWND hwndDlg )
{
	// TOOD: loop through selections
	int s = m_playlistslist.GetSelectionMark();
	if ( s != -1 )
		RenamePlaylist( playlistGUIDs[ s ], hwndDlg );
}

BOOL playlists_OnCustomDraw( HWND hwndDlg, NMLVCUSTOMDRAW *plvcd, LRESULT *pResult )
{
	static BOOL bDrawFocus;
	static RECT rcView;
	static CLOUDCOLUMNPAINT cloudColumnPaint;

	*pResult = CDRF_DODEFAULT;

	switch ( plvcd->nmcd.dwDrawStage )
	{
		case CDDS_PREPAINT:
			*pResult |= CDRF_NOTIFYITEMDRAW;
			CopyRect( &rcView, &plvcd->nmcd.rc );

			cloudColumnPaint.hwndList = plvcd->nmcd.hdr.hwndFrom;
			cloudColumnPaint.hdc      = plvcd->nmcd.hdc;
			cloudColumnPaint.prcView  = &rcView;
			return TRUE;

		case CDDS_ITEMPREPAINT:
			*pResult |= CDRF_NOTIFYSUBITEMDRAW;
			bDrawFocus = ( CDIS_FOCUS & plvcd->nmcd.uItemState );
			if ( bDrawFocus )
			{
				plvcd->nmcd.uItemState &= ~CDIS_FOCUS;
				*pResult |= CDRF_NOTIFYPOSTPAINT;
			}
			return TRUE;

		case CDDS_ITEMPOSTPAINT:
			if ( bDrawFocus )
			{
				RECT rc;
				rc.left = LVIR_BOUNDS;
				SendMessageW( plvcd->nmcd.hdr.hwndFrom, LVM_GETITEMRECT, plvcd->nmcd.dwItemSpec, (LPARAM)&rc );

				rc.left += 3;
				DrawFocusRect( plvcd->nmcd.hdc, &rc );

				plvcd->nmcd.uItemState |= CDIS_FOCUS;

				bDrawFocus = FALSE;
			}

			*pResult = CDRF_SKIPDEFAULT;
			return TRUE;

		case( CDDS_SUBITEM | CDDS_ITEMPREPAINT ):
			// TODO need to have a map between column ids so we do this correctly
			if ( plvcd->iSubItem == 1 )
			{
				if ( 0 == plvcd->iSubItem && 0 == plvcd->nmcd.rc.right || playlistGUIDs.empty() )
					break;

				cloudColumnPaint.iItem    = plvcd->nmcd.dwItemSpec;
				cloudColumnPaint.iSubItem = plvcd->iSubItem;

				int cloud_icon = 0;
				size_t item = plvcd->nmcd.dwItemSpec;
				if ( item >= 0 || item < playlistGUIDs.size() )
				{
					AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

					PlaylistInfo info( playlistGUIDs[ item ] );
					if ( info.Valid() )
						cloud_icon = info.GetCloud();
				}

				// TODO have this show an appropriate cloud icon for the playlist
				//		currently all we have is cloud or nothing as we'll only
				//		have files locally for this for the moment (need todo!!!)
				cloudColumnPaint.value   = cloud_icon;
				cloudColumnPaint.prcItem = &plvcd->nmcd.rc;
				cloudColumnPaint.rgbBk   = plvcd->clrTextBk;
				cloudColumnPaint.rgbFg   = plvcd->clrText;

				if ( MLCloudColumn_Paint( plugin.hwndLibraryParent, &cloudColumnPaint ) )
				{
					*pResult = CDRF_SKIPDEFAULT;

					return TRUE;
				}
			}
			break;
	}

	return FALSE;
}

BOOL playlists_Notify( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	LPNMHDR l = (LPNMHDR)lParam;
	if ( l->idFrom == IDC_PLAYLIST_LIST )
	{
		switch ( l->code )
		{
			case LVN_ITEMCHANGED:
				playlists_ManageButtons( hwndDlg );
				break;
			case LVN_BEGINDRAG:
				root_is_drag_and_dropping = 1; SetCapture( hwndDlg );
				break;
			case LVN_GETDISPINFO:
				return playlists_GetDisplayInfo( (NMLVDISPINFO *)lParam );
			case NM_DBLCLK:
				playlists_Play( enqueuedef == 1 );
				break;
			case NM_CLICK:
			{
				LPNMITEMACTIVATE pnmitem = (LPNMITEMACTIVATE)lParam;
				if ( cloud_avail && pnmitem->iItem != -1 && pnmitem->iSubItem == 1 )
				{
					RECT itemRect = { 0 };
					if ( pnmitem->iSubItem )
						ListView_GetSubItemRect( pnmitem->hdr.hwndFrom, pnmitem->iItem, pnmitem->iSubItem, LVIR_BOUNDS, &itemRect );
					else
					{
						ListView_GetItemRect( pnmitem->hdr.hwndFrom, pnmitem->iItem, &itemRect, LVIR_BOUNDS );
						itemRect.right = itemRect.left + ListView_GetColumnWidth( pnmitem->hdr.hwndFrom, pnmitem->iSubItem );
					}

					MapWindowPoints( pnmitem->hdr.hwndFrom, HWND_DESKTOP, (POINT *)&itemRect, 2 );

					size_t item = pnmitem->iItem;
					if ( item < 0 || item >= playlistGUIDs.size() )
						return 0;

					AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

					PlaylistInfo info( playlistGUIDs[ item ] );

					HMENU cloud_menu = (HMENU)0x666;
					ReferenceCountedNXString uid;
					NXStringCreateWithFormatting( &uid, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
												  (int)info.playlist_guid.Data1, (int)info.playlist_guid.Data2,
												  (int)info.playlist_guid.Data3, (int)info.playlist_guid.Data4[ 0 ],
												  (int)info.playlist_guid.Data4[ 1 ], (int)info.playlist_guid.Data4[ 2 ],
												  (int)info.playlist_guid.Data4[ 3 ], (int)info.playlist_guid.Data4[ 4 ],
												  (int)info.playlist_guid.Data4[ 5 ], (int)info.playlist_guid.Data4[ 6 ],
												  (int)info.playlist_guid.Data4[ 7 ] );

					WASABI_API_SYSCB->syscb_issueCallback( api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_GET_CLOUD_STATUS, (intptr_t)uid->string, (intptr_t)&cloud_menu );
					if ( cloud_menu )
					{
						int r = Menu_TrackPopup( plugin.hwndLibraryParent, cloud_menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NONOTIFY, itemRect.right, itemRect.top, pnmitem->hdr.hwndFrom, NULL );
						if ( r >= CLOUD_SOURCE_MENUS && r < CLOUD_SOURCE_MENUS_PL_UPPER )	// deals with cloud specific menus
						{
							// 0 = no change
							// 1 = adding to cloud
							// 2 = added locally
							// 4 = removed
							int mode = -(int)info.GetIndex();
							WASABI_API_SYSCB->syscb_issueCallback( api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_PROCESS_CLOUD_STATUS, (intptr_t)r, (intptr_t)&mode );
							if ( mode > 0 )
							{
								info.SetCloud( ( mode == 1 ? 1 : 0 ) );
								AGAVE_API_PLAYLISTS->Flush();

								UpdatePlaylists();

								last_item1 = -1;
							}
						}

						DestroyMenu( cloud_menu );
					}
				}
			}
			break;
			case LVN_KEYDOWN:
			{
				LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)lParam;
				switch ( pnkd->wVKey )
				{
					case 0x2E:         //Delete
						playlists_Delete( hwndDlg );
						break;
					case VK_F2:
						Playlists_RenameSelected( hwndDlg );
						SendMessage( hwndDlg, WM_NEXTDLGCTL, (WPARAM)l->hwndFrom, (LPARAM)TRUE );
						break;
					case 'A':
						if ( !( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) && ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
							m_playlistslist.SelectAll();
						break;
				}
			}
			break;

			case NM_CUSTOMDRAW:
			{
				LRESULT result = 0;
				if ( cloud_avail && playlists_OnCustomDraw( hwndDlg, (NMLVCUSTOMDRAW *)lParam, &result ) )
				{
					SetWindowLongPtrW( hwndDlg, DWLP_MSGRESULT, (LONG_PTR)result );

					return 1;
				}
				break;
			}
		}
	}

	switch ( l->code )
	{
		case HDN_ITEMCHANGING:
		{
			LPNMHEADERW phdr = (LPNMHEADERW)lParam;
			if ( phdr->pitem && ( HDI_WIDTH & phdr->pitem->mask ) && phdr->iItem == 1 )
			{
				if ( !cloud_avail )
					phdr->pitem->cxy = 0;
				else
				{
					INT width = phdr->pitem->cxy;
					if ( MLCloudColumn_GetWidth( plugin.hwndLibraryParent, &width ) )
						phdr->pitem->cxy = width;
				}
			}
			break;
		}
	}

	return 0;
}

void playlists_MouseMove( HWND hwndDlg, LPARAM lParam )
{
	if ( root_is_drag_and_dropping && GetCapture() == hwndDlg )
	{
		POINT p = { GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) };
		ClientToScreen( hwndDlg, &p );

		mlDropItemStruct m = { 0 };
		m.type = ML_TYPE_FILENAMES;
		m.p    = p;

		pluginHandleIpcMessage( ML_IPC_HANDLEDRAG, (WPARAM)&m );
	}
}

void playlists_LeftButtonUp( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	if ( root_is_drag_and_dropping && GetCapture() == hwndDlg )
	{
		ReleaseCapture();

		POINT p = { GET_X_LPARAM( lParam ),GET_Y_LPARAM( lParam ) };
		ClientToScreen( hwndDlg, &p );

		mlDropItemStruct m = { 0 };
		m.type = ML_TYPE_FILENAMES;
		m.p    = p;

		pluginHandleIpcMessage( ML_IPC_HANDLEDRAG, (WPARAM)&m );

		if ( m.result > 0 )
		{
			//std::vector<char> data;
			std::string data;

			for ( size_t i = 0; i < playlistGUIDs.size(); i++ )
			{
				if ( !m_playlistslist.GetSelected( i ) )
					continue;

				AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

				PlaylistInfo info( playlistGUIDs[ i ] );

				wchar_t str[ MAX_PATH ] = { 0 };
				if ( PathIsFileSpecW( info.GetFilename() ) )
					PathCombineW( str, g_path, info.GetFilename() );
				else
					lstrcpynW( str, info.GetFilename(), MAX_PATH );

				AutoChar charStr( str );
				// HAKAN: why (len + 1) ?
				//data.append(charStr, lstrlenA( charStr + 1));
				data.append(charStr, lstrlenA(charStr));
			}
			// HAKAN: No need to add trailing zero
			//data.push_back( 0 );

			m.flags  = 0;
			m.result = 0;
			m.data   = (void *)data.c_str();

			pluginHandleIpcMessage( ML_IPC_HANDLEDROP, (WPARAM)&m );
			RefreshPlaylistsList();
		}

		root_is_drag_and_dropping = 0;
	}
}

enum
{
	BPM_ECHO_WM_COMMAND = 0x1, // send WM_COMMAND and return value
	BPM_WM_COMMAND      = 0x2, // just send WM_COMMAND
};

static void playlists_PlayEnqueue( HWND hwndDlg, HWND from, UINT idFrom )
{
	HMENU listMenu = GetSubMenu( g_context_menus3, 0 );
	int   count    = GetMenuItemCount( listMenu );
	if ( count > 2 )
	{
		for ( int i = 2; i < count; i++ )
			DeleteMenu( listMenu, 2, MF_BYPOSITION );
	}

	UINT menuStatus;
	if ( m_playlistslist.GetNextSelected( -1 ) == -1 )
		menuStatus = MF_BYCOMMAND | MF_GRAYED;
	else
		menuStatus = MF_BYCOMMAND | MF_ENABLED;

	EnableMenuItem( listMenu, IDC_PLAYLIST_INVERT_SELECTION, menuStatus );

	if ( m_playlistslist.GetCount() > 0 )
		menuStatus = MF_BYCOMMAND | MF_ENABLED;
	else
		menuStatus = MF_BYCOMMAND | MF_GRAYED;

	EnableMenuItem( listMenu, IDC_PLAYLIST_SELECT_ALL, menuStatus );
	playlist_ButtonPopupMenu( hwndDlg, idFrom, listMenu, BPM_WM_COMMAND );
}

void playlists_Command( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	switch ( LOWORD( wParam ) )
	{
		case IDC_VIEWLIST:
			playlists_ViewList();
			break;
		case IDC_IMPORT:
			playlists_Import( hwndDlg, lParam );
			break;
		case IDC_CREATENEWPL:
		case IDC_NEWPLAYLIST:
			playlists_Add( hwndDlg );
			break;
		case IDC_PLAY:
		case IDC_ENQUEUE:
		case IDC_CUSTOM:
		{
			if ( HIWORD( wParam ) == MLBN_DROPDOWN )
			{
				playlists_PlayEnqueue( hwndDlg, (HWND)lParam, LOWORD( wParam ) );
			}
			else
			{
				int action;
				if ( LOWORD( wParam ) == IDC_PLAY )
					action = ( HIWORD( wParam ) == 1 ) ? enqueuedef == 1 : 0;
				else if ( LOWORD( wParam ) == IDC_ENQUEUE )
					action = ( HIWORD( wParam ) == 1 ) ? ( enqueuedef != 1 ) : 1;
				else
					// so custom can work with the menu item part
					break;

				playlists_Play( action );
			}
			break;
		}
		case IDC_SAVE:
			playlists_Save( hwndDlg );
			break;
		case IDC_DELETE:
			playlists_Delete( hwndDlg );
			break;
		case IDC_RENAME:
			Playlists_RenameSelected( hwndDlg );
			break;
	}
}

void playlists_DropFiles( HDROP hDrop )
{
	wchar_t l_playlist_filename[ 2048 ] = { 0 };
	int y = DragQueryFileW( hDrop, 0xffffffff, l_playlist_filename, 2048 );

	for ( int x = 0; x < y; x++ )
	{
		Playlist currentPlaylist2;
		DragQueryFileW( hDrop, x, l_playlist_filename, 2048 );
		// make sure that we only add valid playlists and not normal files
		if ( AGAVE_API_PLAYLISTMANAGER->CanLoad( l_playlist_filename ) )
		{
			ImportPlaylist( l_playlist_filename );
		}
	}
}

void playlists_Sort( size_t sort_type )
{
	int  cur_sel  = mediaLibrary.GetSelectedTreeItem();
	GUID cur_guid = tree_to_guid_map[ cur_sel ];

	// keep the old tree ids before sorting so we can then re-map
	// without having to remove and re-add all of the tree items
	std::vector<int> tree_ids;

	AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

	size_t count = AGAVE_API_PLAYLISTS->GetCount();
	for ( size_t i = 0; i != count; i++ )
	{
		PlaylistInfo info( i );
		if ( info.Valid() )
			tree_ids.push_back( info.treeId );
	}

	if ( AGAVE_API_PLAYLISTS->Sort( sort_type ) )
	{
		for ( size_t i = 0; i != count; i++ )
		{
			PlaylistInfo info( i );
			UpdateTree( info, tree_ids[ i ] );
		}

		for ( size_t i = 0; i != count; i++ )
		{
			PlaylistInfo info( i );
			if ( cur_guid == info.playlist_guid )
			{
				mediaLibrary.SelectTreeItem( info.treeId );
			}
		}

		RefreshPlaylistsList();
	}
}

void playlists_ContextMenu( HWND hwndDlg, HWND from, int x, int y )
{
	if ( from != m_playlistslist.getwnd() )
		return;

	POINT pt = { x,y };

	if ( x == -1 || y == -1 ) // x and y are -1 if the user invoked a shift-f10 popup menu
	{
		RECT channelRect = { 0 };
		int selected = m_playlistslist.GetNextSelected();
		if ( selected != -1 ) // if something is selected we'll drop the menu from there
		{
			m_playlistslist.GetItemRect( selected, &channelRect );
			ClientToScreen( hwndDlg, (POINT *)&channelRect );
		}
		else // otherwise we'll drop it from the top-left corner of the listview, adjusting for the header location
		{
			GetWindowRect( hwndDlg, &channelRect );

			HWND hHeader = (HWND)SNDMSG( from, LVM_GETHEADER, 0, 0L );
			RECT headerRect;
			if ( ( WS_VISIBLE & GetWindowLongPtr( hHeader, GWL_STYLE ) ) && GetWindowRect( hHeader, &headerRect ) )
			{
				channelRect.top += ( headerRect.bottom - headerRect.top );
			}
		}

		x = channelRect.left;
		y = channelRect.top;
	}

	HWND hHeader = (HWND)SNDMSG( from, LVM_GETHEADER, 0, 0L );
	RECT headerRect;
	if ( 0 == ( WS_VISIBLE & GetWindowLongPtr( hHeader, GWL_STYLE ) ) || FALSE == GetWindowRect( hHeader, &headerRect ) )
		SetRectEmpty( &headerRect );

	if ( FALSE != PtInRect( &headerRect, pt ) )
		return;

	HMENU menu = GetSubMenu( g_context_menus, 0 );
	sendTo.AddHere( hwndDlg, GetSubMenu( menu, 2 ), ML_TYPE_FILENAMES, 1, ( ML_TYPE_PLAYLIST + 1 ) );

	HMENU cloud_hmenu = (HMENU)0x666;
	size_t index = 0, i = 0;
	if ( playlists_CloudAvailable() )
	{
		AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );
		for ( ; i < playlistGUIDs.size(); i++ )
		{
			if ( !m_playlistslist.GetSelected( i ) )
				continue;

			PlaylistInfo info( playlistGUIDs[ i ] );

			ReferenceCountedNXString uid;
			NXStringCreateWithFormatting( &uid, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
										  (int)info.playlist_guid.Data1, (int)info.playlist_guid.Data2,
										  (int)info.playlist_guid.Data3, (int)info.playlist_guid.Data4[ 0 ],
										  (int)info.playlist_guid.Data4[ 1 ], (int)info.playlist_guid.Data4[ 2 ],
										  (int)info.playlist_guid.Data4[ 3 ], (int)info.playlist_guid.Data4[ 4 ],
										  (int)info.playlist_guid.Data4[ 5 ], (int)info.playlist_guid.Data4[ 6 ],
										  (int)info.playlist_guid.Data4[ 7 ] );

			index = info.GetIndex();

			WASABI_API_SYSCB->syscb_issueCallback( api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_GET_CLOUD_STATUS, (intptr_t)uid->string, (intptr_t)&cloud_hmenu );

			if ( cloud_hmenu && cloud_hmenu != (HMENU)0x666 )
			{
				MENUITEMINFOW m = { sizeof( m ), MIIM_TYPE | MIIM_ID | MIIM_SUBMENU, MFT_SEPARATOR, 0 };
				m.wID = CLOUD_SOURCE_MENUS - 1;
				InsertMenuItemW( menu, 3, TRUE, &m );

				wchar_t a[ 100 ] = { 0 };
				m.fType      = MFT_STRING;
				m.dwTypeData = WASABI_API_LNGSTRINGW_BUF( IDS_CLOUD_SOURCES, a, 100 );
				m.wID        = CLOUD_SOURCE_MENUS;
				m.hSubMenu   = cloud_hmenu;

				InsertMenuItemW( menu, 4, TRUE, &m );
			}
			break;
		}
	}

	UpdateMenuItems( hwndDlg, menu );

	UINT menuStatus;
	if ( m_playlistslist.GetNextSelected( -1 ) == -1 )
	{
		menuStatus = MF_BYCOMMAND | MF_GRAYED;
		EnableMenuItem( menu, 2, MF_BYPOSITION | MF_GRAYED );
		EnableMenuItem( menu, CLOUD_SOURCE_MENUS, MF_BYCOMMAND | MF_GRAYED );
	}
	else
	{
		menuStatus = MF_BYCOMMAND | MF_ENABLED;
		EnableMenuItem( menu, 2, MF_BYPOSITION | MF_ENABLED );
		EnableMenuItem( menu, CLOUD_SOURCE_MENUS, MF_BYCOMMAND | MF_ENABLED );
	}

	EnableMenuItem( menu, IDC_PLAY,                 menuStatus );
	EnableMenuItem( menu, IDC_ENQUEUE,              menuStatus );
	EnableMenuItem( menu, IDC_DELETE,               menuStatus );
	EnableMenuItem( menu, ID_QUERYMENU_ADDNEWQUERY, menuStatus );
	EnableMenuItem( menu, IDC_RENAME,               menuStatus );
	EnableMenuItem( menu, IDC_ENQUEUE,              menuStatus );
	EnableMenuItem( menu, IDC_VIEWLIST,             menuStatus );

	int r = Menu_TrackPopup( plugin.hwndLibraryParent, menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, x, y, hwndDlg, NULL );
	switch ( r )
	{
		case IDC_VIEWLIST:
			playlists_ViewList();
			break;
		case IDC_NEWPLAYLIST:
			playlists_Add( hwndDlg );
			break;
		case IDC_PLAY:
			playlists_Play( 0 );
			break;
		case IDC_ENQUEUE:
			playlists_Play( 1 );
			break;
		case IDC_DELETE:
			playlists_Delete( hwndDlg );
			SendMessage( hwndDlg, WM_NEXTDLGCTL, (WPARAM)from, (LPARAM)TRUE );
			break;
		case ID_QUERYMENU_ADDNEWQUERY:
			playlists_Add( hwndDlg );
			SendMessage( hwndDlg, WM_NEXTDLGCTL, (WPARAM)from, (LPARAM)TRUE );
			break;
		case IDC_RENAME:
			Playlists_RenameSelected( hwndDlg );
			SendMessage( hwndDlg, WM_NEXTDLGCTL, (WPARAM)from, (LPARAM)TRUE );
			break;
		default:
			if ( sendTo.WasClicked( r ) )
			{
				bool playlist_type_worked = true;
				int numPlaylists = m_playlistslist.GetSelectedCount();
				if ( !numPlaylists )
					break;

				mlPlaylist **playlists = new mlPlaylist * [ numPlaylists + 1 ];
				playlists[ numPlaylists ] = 0; // null terminate

				// TODO: m_playlistslist.GetNextSelected()
				for ( int i = 0, pl = 0; i < m_playlistslist.GetCount(); i++ )
				{
					if ( !m_playlistslist.GetSelected( i ) )
						continue;

					playlists[ pl ] = new mlPlaylist;
					memset( playlists[ pl ], 0, sizeof( mlPlaylist ) );

					{ // scope for lock
						AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

						PlaylistInfo info( playlistGUIDs[ i ] );
						playlists[ pl ]->filename = _wcsdup( info.GetFilename() );
						playlists[ pl ]->length   = info.GetLength();
						playlists[ pl ]->numItems = info.GetSize();
						playlists[ pl ]->title    = _wcsdup( info.GetName() );
					}

					pl++;
				}

				if ( sendTo.SendPlaylists( playlists ) != 1 )
				{
					for ( int i = 0; i < numPlaylists; i++ )
					{
						if ( sendTo.SendPlaylist( playlists[ i ] ) != 1 )
						{
							playlist_type_worked = false;
							break;
						}
					}
				}

				for ( int i = 0; i < numPlaylists; i++ )
				{
					free( (void *)playlists[ i ]->filename );
					free( (void *)playlists[ i ]->title );

					delete playlists[ i ];
				}

				delete[] playlists;


				if ( !playlist_type_worked )
				{
					//std::vector<wchar_t> data;
					std::wstring data;

					{ // scope for lock
						AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

						for ( size_t ipl = 0; ipl < playlistGUIDs.size(); ipl++ )
						{
							if ( !m_playlistslist.GetSelected( ipl ) )
								continue;
							PlaylistInfo info( playlistGUIDs[ ipl ] );

							wchar_t str[ MAX_PATH ] = { 0 };

							if ( PathIsFileSpecW( info.GetFilename() ) )
								PathCombineW( str, g_path, info.GetFilename() );
							else
								lstrcpynW( str, info.GetFilename(), MAX_PATH );

							// HAKAN: why (len + 1) ?
							//data.append( str, lstrlen(str) + 1);
							data.append(str, lstrlen(str));
						}
					}

					// HAKAN: No need to add trailing zero
					//data.push_back( 0 );
					// 
					// build my data.
					sendTo.SendFilenames( data.c_str() );
				}
			}
			else
			{
				if ( r >= CLOUD_SOURCE_MENUS && r < CLOUD_SOURCE_MENUS_PL_UPPER )	// deals with cloud specific menus
				{
					// 0 = no change
					// 1 = adding to cloud
					// 2 = added locally
					// 4 = removed
					int mode = -(int)index;
					WASABI_API_SYSCB->syscb_issueCallback( api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_PROCESS_CLOUD_STATUS, (intptr_t)r, (intptr_t)&mode );
					if ( mode > 0 )
					{
						PlaylistInfo info( playlistGUIDs[ i ] );
						info.SetCloud( ( mode == 1 ? 1 : 0 ) );

						AGAVE_API_PLAYLISTS->Flush();

						UpdatePlaylists();

						last_item1 = -1;
					}
				}
			}
			break;
	}

	sendTo.Cleanup();

	if ( cloud_hmenu && cloud_hmenu != (HMENU)0x666 )
	{
		DeleteMenu( menu, CLOUD_SOURCE_MENUS - 1, MF_BYCOMMAND );
		DeleteMenu( menu, CLOUD_SOURCE_MENUS, MF_BYCOMMAND );
		DestroyMenu( cloud_hmenu );
	}
}

static HRGN g_rgnUpdate = NULL;
static int  offsetX     = 0;
static int  offsetY     = 0;

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

static void LayoutWindows( HWND hwnd, BOOL fRedraw, BOOL fUpdateAll = FALSE )
{
	static INT controls[] =
	{
		GROUP_STATUSBAR, IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM, IDC_VIEWLIST, IDC_CREATENEWPL, IDC_IMPORT, IDC_SAVE,
		GROUP_MAIN, IDC_PLAYLIST_LIST
	};

	INT    index;
	RECT   rc, rg, ri;
	LAYOUT layout[ sizeof( controls ) / sizeof( controls[ 0 ] ) ], *pl;
	BOOL   skipgroup;
	HRGN   rgn = NULL;

	GetClientRect( hwnd, &rc );
	if ( rc.right == rc.left || rc.bottom == rc.top )
		return;

	if ( rc.right > WASABI_API_APP->getScaleX( 4 ) )
		rc.right -= WASABI_API_APP->getScaleX( 4 );

	SetRect( &rg, rc.left, rc.top, rc.right, rc.top );

	pl        = layout;
	skipgroup = FALSE;

	InvalidateRect( hwnd, NULL, TRUE );

	for ( index = 0; index < sizeof( controls ) / sizeof( *controls ); index++ )
	{
		if ( controls[ index ] >= GROUP_MIN && controls[ index ] <= GROUP_MAX ) // group id
		{
			skipgroup = FALSE;
			switch ( controls[ index ] )
			{
				case GROUP_STATUSBAR:
				{
					wchar_t buffer[ 128 ] = { 0 };
					GetDlgItemTextW( hwnd, IDC_PLAY, buffer, ARRAYSIZE( buffer ) );
					LRESULT idealSize = MLSkinnedButton_GetIdealSize( GetDlgItem( hwnd, IDC_PLAY ), buffer );

					SetRect( &rg, rc.left + WASABI_API_APP->getScaleX( 1 ),
							 rc.bottom - WASABI_API_APP->getScaleY( HIWORD( idealSize ) ),
							 rc.right, rc.bottom );
					rc.bottom = rg.top - WASABI_API_APP->getScaleY( 3 );
					break;
				}
				case GROUP_MAIN:
					SetRect( &rg, rc.left + WASABI_API_APP->getScaleX( 1 ), rc.top, rc.right, rc.bottom );
					break;
			}
			continue;
		}

		if ( skipgroup )
			continue;

		pl->id   = controls[ index ];
		pl->hwnd = GetDlgItem( hwnd, pl->id );
		if ( !pl->hwnd )
			continue;

		GetWindowRect( pl->hwnd, &ri );
		MapWindowPoints( HWND_DESKTOP, hwnd, (LPPOINT)&ri, 2 );
		pl->flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS;

		switch ( pl->id )
		{
			case IDC_PLAY:
			case IDC_ENQUEUE:
			case IDC_CUSTOM:
			case IDC_VIEWLIST:
			case IDC_CREATENEWPL:
			case IDC_IMPORT:
			case IDC_SAVE:
				if ( IDC_CUSTOM != pl->id || customAllowed )
				{
					if ( groupBtn && ( pl->id == IDC_PLAY ) && ( enqueuedef == 1 ) )
					{
						pl->flags |= SWP_HIDEWINDOW;
						break;
					}

					if ( groupBtn && ( pl->id == IDC_ENQUEUE ) && ( enqueuedef != 1 ) )
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
					GetWindowText( pl->hwnd, buffer, ARRAYSIZE( buffer ) );

					LRESULT idealSize = MLSkinnedButton_GetIdealSize( pl->hwnd, buffer );
					LONG    width     = LOWORD( idealSize ) + ( pl->id != IDC_IMPORT ? WASABI_API_APP->getScaleX( 6 ) : 0 );

					SETLAYOUTPOS( pl, rg.left, rg.bottom - WASABI_API_APP->getScaleY( HIWORD( idealSize ) ), width, WASABI_API_APP->getScaleY( HIWORD( idealSize ) ) );

					pl->flags |= ( ( rg.right - rg.left ) > width ) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
					if ( SWP_SHOWWINDOW & pl->flags )
						rg.left += ( pl->cx + WASABI_API_APP->getScaleX( 4 ) );
				}
				else
					pl->flags |= SWP_HIDEWINDOW;
				break;
			case IDC_PLAYLIST_LIST:
				pl->flags |= ( rg.top < rg.bottom ) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
				SETLAYOUTPOS( pl, rg.left, rg.top + WASABI_API_APP->getScaleY( 1 ), rg.right - rg.left + WASABI_API_APP->getScaleX( 1 ), ( rg.bottom - rg.top ) - WASABI_API_APP->getScaleY( 2 ) );
				break;
		}

		SETLAYOUTFLAGS( pl, ri );
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

	if ( pl != layout )
	{
		LAYOUT *pc;
		HDWP hdwp = BeginDeferWindowPos( (INT)( pl - layout ) );
		for ( pc = layout; pc < pl && hdwp; pc++ )
			hdwp = DeferWindowPos( hdwp, pc->hwnd, NULL, pc->x, pc->y, pc->cx, pc->cy, pc->flags );

		if ( hdwp )
			EndDeferWindowPos( hdwp );

		if ( !rgn )
			rgn = CreateRectRgn( 0, 0, 0, 0 );

		for ( pc = layout; pc < pl && hdwp; pc++ )
		{
			switch ( pc->id )
			{
				case IDC_PLAYLIST_LIST:
					PostMessage( hwnd, WM_APP + 100, 0, 0 );
					break;
			}
		}

		if ( fRedraw )
		{
			GetUpdateRgn( hwnd, rgn, FALSE );
			for ( pc = layout; pc < pl && hdwp; pc++ )
			{
				if ( pc->rgn )
				{
					OffsetRgn( pc->rgn, pc->x, pc->y );
					CombineRgn( rgn, rgn, pc->rgn, RGN_OR );
				}
			}

			RedrawWindow( hwnd, NULL, rgn, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN );
		}

		if ( g_rgnUpdate )
		{
			GetUpdateRgn( hwnd, g_rgnUpdate, FALSE );
			for ( pc = layout; pc < pl && hdwp; pc++ )
			{
				if ( pc->rgn )
				{
					OffsetRgn( pc->rgn, pc->x, pc->y );
					CombineRgn( g_rgnUpdate, g_rgnUpdate, pc->rgn, RGN_OR );
				}
			}
		}

		for ( pc = layout; pc < pl && hdwp; pc++ )
			if ( pc->rgn )
				DeleteObject( pc->rgn );
	}

	if ( rgn )
		DeleteObject( rgn );

	ValidateRgn( hwnd, NULL );
}

INT_PTR CALLBACK view_playlistsDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR a = dialogSkinner.Handle( hwndDlg, uMsg, wParam, lParam );	if ( a )	return a;
	switch ( uMsg )
	{
		case WM_INITMENUPOPUP:
			sendTo.InitPopupMenu( wParam );
			return 0;
		case WM_INITDIALOG:
			playlists_InitDialog( hwndDlg );
			return TRUE;
		case WM_NOTIFY:
			return playlists_Notify( hwndDlg, wParam, lParam );
		case WM_MOUSEMOVE:
			playlists_MouseMove( hwndDlg, lParam );
			return 0;
		case WM_LBUTTONUP:
			playlists_LeftButtonUp( hwndDlg, wParam, lParam );
			return 0;
		case WM_COMMAND:
			playlists_Command( hwndDlg, wParam, lParam );
			break;
		case WM_PAINT:
			playlists_Paint( hwndDlg );
			return 0;
		case WM_ERASEBKGND:
			return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT
		case WM_DESTROY:
			playlists_Destroy( hwndDlg );
			break;
		case WM_DROPFILES:
			playlists_DropFiles( (HDROP)wParam );
			return 0;
		case WM_CONTEXTMENU:
			playlists_ContextMenu( hwndDlg, (HWND)wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
			return 0;
		case WM_DISPLAYCHANGE:
			LayoutWindows( hwndDlg, TRUE );
			return 0;
		case WM_APP + 104:
		{
			playlist_UpdateButtonText( hwndDlg, wParam );
			LayoutWindows( hwndDlg, TRUE );

			return 0;
		}
		case WM_APP + 102:
		{
			if ( cloud_avail )
			{
				int width = 27;
				MLCloudColumn_GetWidth( plugin.hwndLibraryParent, &width );
				m_playlistslist.SetColumnWidth( 1, width );
				MLSkinnedHeader_SetCloudColumn( ListView_GetHeader( m_playlistslist.getwnd() ), 1 );
			}
		}
		case WM_APP + 101:
			m_playlistslist.RefreshAll(); UpdateWindow( m_playlistslist.getwnd() );
		case WM_APP + 100:
			AutoSizePlaylistColumns();
			if ( !loaded )
			{
				loaded = true;
				SetWindowRedraw( m_playlistslist.getwnd(), TRUE );
			}
			return 0;
		case WM_WINDOWPOSCHANGED:
			if ( ( SWP_NOSIZE | SWP_NOMOVE ) != ( ( SWP_NOSIZE | SWP_NOMOVE ) & ( (WINDOWPOS *)lParam )->flags ) ||
				 ( SWP_FRAMECHANGED & ( (WINDOWPOS *)lParam )->flags ) )
			{
				LayoutWindows( hwndDlg, !( SWP_NOREDRAW & ( (WINDOWPOS *)lParam )->flags ) );
			}
			return 0;
		case WM_USER + 0x200:
			SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, 1 ); // yes, we support no - redraw resize
			return TRUE;
		case WM_USER + 0x201:
			offsetX = (short)LOWORD( wParam );
			offsetY = (short)HIWORD( wParam );
			g_rgnUpdate = (HRGN)lParam;
			return TRUE;
		case WM_ML_CHILDIPC:
		{
			if ( lParam == ML_CHILDIPC_DROPITEM && wParam )
			{
				mlDropItemStruct *dis = (mlDropItemStruct *)wParam;
				if ( dis )
				{
					switch ( dis->type )
					{
						case ML_TYPE_FILENAMES:
						case ML_TYPE_STREAMNAMES:
						case ML_TYPE_FILENAMESW:
						case ML_TYPE_STREAMNAMESW:
						case ML_TYPE_ITEMRECORDLIST:
						case ML_TYPE_ITEMRECORDLISTW:
						case ML_TYPE_CDTRACKS:
							// check we're not dropping back on ourself - not
							// pretty but it prevents the new playlist prompt
							if ( root_is_drag_and_dropping )
							{
								RECT r;
								GetWindowRect( hwndDlg, &r );
								dis->result = ( !PtInRect( &r, dis->p ) ? 1 : -1 );
							}
							// otherwise allow through as from external
							else
								dis->result = 1;
							break;
						default:
							dis->result = -1;
							break;
					}
				}

				return 0;
			}
		}
	}

	return 0;
}