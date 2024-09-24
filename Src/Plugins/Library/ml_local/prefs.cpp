#include "main.h"
#include "api__ml_local.h"
#include "ml_local.h"
#include <time.h>
#include "..\..\General\gen_ml/config.h"
#include "resource.h"
#include "../nu/listview.h"
#include "..\..\General\gen_ml/gaystring.h"
#include "./scanfolderbrowser.h"
#include "../replicant/nu/AutoChar.h"
#include "../replicant/nu/AutoWide.h"
#include <shlwapi.h>
#include <strsafe.h>

extern HWND subWnd = 0, prefsWnd = 0;
extern int g_bgrescan_int, g_bgrescan_do, g_autochannel_do;
HWND g_bgrescan_status_hwnd;
static W_ListView *m_dir_lv;


static void EnableDisableRecentPlayingControls( HWND hwndDlg, int tracking )
{
	if ( tracking == -1 ) tracking = !!g_config->ReadInt( L"trackplays", 1 );
	EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK7 ), tracking );
	EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK8 ), tracking );
	EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT2 ), tracking && !!g_config->ReadInt( L"trackplays_wait_secs", 0 ) );
	EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT3 ), tracking && !!g_config->ReadInt( L"trackplays_wait_percent", 0 ) );
	EnableWindow( GetDlgItem( hwndDlg, IDC_STATIC4 ), tracking );
	EnableWindow( GetDlgItem( hwndDlg, IDC_STATIC5 ), tracking );
}

// Recently Played
// When 'Recently Played' is enabled, Winamp will keep track of\nwhen and how many times items in the Media Library are played.
static INT_PTR CALLBACK Prefs1Proc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			CheckDlgButton( hwndDlg, IDC_REMEMBER_SEARCH, !!g_config->ReadInt( L"remembersearch", 0 ) );
			CheckDlgButton( hwndDlg, IDC_CHECK3,          !!g_config->ReadInt( L"useminiinfo2",   0 ) );

			CheckDlgButton( hwndDlg, IDC_CHECK_ATF,       !!g_config->ReadInt( L"newtitle",       1 ) );
			CheckDlgButton( hwndDlg, IDC_CHECK5,          !!g_config->ReadInt( L"audiorefine",    0 ) );
			CheckDlgButton( hwndDlg, IDC_CHECK6,           !g_config->ReadInt( L"dbloadatstart",  1 ) );

			CheckDlgButton( hwndDlg, IDC_ARTIST_AS_ALBUMARTIST, g_config->ReadInt( L"artist_as_albumartist", 1 ) );

			SetDlgItemInt( hwndDlg, IDC_EDIT_QUERYDELAY, g_config->ReadInt( L"querydelay", 250 ), 0 );

			int tracking = !!g_config->ReadInt( L"trackplays", 1 );
			CheckDlgButton( hwndDlg, IDC_CHECK2, tracking );
			CheckDlgButton( hwndDlg, IDC_CHECK7, !!g_config->ReadInt( L"trackplays_wait_secs", 0 ) );
			CheckDlgButton( hwndDlg, IDC_CHECK8, !!g_config->ReadInt( L"trackplays_wait_percent", 0 ) );
			SetDlgItemInt( hwndDlg, IDC_EDIT2, g_config->ReadInt( L"trackplays_wait_secs_lim", 5 ), 0 );
			SetDlgItemInt( hwndDlg, IDC_EDIT3, g_config->ReadInt( L"trackplays_wait_percent_lim", 50 ), 0 );
			EnableDisableRecentPlayingControls( hwndDlg, tracking );
		}
		break;
		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case IDC_CHECK3:
					g_config->WriteInt( L"useminiinfo2", !!IsDlgButtonChecked( hwndDlg, IDC_CHECK3 ) );
					PostMessage( plugin.hwndLibraryParent, WM_USER + 30, 0, 0 );
					break;
				case IDC_REMEMBER_SEARCH:
					g_config->WriteInt( L"remembersearch", !!IsDlgButtonChecked( hwndDlg, IDC_REMEMBER_SEARCH ) );
					break;
				case IDC_CHECK_ATF:
					g_config->WriteInt( L"newtitle", !!IsDlgButtonChecked( hwndDlg, IDC_CHECK_ATF ) );
					break;
				case IDC_CHECK5:
					g_config->WriteInt( L"audiorefine", !!IsDlgButtonChecked( hwndDlg, IDC_CHECK5 ) );
					if ( plugin.hwndLibraryParent ) PostMessage( plugin.hwndLibraryParent, WM_USER + 30, 0, 0 );
					break;
				case IDC_CHECK6:
					g_config->WriteInt( L"dbloadatstart", !IsDlgButtonChecked( hwndDlg, IDC_CHECK6 ) );
					break;
				case IDC_CHECK2:
				{
					int tracking = !!IsDlgButtonChecked( hwndDlg, IDC_CHECK2 );
					g_config->WriteInt( L"trackplays", tracking );
					EnableDisableRecentPlayingControls( hwndDlg, tracking );
				}
				break;
				case IDC_CHECK7:
				{
					g_config->WriteInt( L"trackplays_wait_secs", !!IsDlgButtonChecked( hwndDlg, IDC_CHECK7 ) );
					EnableDisableRecentPlayingControls( hwndDlg, -1 );
				}
				break;
				case IDC_CHECK8:
				{
					g_config->WriteInt( L"trackplays_wait_percent", !!IsDlgButtonChecked( hwndDlg, IDC_CHECK8 ) );
					EnableDisableRecentPlayingControls( hwndDlg, -1 );
				}
				break;
				case IDC_BUTTON2:
				{
					wchar_t title[ 64 ] = { 0 };
					WASABI_API_LNGSTRINGW_BUF( IDS_RECENTLY_PLAYED, title, 64 );
					MessageBoxW( hwndDlg, WASABI_API_LNGSTRINGW( IDS_RECENTLY_PLAYED_TEXT ), title, 0 );
				}
				break;
				case IDC_ARTIST_AS_ALBUMARTIST:
				{
					int oldValue = g_config->ReadInt( L"artist_as_albumartist", 1 );
					int newValue = !!IsDlgButtonChecked( hwndDlg, IDC_ARTIST_AS_ALBUMARTIST );
					if ( oldValue != newValue )
					{
						// TODO: prompt to re-read metadata on entire library
					}
					g_config->WriteInt( L"artist_as_albumartist", newValue );
				}
				break;
				case IDC_EDIT_QUERYDELAY:
				{
					if ( HIWORD( wParam ) == EN_CHANGE )
					{
						BOOL t;
						int v = GetDlgItemInt( hwndDlg, IDC_EDIT_QUERYDELAY, &t, FALSE );
						if ( t )
						{
							if ( v < 1 )
							{
								v = 1;
								SetDlgItemInt( hwndDlg, IDC_EDIT_QUERYDELAY, v, 0 );
							}
							else if ( v > 5000 )
							{
								v = 5000;
								SetDlgItemInt( hwndDlg, IDC_EDIT_QUERYDELAY, v, 0 );
							}
							g_config->WriteInt( L"querydelay", v );
							g_querydelay = v;
						}
					}
				}
				break;
				case IDC_BUTTON1:
					nukeLibrary( hwndDlg );
					break;
				case IDC_EDIT2:
					if ( HIWORD( wParam ) == EN_CHANGE )
					{
						BOOL t;
						int v = GetDlgItemInt( hwndDlg, IDC_EDIT2, &t, FALSE );
						if ( t )
						{
							if ( v < 0 )
							{
								v = 1;
								SetDlgItemInt( hwndDlg, IDC_EDIT2, v, 0 );
							}
							g_config->WriteInt( L"trackplays_wait_secs_lim", v );
						}
					}
					break;
				case IDC_EDIT3:
					if ( HIWORD( wParam ) == EN_CHANGE )
					{
						BOOL t;
						int v = GetDlgItemInt( hwndDlg, IDC_EDIT3, &t, FALSE );
						if ( t )
						{
							int tweaked = 0;
							if ( v > 99 )
							{
								v = 99;
								tweaked = 1;
							}
							else if ( v < 1 )
							{
								v = 1;
								tweaked = 1;
							}
							if ( tweaked )
							{
								SetDlgItemInt( hwndDlg, IDC_EDIT3, v, 0 );
							}

							g_config->WriteInt( L"trackplays_wait_percent_lim", v );
						}
					}
					break;
			}
			break;
	}
	return 0;
}

static void parseMetaStr( wchar_t *buf2, int *guess, int *meta, int *subdir )
{
	wchar_t metaPS[ 16 ] = { 0 }, metaMS[ 16 ] = { 0 }, smartPS[ 16 ] = { 0 }, smartMS[ 16 ] = { 0 }, guessS[ 16 ] = { 0 }, recurseS[ 16 ] = { 0 };
	StringCchPrintfW( recurseS, 16, L"-%s", WASABI_API_LNGSTRINGW( IDS_RECURSE_STR ) );
	StringCchPrintfW( guessS,   16, L"-%s", WASABI_API_LNGSTRINGW( IDS_GUESS_STR ) );
	StringCchPrintfW( smartPS,  16, L"+%s", WASABI_API_LNGSTRINGW( IDS_SMART_STR ) );
	StringCchPrintfW( smartMS,  16, L"-%s", WASABI_API_LNGSTRINGW( IDS_SMART_STR ) );
	StringCchPrintfW( metaPS,   16, L"+%s", WASABI_API_LNGSTRINGW( IDS_META_STR ) );
	StringCchPrintfW( metaMS,   16, L"-%s", WASABI_API_LNGSTRINGW( IDS_META_STR ) );

	if ( wcsstr( buf2, metaPS ) )
	{
		*meta = 1;
	}
	else if ( wcsstr( buf2, metaMS ) )
	{
		*meta = 0;
	}
	else
	{
		*meta = -1;
	}
	if ( wcsstr( buf2, smartPS ) )
	{
		*guess = 0;
	}
	else if ( wcsstr( buf2, smartMS ) )
	{
		*guess = 1;
	}
	else if ( wcsstr( buf2, guessS ) )
	{
		*guess = 2;
	}
	else
	{
		*guess = -1;
	}
	if ( wcsstr( buf2, recurseS ) )
	{
		*subdir = 0;
	}
	else
	{
		*subdir = 1;
	}
}

static void makeMetaStr( int guess_mode, int use_metadata, int subdir, wchar_t *buf, int bufLen )
{
	wchar_t guessstr[ 32 ] = { 0 }, recurseS[ 16 ] = { 0 };
	if ( guess_mode >= 0 )
	{
		if ( guess_mode == 1 )
			StringCchPrintfW( guessstr, 32, L"-%s", WASABI_API_LNGSTRINGW( IDS_SMART_STR ) );
		else if ( guess_mode == 2 )
			StringCchPrintfW( guessstr, 32, L"-%s", WASABI_API_LNGSTRINGW( IDS_GUESS_STR ) );
		else
			StringCchPrintfW( guessstr, 32, L"+%s", WASABI_API_LNGSTRINGW( IDS_SMART_STR ) );
	}

	if ( use_metadata >= 0 && guess_mode >= 0 )
	{
		wchar_t bufS[ 64 ] = { 0 };
		StringCchPrintfW( bufS, 64, L"%%c%s%%s%%s", WASABI_API_LNGSTRINGW( IDS_META_STR ) );
		StringCchPrintfW( recurseS, 16, L"%s%s", subdir ? L"" : L"-", subdir ? L"" : WASABI_API_LNGSTRINGW( IDS_RECURSE_STR ) );
		StringCchPrintfW( buf, bufLen, bufS, use_metadata ? L'+' : L'-', guessstr, recurseS );
	}
	else if ( use_metadata >= 0 )
	{
		wchar_t metaS[ 16 ] = { 0 };
		StringCchPrintfW( metaS, 16, L"%s%s", use_metadata ? L"+" : L"-", WASABI_API_LNGSTRINGW( IDS_META_STR ) );
		StringCchPrintfW( recurseS, 16, L"%s%s", subdir ? L"" : L"-", subdir ? L"" : WASABI_API_LNGSTRINGW( IDS_RECURSE_STR ) );
		StringCchPrintfW( buf, bufLen, L"%s%s", metaS, recurseS );
	}
	else if ( guess_mode >= 0 )
	{
		StringCchPrintfW( recurseS, 16, L"%s%s", subdir ? L"" : L"-", subdir ? L"" : WASABI_API_LNGSTRINGW( IDS_RECURSE_STR ) );
		StringCchPrintfW( buf, bufLen, L"%s%s", guessstr, recurseS );
	}
	else if ( !subdir )
	{
		StringCchPrintfW( buf, bufLen, L"-%s", WASABI_API_LNGSTRINGW( IDS_RECURSE_STR ) );
	}
	else
	{
		StringCchCopyW( buf, bufLen, WASABI_API_LNGSTRINGW( IDS_DEFAULT ) );
	}
}

static void saveList()
{
	GayStringW gs;
	int a = 0;
	int x, l = m_dir_lv->GetCount();
	for ( x = 0; x < l; x++ )
	{
		wchar_t buf[ 1024 ] = { 0 }, buf2[ 64 ] = { 0 };
		m_dir_lv->GetText( x, 0, buf2, 64 );
		m_dir_lv->GetText( x, 1, buf, 1024 );
		if ( buf[ 0 ] )
		{
			if ( a )
				gs.Append( L"|" );
			else
				a = 1;

			int meta;
			int guess;
			int subdir;
			parseMetaStr( buf2, &guess, &meta, &subdir );

			if ( meta >= 0 || guess >= 0 || !subdir )
			{
				gs.Append( L"<" );
				if ( guess >= 0 )
					gs.Append( guess == 1 ? L"s" : ( guess == 2 ? L"g" : L"S" ) );

				if ( meta >= 0 )
					gs.Append( meta ? L"M" : L"m" );

				if ( !subdir )
					gs.Append( L"r" );

				gs.Append( L">" );
			}
			gs.Append( buf );
		}
	}
	g_config->WriteString( "scandirlist", 0 ); // erase the old ini value
	g_config->WriteString( "scandirlist_utf8", AutoChar( gs.Get(), CP_UTF8 ) );
}

int autoscan_add_directory( const wchar_t *path, int *guess, int *meta, int *recurse, int noaddjustcheck )
{
	char *_s1 = g_config->ReadString( "scandirlist", "" );
	UINT codePage = CP_ACP;
	if ( !_s1 || !*_s1 )
	{
		_s1 = g_config->ReadString( "scandirlist_utf8", "" );
		codePage = CP_UTF8;
	}

	wchar_t *s1 = AutoWideDup( _s1, codePage );

	int bufLen = 0;
	wchar_t *s = (wchar_t *)calloc( ( bufLen = ( wcslen( s1 ) + 2 ) ), sizeof( wchar_t ) );
	if ( s )
	{
		wcsncpy( s, s1, bufLen );
		s[ wcslen( s ) + 1 ] = 0;

		wchar_t *p = s;
		while ( p && *p == L'|' ) p++;

		while ( ( p = wcsstr( p, L"|" ) ) )
		{
			*p++ = 0;
			while ( p && *p == L'|' ) p++;
		}
		p = s;
		while ( p && *p )
		{
			while ( p && *p == L'|' ) p++;

			int use_meta = -1;
			int do_guess = -1;
			int do_recurse = 1;
			if ( *p == L'<' && wcsstr( p, L">" ) )
			{
				p++;
				while ( p && *p != L'>' )
				{
					// <MmSs>can prefix directory
					// M=metadata use override
					// m=no metadata
					// S=smart guessing
					// s=stupid guessing
					// g=no guessing
					if ( *p == L'M' )
						use_meta = 1;
					else if ( *p == L'm' )
						use_meta = 0;
					else if ( *p == L'S' )
						do_guess = 0;
					else if ( *p == L's' )
						do_guess = 1;
					else if ( *p == L'g' )
						do_guess = 2;
					else if ( *p == L'r' )
						do_recurse = 0;

					p++;
				}
				p++;
			}

			if ( !_wcsnicmp( p, path, wcslen( p ) ) && ( path[ wcslen( p ) ] == L'\\' || !path[ wcslen( p ) ] ) )
			{
				free( s ); // directory already in there

				if ( meta )
					*meta = use_meta;

				if ( guess )
					*guess = do_guess;

				if ( recurse )
					*recurse = do_recurse;

				return 1;
			}
			p += wcslen( p ) + 1;
		}
		free( s );
	}

	if ( !noaddjustcheck )
	{
		WIN32_FIND_DATAW fd = { 0 };
		int bufLen = 0;
		wchar_t *s = (wchar_t *)calloc( ( bufLen = ( wcslen( path ) + 32 ) ), sizeof( wchar_t ) );
		StringCchPrintfW( s, bufLen, L"%s\\*.*", path );
		HANDLE h = FindFirstFileW( s, &fd );
		free( s );
		if ( h != INVALID_HANDLE_VALUE )
		{
			FindClose( h ); // we are a directory, yay
			GayStringW gs;
			gs.Set( s1 );

			if ( s1[ 0 ] )
				gs.Append( L"|" );

			gs.Append( path );

			g_config->WriteString( "scandirlist", 0 );
			g_config->WriteString( "scandirlist_utf8", AutoCharDup( gs.Get(), codePage, 0 ) );

			if ( m_dir_lv )
			{
				int x = m_dir_lv->GetCount();
				m_dir_lv->InsertItem( x, WASABI_API_LNGSTRINGW( IDS_DEFAULT ), 0 );
				m_dir_lv->SetItemText( x, 1, path );
			}
		}
	}
	return 0;
}

static int m_edit_idx;
static INT_PTR CALLBACK editDirProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			wchar_t buf[ 1024 ] = { 0 };
			SHAutoComplete( GetDlgItem( hwndDlg, IDC_EDIT1 ), SHACF_AUTOAPPEND_FORCE_ON | SHACF_AUTOSUGGEST_FORCE_ON | /*SHACF_FILESYS_DIRS*/0x020 | 0x010 | SHACF_USETAB );
			m_dir_lv->GetText( m_edit_idx, 1, buf, 1024 );
			SetDlgItemTextW( hwndDlg, IDC_EDIT1, buf );
			m_dir_lv->GetText( m_edit_idx, 0, buf, 1024 );
			int guess, meta, subdir;
			parseMetaStr( buf, &guess, &meta, &subdir );
			if ( meta )
				CheckDlgButton( hwndDlg, IDC_CHECK1, meta > 0 ? BST_CHECKED : BST_INDETERMINATE );

			if ( guess < 0 )
				CheckDlgButton( hwndDlg, IDC_RADIO6, BST_CHECKED );
			else if ( !guess )
				CheckDlgButton( hwndDlg, IDC_RADIO1, BST_CHECKED );
			else if ( guess == 2 )
				CheckDlgButton( hwndDlg, IDC_RADIO8, BST_CHECKED );
			else if ( guess > 0 )
				CheckDlgButton( hwndDlg, IDC_RADIO2, BST_CHECKED );

			if ( subdir ) CheckDlgButton( hwndDlg, IDC_CHECK2, BST_CHECKED );
		}
		return 0;
		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case IDOK:
				{
					//save to m_edit_idx :)
					wchar_t buf[ 2048 ] = { 0 };
					GetDlgItemTextW( hwndDlg, IDC_EDIT1, buf, 2048 );
					m_dir_lv->SetItemText( m_edit_idx, 1, buf );

					int meta = IsDlgButtonChecked( hwndDlg, IDC_CHECK1 );
					if ( meta == BST_CHECKED )
						meta = 1;
					else if ( meta == BST_INDETERMINATE )
						meta = -1;
					else
						meta = 0;

					int guess = -1;
					if ( IsDlgButtonChecked( hwndDlg, IDC_RADIO1 ) )
						guess = 0;
					else if ( IsDlgButtonChecked( hwndDlg, IDC_RADIO2 ) )
						guess = 1;
					else if ( IsDlgButtonChecked( hwndDlg, IDC_RADIO8 ) )
						guess = 2;

					int subdir = 1;
					if ( !IsDlgButtonChecked( hwndDlg, IDC_CHECK2 ) )
						subdir = 0;

					makeMetaStr( guess, meta, subdir, buf, 64 );
					m_dir_lv->SetItemText( m_edit_idx, 0, buf );
					EndDialog( hwndDlg, 1 );
				}
				return 0;
				case IDCANCEL:
					EndDialog( hwndDlg, 0 );
					return 0;
				case IDC_BUTTON1:
					wchar_t path[ MAX_PATH ] = { 0 };
					GetDlgItemTextW( hwndDlg, IDC_EDIT1, path, MAX_PATH );
					ScanFolderBrowser browse( FALSE );
					browse.SetSelection( path );
					if ( browse.Browse( hwndDlg ) )
					{
						wchar_t path[ MAX_PATH ] = { 0 };
						SHGetPathFromIDListW( browse.GetPIDL(), path );
						SetDlgItemTextW( hwndDlg, IDC_EDIT1, path );
					}
					return 0;
			}
			return 0;
	}
	return 0;
}

void hideShowMetadataRadioboxes( HWND hwndDlg )
{
	int enabled = IsDlgButtonChecked( hwndDlg, IDC_CHECK1 );
	EnableWindow( GetDlgItem( hwndDlg, IDC_STATIC1 ), enabled );
	EnableWindow( GetDlgItem( hwndDlg, IDC_STATIC2 ), enabled );
	EnableWindow( GetDlgItem( hwndDlg, IDC_COMBO1 ),  enabled );
	EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO1 ),  enabled );
	EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO2 ),  enabled );
	EnableWindow( GetDlgItem( hwndDlg, IDC_RADIO3 ),  enabled );
}

static INT_PTR CALLBACK confMetaProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			if ( g_config->ReadInt( L"usemetadata", 1 ) ) CheckDlgButton( hwndDlg, IDC_CHECK1, BST_CHECKED );
			{
				int gm = g_config->ReadInt( L"guessmode", 0 );
				CheckDlgButton( hwndDlg, gm == 2 ? IDC_RADIO3 : ( gm == 1 ? IDC_RADIO2 : IDC_RADIO1 ), BST_CHECKED );
			}

			SendDlgItemMessageW( hwndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW( IDS_ANY ) );
			SendDlgItemMessageW( hwndDlg, IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW( IDS_ALL ) );
			SendDlgItemMessage( hwndDlg, IDC_COMBO1, CB_SETCURSEL, g_guessifany ? 0 : 1, 0 );
			hideShowMetadataRadioboxes( hwndDlg );
		}
		break;
		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case IDOK:
					g_config->WriteInt( L"usemetadata", !!IsDlgButtonChecked( hwndDlg, IDC_CHECK1 ) );
					{
						int a = SendDlgItemMessage( hwndDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0 );
						g_guessifany = ( a == 0 );
						g_config->WriteInt( L"guessifany", g_guessifany );
					}

					if ( IsDlgButtonChecked( hwndDlg, IDC_RADIO1 ) )
						g_config->WriteInt( L"guessmode", 0 );

					if ( IsDlgButtonChecked( hwndDlg, IDC_RADIO2 ) )
						g_config->WriteInt( L"guessmode", 1 );

					if ( IsDlgButtonChecked( hwndDlg, IDC_RADIO3 ) )
						g_config->WriteInt( L"guessmode", 2 );

				case IDCANCEL:
					EndDialog( hwndDlg, 0 );
					break;
				case IDC_CHECK1:
					hideShowMetadataRadioboxes( hwndDlg );
					break;
			}
			break;
	}
	return 0;
}

static INT_PTR CALLBACK Prefs3Proc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static int m_last_isscanning, m_set_rescan_int;
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			m_last_isscanning = -1;
			m_set_rescan_int = 0;

			SendMessage( hwndDlg, WM_TIMER, 100, 0 );
			SetTimer( hwndDlg, 100, 1000, NULL );
			g_bgrescan_status_hwnd = GetDlgItem( hwndDlg, IDC_STATUS );

			CheckDlgButton( hwndDlg, IDC_CHECK4, g_bgrescan_do ? BST_CHECKED : 0 );
			SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_CHECK4, 0 ), 0 );
			CheckDlgButton( hwndDlg, IDC_CHECK3, g_config->ReadInt( L"bgrescan_startup", 0 ) ? BST_CHECKED : 0 );
			CheckDlgButton( hwndDlg, IDC_CHECK2, g_config->ReadInt( L"bgrescan_compact", 1 ) ? BST_CHECKED : 0 );
			CheckDlgButton( hwndDlg, IDC_CHECK5, g_config->ReadInt( L"autoaddplays", 0 ) ? BST_CHECKED : 0 );

			SendDlgItemMessage( hwndDlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG( 9999, 1 ) );
			SetDlgItemInt( hwndDlg, IDC_EDIT3, g_bgrescan_int, FALSE );
			m_set_rescan_int = 1;

			delete m_dir_lv;
			m_dir_lv = new W_ListView;
			m_dir_lv->setwnd( GetDlgItem( hwndDlg, IDC_LIST1 ) );
			m_dir_lv->AddCol( WASABI_API_LNGSTRINGW( IDS_OPTIONS ), 120 );
			m_dir_lv->AddCol( WASABI_API_LNGSTRINGW( IDS_DIRECTORY ), 500 );
			char *_s1 = g_config->ReadString( "scandirlist", "" );
			UINT codePage = CP_ACP;
			if ( !_s1 || !*_s1 )
			{
				_s1 = g_config->ReadString( "scandirlist_utf8", "" );
				codePage = CP_UTF8;
			}
			wchar_t *s1 = AutoWideDup( _s1, codePage );
			int bufLen = 0;
			wchar_t *s = (wchar_t *)calloc( ( bufLen = ( wcslen( s1 ) + 2 ) ), sizeof( wchar_t ) );
			if ( s )
			{
				wcsncpy( s, s1, bufLen );
				s[ wcslen( s ) + 1 ] = 0;

				wchar_t *p = s;
				while ( p && *p == L'|' ) p++;

				while ( ( p = wcsstr( p, L"|" ) ) )
				{
					*p++ = 0;
					while ( p && *p == L'|' ) p++;
				}
				int x = 0;
				p = s;
				while ( p && *p )
				{
					while ( p && *p == L'|' ) p++;

					int guess_mode = -1;
					int use_metadata = -1;
					int subdir = 1;
					if ( *p == L'<' && wcsstr( p, L">" ) )
					{
						p++;
						while ( p && *p != L'>' )
						{
							// <MmSs>can prefix directory
							// M=metadata use override
							// m=no metadata
							// S=smart guessing
							// s=stupid guessing
							if ( *p == L'M' )
								use_metadata = 1;
							else if ( *p == L'm' )
								use_metadata = 0;
							else if ( *p == L'S' )
								guess_mode = 0;
							else if ( *p == L's' )
								guess_mode = 1;
							else if ( *p == L'r' )
								subdir = 0;
							else if ( *p == L'g' )
								guess_mode = 2;

							p++;
						}
						p++;
					}
					wchar_t buf[ 64 ] = { 0 };
					makeMetaStr( guess_mode, use_metadata, subdir, buf, 64 );

					m_dir_lv->InsertItem( x, buf, 0 );
					m_dir_lv->SetItemText( x, 1, p );
					x++;

					p += wcslen( p ) + 1;
				}
				free( s );
			}

			if ( NULL != WASABI_API_APP )
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel( m_dir_lv->getwnd(), TRUE );
		}
		break;
		// process this to update the ui state when a scan is running and we didn't manually run it
		case WM_USER + 101:
			SendMessage( hwndDlg, WM_TIMER, 100, 0 );
			SetTimer( hwndDlg, 100, 1000, NULL );
			break;
		case WM_TIMER:
			if ( wParam == 100 )
			{
				extern int g_bgscan_scanning;
				if ( !!g_bgscan_scanning != !!m_last_isscanning )
				{
					m_last_isscanning = !!g_bgscan_scanning;
					SetDlgItemTextW( hwndDlg, IDC_RESCAN, WASABI_API_LNGSTRINGW( ( m_last_isscanning ? IDS_STOP_SCAN : IDS_RESCAN_NOW ) ) );
				}
			}
			break;
		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case IDC_CHECK5:
					g_config->WriteInt( L"autoaddplays", !!IsDlgButtonChecked( hwndDlg, IDC_CHECK5 ) );
					break;
				case IDC_CHECK2:
					g_config->WriteInt( L"bgrescan_compact", !!IsDlgButtonChecked( hwndDlg, IDC_CHECK2 ) );
					break;
				case IDC_CHECK3:
					g_config->WriteInt( L"bgrescan_startup", !!IsDlgButtonChecked( hwndDlg, IDC_CHECK3 ) );
					break;
				case IDC_CHECK4:
					g_config->WriteInt( L"bgrescan_do", g_bgrescan_do = !!IsDlgButtonChecked( hwndDlg, IDC_CHECK4 ) );
					EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT3 ), g_bgrescan_do );
					EnableWindow( GetDlgItem( hwndDlg, IDC_SPIN1 ), g_bgrescan_do );
					break;
				case IDC_EDIT3:
				{
					if ( HIWORD( wParam ) == EN_CHANGE && m_set_rescan_int )
					{
						BOOL t;
						int x = GetDlgItemInt( hwndDlg, IDC_EDIT3, &t, FALSE );
						if ( t )
						{
							if ( x < 1 ) x = 1;
							g_config->WriteInt( L"bgrescan_int", g_bgrescan_int = x );
							extern time_t g_bgscan_last_rescan;
							g_bgscan_last_rescan = time( NULL );
						}
					}
				}
				break;
				case IDC_BUTTON1:
				{
					ScanFolderBrowser browse( FALSE );
					if ( browse.Browse( hwndDlg ) )
					{
						GayStringW n;
						wchar_t path[ MAX_PATH ] = { 0 };

						SHGetPathFromIDListW( browse.GetPIDL(), path );

						char *_s1 = g_config->ReadString( "scandirlist", "" );
						UINT codePage = CP_ACP;
						if ( !_s1 || !*_s1 )
						{
							_s1 = g_config->ReadString( "scandirlist_utf8", "" );
							codePage = CP_UTF8;
						}

						wchar_t *scanlist = AutoWide( _s1, codePage );
						n.Set( scanlist );

						if ( scanlist[ 0 ] )
							n.Append( L"|" );

						n.Append( path );
						g_config->WriteString( "scandirlist", 0 );
						g_config->WriteString( "scandirlist_utf8", AutoChar( n.Get(), CP_UTF8 ) );
						int x = m_dir_lv->GetCount();
						m_dir_lv->InsertItem( x, WASABI_API_LNGSTRINGW( IDS_DEFAULT ), 0 );
						m_dir_lv->SetItemText( x, 1, path );
					}
				}
				break;
				case IDC_BUTTON4:
				{
					int x, l = m_dir_lv->GetCount(), s = 0;
					for ( x = 0; x < l; x++ )
					{
						if ( m_dir_lv->GetSelected( x ) )
						{
							m_edit_idx = x;
							if ( WASABI_API_DIALOGBOXW( IDD_EDITDIR, hwndDlg, editDirProc ) )
								s = 1;
							else
								break;
						}
					}
					if ( s )
					{
						saveList();
					}
				}
				break;
				case IDC_BUTTON3:
				{
					int x, l = m_dir_lv->GetCount();
					int s = 0;
					for ( x = 0; x < l; x++ )
					{
						if ( m_dir_lv->GetSelected( x ) )
						{
							s = 1;
							m_dir_lv->DeleteItem( x-- );
							l--;
						}
					}
					if ( s )
					{
						saveList();
					}
				}
				break;
				case IDC_RESCAN:
					extern int g_bgscan_scanning;
					extern time_t g_bgscan_last_rescan;
					if ( g_bgscan_scanning )
					{
						Scan_Cancel();
						SetWindowTextW( g_bgrescan_status_hwnd, WASABI_API_LNGSTRINGW( IDS_RESCAN_ABORTED ) );
						SendMessage( hwndDlg, WM_TIMER, 100, 0 );
					}
					else
					{
						extern int openDb( void );
						openDb();
						if ( plugin.hwndLibraryParent ) SendMessage( plugin.hwndLibraryParent, WM_USER + 575, 0xffff00dd, 0 );
						SendMessage( hwndDlg, WM_TIMER, 100, 0 );
					}
					break;
				case IDC_CONFMETA:
					WASABI_API_DIALOGBOXW( IDD_PREFS_METADATA, hwndDlg, confMetaProc );
					break;
			};
			break;
		case WM_NOTIFY:
		{
			LPNMHDR l = (LPNMHDR)lParam;
			if ( l->idFrom == IDC_LIST1 && l->code == NM_DBLCLK )
			{
				SendMessage( hwndDlg, WM_COMMAND, IDC_BUTTON4, 0 );
			}
		}
		break;
		case WM_DESTROY:
			m_set_rescan_int = 0;
			g_bgrescan_status_hwnd = 0;
			if ( NULL != WASABI_API_APP )
				WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel( m_dir_lv->getwnd(), FALSE );
			delete m_dir_lv;
			m_dir_lv = 0;
			break;
	}
	return 0;
}

static void _dosetsel( HWND hwndDlg )
{
	HWND tabwnd = GetDlgItem( hwndDlg, IDC_TAB1 );
	int sel = TabCtrl_GetCurSel( tabwnd );

	if ( sel >= 0 && ( sel != g_config->ReadInt( L"lastprefp", 0 ) || !subWnd ) )
	{
		g_config->WriteInt( L"lastprefp", sel );
		if ( subWnd )
			DestroyWindow( subWnd );

		subWnd = 0;

		UINT t = 0;
		DLGPROC p = NULL;
		switch ( sel )
		{
			case 0: t = IDD_PREFS1; p = Prefs1Proc; break;
			case 1: t = IDD_PREFS3; p = Prefs3Proc; break;
		}

		if ( t ) subWnd = WASABI_API_CREATEDIALOGW( t, hwndDlg, p );

		if ( subWnd )
		{
			RECT r;
			GetClientRect( tabwnd, &r );
			TabCtrl_AdjustRect( tabwnd, FALSE, &r );
			SetWindowPos( subWnd, HWND_TOP, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOACTIVATE );
			ShowWindow( subWnd, SW_SHOWNA );
		}

		if ( !SendMessage( plugin.hwndWinampParent, WM_WA_IPC, IPC_ISWINTHEMEPRESENT, IPC_USE_UXTHEME_FUNC ) )
		{
			SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)tabwnd, IPC_USE_UXTHEME_FUNC );
			SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)subWnd, IPC_USE_UXTHEME_FUNC );
		}
	}
}

// frame proc
INT_PTR CALLBACK PrefsProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			TCITEMW item;
			HWND tabwnd = GetDlgItem( hwndDlg, IDC_TAB1 );
			item.mask = TCIF_TEXT;
			item.pszText = WASABI_API_LNGSTRINGW( IDS_OPTIONS );
			SendMessageW( tabwnd, TCM_INSERTITEMW, 0, (LPARAM)&item );
			item.pszText = WASABI_API_LNGSTRINGW( IDS_WATCH_FOLDERS );
			SendMessageW( tabwnd, TCM_INSERTITEMW, 1, (LPARAM)&item );
			TabCtrl_SetCurSel( tabwnd, g_config->ReadInt( L"lastprefp", 0 ) );
			_dosetsel( hwndDlg );
			prefsWnd = hwndDlg;
		}
		return 0;
		case WM_NOTIFY:
		{
			LPNMHDR p = (LPNMHDR)lParam;
			if ( p->idFrom == IDC_TAB1 && p->code == TCN_SELCHANGE ) _dosetsel( hwndDlg );
		}
		return 0;
		case WM_DESTROY:
			subWnd = NULL;
			prefsWnd = NULL;
			return 0;
	}
	return 0;
}

void refreshPrefs( int screen )
{
	if ( subWnd && g_config->ReadInt( L"lastprefp", -1 ) == screen )
	{
		if ( screen == 4 ) SendMessage( subWnd, WM_INITDIALOG, 0, 0 );
	}
}