#include <strsafe.h>
#include <shlobj.h>
#include <direct.h>

#include <atomic>

#include "main.h"
#include "Playlist.h"
#include "resource.h"
#include "nu/listview.h"
#include "CurrentPlaylist.h"
#include "nu/AutoCharFn.h"
#include "../../General/gen_ml/ml.h"
#include "../../General/gen_ml/ml_ipc.h"
#include "SendTo.h"
#include "PlaylistView.h"
#include "PlaylistDirectoryCallback.h"
#include "api__ml_playlists.h"
#include "../../General/gen_ml/menufucker.h"
#include "nu/menushortcuts.h"
#include "../../General/gen_ml/ml_ipc_0313.h"
#include "ml_local/api_mldb.h"
#include "ml_pmp/pmp.h"
#include "replicant/nswasabi/ReferenceCounted.h"
#include "replicant/nx/win/nxstring.h"
#include "playlist/plstring.h"

#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoLock.h"
#include "../replicant/nu/AutoLock.h"

#include "../../../WAT/wa_logger.h"

#include <iostream>
#include <fstream>

#include "../../../Components/wac_network/wac_network_http_receiver_api.h"
#include "../../../Components/wac_downloadManager/wac_downloadManager_api.h"

//#include "../replicant/replicant/metadata/metadata.h"

using namespace Nullsoft::Utility;


#define SIMULTANEOUS_DOWNLOADS            1
#define PLAYLIST_DOWNLOAD_SUBFOLDER       "\\Winamp_Library\\"
#define WINAMP_REDIRECT_LINK_PROXY_FILE   L"http://client.winamp.com/fileproxy?destination="

std::vector<DownloadToken> plDownloads;
LockGuard itemsPlaylistQueueLock;

Playlist          currentPlaylist;
wchar_t           currentPlaylistFilename[FILENAME_SIZE] = {0};
wchar_t           currentPlaylistTitle[FILETITLE_SIZE]   = {0};
wchar_t           current_playing[FILENAME_SIZE]         = {0};
W_ListView        playlist_list;
static GUID       playlist_guid                          = INVALID_GUID;
int               IPC_LIBRARY_SENDTOMENU;
int               we_are_drag_and_dropping               = 0;

static void       AutoSizePlaylistColumns();

static SendToMenu sendTo;
viewButtons       view                                   = {0};

typedef enum 
{
	SCROLLDIR_NONE = 0,
	SCROLLDIR_UP   = 1,
	SCROLLDIR_DOWN = -1,
} SCROLLDIR;

#define SCROLLTIMER_ID		100

static INT scrollDelay       = 0;
static INT scrollTimerElapse = 0;
static int scrollDirection   = SCROLLDIR_NONE;

void UpdatePlaylistTime(HWND hwndDlg);

static bool opened  = false;
static bool changed = false;
static bool loaded  = false;

HWND activeHWND    = 0;
HWND saveHWND      = 0;

int  groupBtn      = 1; 
int  customAllowed = 0; 
int  enqueuedef    = 0;

void Changed( bool _changed = true )
{
	changed = _changed;

	EnableWindow( saveHWND, changed );
}

void SyncPlaylist()
{
	if ( opened )
	{
		playlist_list.SetVirtualCount( (INT)currentPlaylist.GetNumItems() );
		playlist_list.RefreshAll();
		UpdatePlaylistTime( GetParent( playlist_list.getwnd() ) );

		if ( !current_playing[ 0 ] )
			lstrcpynW( current_playing, (wchar_t *)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_PLAYING_FILENAME ), FILENAME_SIZE );

		PostMessage( activeHWND, WM_APP + 103, (WPARAM)current_playing, 1 );
	}
}

void SyncMenuWithAccelerators( HWND hwndDlg, HMENU menu )
{
	HACCEL szAccel[ 24 ] = { 0 };
	INT c = WASABI_API_APP->app_getAccelerators( hwndDlg, szAccel, sizeof( szAccel ) / sizeof( szAccel[ 0 ] ), FALSE );

	AppendMenuShortcuts( menu, szAccel, c, MSF_REPLACE );
}

void UpdateMenuItems( HWND hwndDlg, HMENU menu )
{
	bool swapPlayEnqueue = false;
	if ( g_config->ReadInt( L"enqueuedef", 0 ) == 1 )
	{
		SwapPlayEnqueueInMenu( menu );

		swapPlayEnqueue = true;
	}

	SyncMenuWithAccelerators( hwndDlg, menu );

	if ( swapPlayEnqueue )
		SwapPlayEnqueueInMenu( menu );
}

void TagEditor( HWND hwnd )
{
	wchar_t fn[ 1024 ] = { 0 };
	wchar_t ft[ 1024 ] = { 0 };
	int v = playlist_list.GetCount();
	for ( int x = 0; x < v; x++ )
	{
		if ( playlist_list.GetSelected( x ) )
		{
			currentPlaylist.GetItem( x, fn, 1024 );
			infoBoxParamW p;
			p.filename = fn;
			p.parent   = hwnd;

			if ( SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&p, IPC_INFOBOXW ) )
				break;

			int length = -1;
			mediaLibrary.GetFileInfo( fn, ft, 1024, &length );

			currentPlaylist.SetItemTitle( x, ft );
			currentPlaylist.SetItemLengthMilliseconds( x, length * 1000 );

			playlist_list.RefreshItem( x );
			Changed();
		}
	}

	MSG msg;
	while ( PeekMessage( &msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) ); //eat return
}

void myOpenURL(HWND hwnd, wchar_t *loc);

void Playlist_GenerateHtmlPlaylist(void)
{
	FILE *fp = 0;
	wchar_t filename[MAX_PATH], tp[MAX_PATH] = {0};

	if (!GetTempPathW(MAX_PATH,tp))
		StringCchCopyW(tp, MAX_PATH, L".");

	if ( GetTempFileNameW( tp, L"WHT", 0, filename ) )
	{
		DeleteFileW( filename );
		StringCchCatW( filename, MAX_PATH, L".html" );
	}
	else
		StringCchCopyW(filename, MAX_PATH, L"wahtml_tmp.html");

	fp = _wfopen(filename, L"wt");
	if ( !fp )
	{
		//MessageBox(activeHWND, IDS_HTML_ERROR_WRITE, IDS_ERROR, MB_OK | MB_ICONWARNING);
		return;
	}

	fprintf(fp, "<!DOCTYPE html>\n"
				"<html><head>\n"
				"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
				"<meta name=\"generator\" content=\"Winamp 5.9\">\n"
				"<style type=\"text/css\">body{background:#000040;font-family:arial,helvetica;font-size:9pt;font-weight:normal;}"
				".name{margin-top:-1em;margin-left:15px;font-size:40pt;color:#004080;text-align:left;font-weight:900;}"
				".name-small{margin-top:-3em;margin-left:140px;font-size:22pt;color:#E1E1E1;text-align:left;}"
				"table{font-size:9pt;color:#004080;text-align:left;border-width:0px;padding:0px;letter-spacing:normal;}"
				"hr{border:0;background-color:#FFBF00;height:1px;}"
				"ol{color:#FFFFFF;font-size:11pt;}"
				"table{margin-left:15px;color:#409FFF;border-width:0px;}"
				".val{color:#FFBF00;}"
				".header{color:#FFBF00;font-size:14pt;}"
				"</style>\n"
				"<title>Winamp Generated PlayList</title></head>\n"
				"<body>"
				"<div>"
				"<div class=\"name\"><p>WINAMP</p></div>"
				"<div class=\"name-small\"><p>playlist</p></div>"
				"</div>"
				"<hr><div>\n"
				"<table><tr><td>\n");

	int x, t = playlist_list.GetCount(), t_in_pl = 0, n_un = 0;
	for ( x = 0; x < t; x++ )
	{
		int a = currentPlaylist.GetItemLengthMilliseconds( x );
		if ( a >= 0 )
			t_in_pl += ( a / 1000 );
		else
			n_un++;
	}

	if ( t != n_un )
	{
		int old_t_in_pl = t_in_pl;
		t_in_pl += ( n_un * t_in_pl ) / ( t - n_un );

		fprintf( fp, "<span class=\"val\">%d</span> track%s in playlist, ", t, t == 1 ? "" : "s" );
		fprintf( fp, "average track length: <span class=\"val\">%d:%02d", old_t_in_pl / ( t - n_un ) / 60, ( old_t_in_pl / ( t - n_un ) ) % 60 );

		fprintf( fp, "</span><br>%slaylist length: ", n_un ? "Estimated p" : "P" );

		if ( t_in_pl / 3600 )
		{
			fprintf( fp, "<span class=\"val\">%d</span> hour%s ", t_in_pl / 3600, t_in_pl / 3600 == 1 ? "" : "s" );
			t_in_pl %= 3600;
		}

		if ( t_in_pl / 60 )
		{
			fprintf( fp, "<span class=\"val\">%d</span> minute%s ", t_in_pl / 60, t_in_pl / 60 == 1 ? "" : "s" );
			t_in_pl %= 60;
		}

		fprintf( fp, "<span class=\"val\">%d</span> second%s %s", t_in_pl, t_in_pl == 1 ? "" : "s", n_un ? "<br>(" : "" );
		if ( n_un )
			fprintf( fp, "<span class=\"val\">%d</span> track%s of unknown length)", n_un, n_un == 1 ? "" : "s" );

		fprintf( fp,
				 "<br>Right-click <a href=\"file://%s\">here</a> to save this HTML file."
				 "</td></tr>",
				 (char *)AutoChar( filename, CP_UTF8 ) );
	}
	else
	{
		fprintf( fp, "There are no tracks in the current playlist.<br>" );
	}

	fprintf(fp, "</table></div>\n");

	if ( t > 0 )
	{
		fprintf( fp, "<blockquote><span class=\"header\">Playlist files:</span><ol>" );

		for ( x = 0; x < t; x++ )
		{
			wchar_t ft[ FILETITLE_SIZE ] = { 0 };
			currentPlaylist.GetItemTitle( x, ft, FILENAME_SIZE );

			AutoChar narrowFt( ft, CP_UTF8 );
			char *p = narrowFt;

			int l = currentPlaylist.GetItemLengthMilliseconds( x );
			if ( l > 0 )
				l /= 1000;

			fprintf( fp, "<li>" );

			while ( p && *p )
			{
				if ( *p == '&' )
					fprintf( fp, "&amp;" );
				else if ( *p == '<' )
					fprintf( fp, "&lt;" );
				else if ( *p == '>' )
					fprintf( fp, "&gt;" );
				else if ( *p == '\'' )
					fprintf( fp, "&#39;" );
				else if ( *p == '"' )
					fprintf( fp, "&quot;" );
				else
					fputc( *p, fp );

				p++;
			}

			if ( l > 0 )
				fprintf( fp, " (%d:%02d) \n", l / 60, l % 60 );
			else
				fprintf( fp, " \n" );
		}

		fprintf( fp, "</ol></blockquote>" );
	}

	fprintf(fp, "<hr><br></body></html>");
	fclose(fp);

	myOpenURL(activeHWND, filename);
}

void Playlist_ResetSelected()
{
	int i = playlist_list.GetCount();
	while ( i-- )
	{
		if ( playlist_list.GetSelected( i ) )
			currentPlaylist.ClearCache( i );
	}

	Changed();
	SyncPlaylist();
}

void Playlist_FindSelected()
{
	if ( playlist_list.GetSelectionMark() >= 0 )
	{
		int l = playlist_list.GetCount();
		for ( int i = 0; i < l; i++ )
		{
			if ( playlist_list.GetSelected( i ) )
				WASABI_API_EXPLORERFINDFILE->AddFile( (wchar_t *)currentPlaylist.ItemName( i ) );
		}

		WASABI_API_EXPLORERFINDFILE->ShowFiles();
	}
}

void Playlist_DeleteSelected( int selected )
{
	selected = !!selected; // convert to 0 or 1

	int i = playlist_list.GetCount();
	while ( i-- )
	{
		if ( !( playlist_list.GetSelected( i ) ^ selected ) )
		{
			currentPlaylist.Remove( i );
			playlist_list.Unselect( i );
		}
	}

	Changed();
	SyncPlaylist();
}

void Playlist_RecycleSelected( HWND hwndDlg, int selected )
{
	SHFILEOPSTRUCTW fileOp;
	fileOp.hwnd                  = hwndDlg;
	fileOp.wFunc                 = FO_DELETE;
	fileOp.pFrom                 = 0;
	fileOp.pTo                   = 0;
	fileOp.fFlags                = SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_USES_RECYCLEBIN ) ? FOF_ALLOWUNDO : 0;
	fileOp.fAnyOperationsAborted = 0;
	fileOp.hNameMappings         = 0;
	fileOp.lpszProgressTitle     = 0;

	selected = !!selected; // convert to 0 or 1
	int i = playlist_list.GetCount();

	wchar_t *files = new wchar_t[ i * ( MAX_PATH + 1 ) + 1 ];  // need room for each file name, null terminated. then have to null terminate the whole list
	if ( files )
	{
		wchar_t *curFile = files;

		for ( int x = 0; x < i; x++ )
		{
			if ( !( playlist_list.GetSelected( x ) ^ selected ) )
			{
				lstrcpynW( curFile, currentPlaylist.ItemName( x ), MAX_PATH );
				curFile += wcslen( currentPlaylist.ItemName( x ) ) + 1;
			}
		}

		if ( curFile != files )
		{
			curFile[ 0 ] = 0; // null terminate

			fileOp.pFrom = files;

			if ( SHFileOperationW( &fileOp ) )
			{
				wchar_t titleStr[ 32 ] = { 0 };
				MessageBox( hwndDlg, WASABI_API_LNGSTRINGW( IDS_ERROR_DELETING_FILES ), WASABI_API_LNGSTRINGW_BUF( IDS_ERROR, titleStr, 32 ), MB_OK );
			}
			else if ( !fileOp.fAnyOperationsAborted )
			{
				while ( i-- )
				{
					if ( !( playlist_list.GetSelected( i ) ^ selected ) )
					{
						currentPlaylist.Remove( i );
						playlist_list.Unselect( i );
					}
				}
			}
		}

		delete[] files;
	}
	else // if malloc failed ... maybe because there'l_enqueue_file too many items.
	{
		while ( i-- )
		{
			if ( !( playlist_list.GetSelected( i ) ^ selected ) )
			{
				fileOp.pFrom = currentPlaylist.ItemName( i );

				if ( SHFileOperationW( &fileOp ) )
					continue;

				if ( fileOp.fAnyOperationsAborted )
					break;

				currentPlaylist.Remove( i );
				playlist_list.Unselect( i );
			}
		}
	}

	Changed();
	SyncPlaylist();
}

int GetSelectedLength()
{
	int length   = 0;
	int selected = -1;

	while ( ( selected = playlist_list.GetNextSelected( selected ) ) != -1 )
	{
		int thisLen = currentPlaylist.GetItemLengthMilliseconds( selected );
		if ( thisLen > 0 )
			length += thisLen / 1000;
	}

	return length;
}

int GetTotalLength()
{
	int length = 0;
	int len    = playlist_list.GetCount();
	for ( int i = 0; i < len; i++ )
	{
		int thisLen = currentPlaylist.GetItemLengthMilliseconds( i );
		if ( thisLen > 0 )
			length += thisLen / 1000;
	}

	return length;
}

void FormatLength( wchar_t *str, int length, int buf_len )
{
	if ( !length )
		lstrcpynW( str, L"0:00", buf_len );
	else if ( length < 60 * 60 )
		StringCchPrintfW( str, buf_len, L"%d:%02d", length / 60, length % 60 );
	else
	{
		int total_days = length / ( 60 * 60 * 24 );
		if ( total_days )
		{
			length -= total_days * 60 * 60 * 24;
			StringCchPrintfW( str, buf_len, L"%d %s+%d:%02d:%02d", total_days, WASABI_API_LNGSTRINGW( ( total_days == 1 ? IDS_DAY : IDS_DAYS ) ), length / 60 / 60, ( length / 60 ) % 60, length % 60 );
		}
		else
			StringCchPrintfW( str, buf_len, L"%d:%02d:%02d", length / 60 / 60, ( length / 60 ) % 60, length % 60 );
	}
}

void UpdatePlaylistTime(HWND hwndDlg)
{
	wchar_t str[64] = {0}, str2[32] = {0};
	int selitems = playlist_list.GetSelectedCount();

	int seltime = GetSelectedLength(), ttime = GetTotalLength();

	FormatLength(str, seltime, 64);
	FormatLength(str2, ttime, 32);

	wchar_t buf2[128] = {0}, sStr[16] = {0};
	if ( selitems )
		StringCchPrintf( buf2, 128, WASABI_API_LNGSTRINGW( IDS_X_OF_X_SELECTED ), selitems, playlist_list.GetCount(), WASABI_API_LNGSTRINGW_BUF( playlist_list.GetCount() == 1 ? IDS_ITEM : IDS_ITEMS_LOWER, sStr, 16 ), str, str2 );
	else
		StringCchPrintf( buf2, 128, WASABI_API_LNGSTRINGW( IDS_X_SELECTED ), playlist_list.GetCount(), WASABI_API_LNGSTRINGW_BUF( playlist_list.GetCount() == 1 ? IDS_ITEM : IDS_ITEMS_LOWER, sStr, 16 ), str2 );

	SetDlgItemText(hwndDlg, IDC_PLSTATUS, buf2);
}

static wchar_t *BuildFilenameList( int is_all )
{
	wchar_t filename[ MAX_PATH ] = { 0 };

	size_t len = MAX_PATH;
	wchar_t *str = (wchar_t *)calloc( len, sizeof( wchar_t ) );
	size_t sofar = 0;

	int numTracks = playlist_list.GetCount();
	for ( int i = 0; i < numTracks; i++ )
	{
		if ( is_all || playlist_list.GetSelected( i ) )
		{
			if ( currentPlaylist.GetItem( i, filename, MAX_PATH ) )
			{
				int filenameLen = lstrlen( filename ) + 1;
				if ( ( filenameLen + sofar ) > len )
				{
					int newLen = sofar * 2; // add some cushion
					wchar_t *newStr = (wchar_t *)realloc( str, newLen * sizeof( wchar_t ) );
					if ( !newStr )
					{
						newLen = sofar + filenameLen;
						// try the minimum possible size to get this to work
						newStr = (wchar_t *)realloc( str, newLen * sizeof( wchar_t ) );
						if ( !newStr )
						{
							free( str );
							return 0;
						}
					}
					str = newStr;
				}

				lstrcpyn( str + sofar, filename, filenameLen );
				sofar += filenameLen;
			}
		}
	}

	*( str + sofar ) = 0;

	return str;
}

void PlaySelection( int enqueue, int is_all )
{
	if ( !enqueue )
		SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_DELETE );

	int numTracks = playlist_list.GetCount();
	for ( int i = 0; i < numTracks; i++ )
	{
		if ( is_all || playlist_list.GetSelected( i ) )
		{
			const wchar_t *filename = currentPlaylist.ItemName( i );
			if ( filename )
			{
				enqueueFileWithMetaStructW l_enqueue_file;

				std::map<std::wstring, std::wstring> &l_extended_infos = currentPlaylist.entries[ i ]->_extended_infos;

				wa::strings::wa_string l_original_filename( filename );

				if ( !currentPlaylist.entries[ i ]->isLocal() && !l_extended_infos.empty() && l_extended_infos.count( L"ext" ) == 1 && l_original_filename.contains( "://" ) )
				{
					wa::strings::wa_string l_ext( "." );
					l_ext.append( ( *l_extended_infos.find( L"ext" ) ).second );
					l_ext.toUpper();

					wa::strings::wa_string l_filename( WINAMP_REDIRECT_LINK_PROXY_FILE );

					l_filename.append( filename );

					wa::strings::wa_string l_upper_case_filename( filename );
					l_upper_case_filename.toUpper();

					if ( !l_upper_case_filename.contains( l_ext.GetW() ) )
					{
						if ( !l_original_filename.contains( "?" ) )
							l_filename.append( "?" );
						else
							l_filename.append( "&" );

						l_filename.append( "ext=" );
						l_filename.append( l_ext );
					}


					l_enqueue_file.filename = _wcsdup( l_filename.GetW().c_str() );
					l_enqueue_file.ext      = _wcsdup( ( *l_extended_infos.find( L"ext" ) ).second.c_str() );
				}
				else
				{
					l_enqueue_file.filename = filename;
					l_enqueue_file.ext      = NULL;
				}
				

				wa::strings::wa_string l_filename( l_enqueue_file.filename );

				if ( l_filename.contains( "://" ) )
				{
					wsprintfW( _log_message_w, L"The link '%s' will be played!", l_filename.GetW().c_str() );

					LOG_DEBUG( _log_message_w );
				}


				if ( currentPlaylist.IsCached( i ) )
				{
					l_enqueue_file.title  = currentPlaylist.ItemTitle( i );
					plstring_retain( (wchar_t *)l_enqueue_file.title );
					l_enqueue_file.length = currentPlaylist.GetItemLengthMilliseconds( i ) / 1000;
				}
				else
				{
					l_enqueue_file.title = 0;
					l_enqueue_file.length = 0;
				}

				plstring_retain( (wchar_t *)l_enqueue_file.filename );
				SendMessage( plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&l_enqueue_file, IPC_PLAYFILEW_NDE_TITLE );
			}
		}
	}

	if ( !enqueue )
	{
		if ( is_all )
		{
			int pos = playlist_list.GetNextSelected( -1 );
			if ( pos != -1 )
			{
				SendMessage( plugin.hwndWinampParent, WM_WA_IPC, pos, IPC_SETPLAYLISTPOS );
				SendMessage( plugin.hwndWinampParent, WM_COMMAND, 40047, 0 ); // stop button, literally
				SendMessage( plugin.hwndWinampParent, WM_COMMAND, 40045, 0 ); // play button, literally

				return;
			}
		}

		SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_STARTPLAY );
	}
}

int playlist_Load( const wchar_t *playlistFileName )
{
	currentPlaylist.Clear();

	return AGAVE_API_PLAYLISTMANAGER->Load( playlistFileName, &currentPlaylist );
}

LRESULT playlist_cloud_listview( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_NOTIFY )
	{
		LPNMHDR l = (LPNMHDR)lParam;
		switch ( l->code )
		{
			case TTN_SHOW:
			{
				LVHITTESTINFO lvh = {0};
				GetCursorPos(&lvh.pt);
				ScreenToClient(hwnd, &lvh.pt);
				ListView_SubItemHitTest(hwnd, &lvh);

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
				LVHITTESTINFO lvh = {0};
				GetCursorPos(&lvh.pt);
				ScreenToClient(hwnd, &lvh.pt);
				ListView_SubItemHitTest(hwnd, &lvh);

				static wchar_t tt_buf2[256] = {L""};
				if ( cloud_avail && lvh.iItem != -1 && lvh.iSubItem == 1 )
				{
					LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO)lParam;

					static int last_item2 = -1;
					if ( last_item2 == lvh.iItem )
					{
						lpnmtdi->lpszText = tt_buf2;

						return 0;
					}

					wchar_t info[ 16 ] = { 0 };
					currentPlaylist.GetItemExtendedInfo( lvh.iItem, L"cloud_status", info, 16 );

					int status = _wtoi( info );
					if ( status == 4 )
					{
						WASABI_API_LNGSTRINGW_BUF( IDS_UPLOAD_TO_SOURCE, tt_buf2, ARRAYSIZE( tt_buf2 ) );
					}
					else
					{
						winampMediaLibraryPlugin *( *gp )( );
						gp = ( winampMediaLibraryPlugin * ( __cdecl * )( void ) )GetProcAddress( cloud_hinst, "winampGetMediaLibraryPlugin" );
						if ( gp )
						{
							winampMediaLibraryPlugin *mlplugin = gp();
							if ( mlplugin && ( mlplugin->version >= MLHDR_VER_OLD && mlplugin->version <= MLHDR_VER ) )
							{
								// TODO handle case when not in a device
								WASABI_API_LNGSTRINGW_BUF( IDS_TRACK_AVAILABLE, tt_buf2, ARRAYSIZE( tt_buf2 ) );

								wchar_t filepath[ 1024 ] = { 0 };
								currentPlaylist.GetItem( lvh.iItem, filepath, 1024 );

								nx_string_t *out_devicenames = 0;
								size_t num_names = mlplugin->MessageProc( 0x405, (INT_PTR)&filepath, (INT_PTR)&out_devicenames, 0 );
								if ( num_names > 0 )
								{
									for ( size_t i = 0; i < num_names; i++ )
									{
										if ( i > 0 )
											StringCchCatW( tt_buf2, ARRAYSIZE( tt_buf2 ), L", " );

										StringCchCatW( tt_buf2, ARRAYSIZE( tt_buf2 ), out_devicenames[ i ]->string );
									}
								}
								else
								{
									WASABI_API_LNGSTRINGW_BUF( IDS_UPLOAD_TO_SOURCE, tt_buf2, ARRAYSIZE( tt_buf2 ) );
								}

								if ( out_devicenames )
									free( out_devicenames );
							}
						}
					}

					last_item2 = lvh.iItem;
					lpnmtdi->lpszText = tt_buf2;

					// bit of a fiddle but it allows for multi-line tooltips
					//SendMessage(l->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 0);
				}
				else
					return CallWindowProcW((WNDPROC)GetPropW(hwnd, L"cloud_list_proc"), hwnd, uMsg, wParam, lParam);
			}

			return 0;
		}
	}

	return CallWindowProcW((WNDPROC)GetPropW(hwnd, L"cloud_list_proc"), hwnd, uMsg, wParam, lParam);
}

void playlist_UpdateButtonText( HWND hwndDlg, int _enqueuedef )
{
	if ( groupBtn )
	{
		switch ( _enqueuedef )
		{
			case 1:
				SetDlgItemTextW( hwndDlg, IDC_PLAY, view.enqueue );
				customAllowed = FALSE;
				break;

			default:
				// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
				//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
				pluginMessage p = { ML_MSG_VIEW_BUTTON_HOOK_IN_USE, (INT_PTR)_enqueuedef, 0, 0 };

				wchar_t *pszTextW = (wchar_t *)SENDMLIPC( plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p );
				if ( pszTextW && pszTextW[ 0 ] != 0 )
				{
					// set this to be a bit different so we can just use one button and not the
					// mixable one as well (leaving that to prevent messing with the resources)
					SetDlgItemTextW( hwndDlg, IDC_PLAY, pszTextW );
					customAllowed = TRUE;
				}
				else
				{
					SetDlgItemTextW( hwndDlg, IDC_PLAY, view.play );
					customAllowed = FALSE;
				}
				break;
		}
	}
}

static void playlist_Init( HWND hwndDlg, LPARAM lParam )
{
	HACCEL accel = WASABI_API_LOADACCELERATORSW( IDR_VIEW_PL_ACCELERATORS );
	if ( accel )
		WASABI_API_APP->app_addAccelerators( hwndDlg, &accel, 1, TRANSLATE_MODE_CHILD );

	if ( !view.play )
		SENDMLIPC( plugin.hwndLibraryParent, ML_IPC_GET_VIEW_BUTTON_TEXT, (WPARAM)&view );

	opened     = true;
	loaded     = false;
	activeHWND = hwndDlg;
	saveHWND   = GetDlgItem(hwndDlg, IDC_SAVE_PL);

	Changed( false );

	cloud_avail = playlists_CloudAvailable();
	groupBtn    = g_config->ReadInt( L"groupbtn", 1 );
	enqueuedef  = ( g_config->ReadInt( L"enqueuedef", 0 ) == 1 );

	// force create this as it helps resolve some quirks with play / enqueue handling and
	// we cannot guarantee the button being on the dialog resource due to old lang packs.
	if ( !IsWindow( GetDlgItem( hwndDlg, IDC_ENQUEUE ) ) )
	{
		HWND newnd = CreateWindowEx( WS_EX_NOPARENTNOTIFY, L"button", view.enqueue, WS_CHILD | WS_TABSTOP | BS_OWNERDRAW, 0, 0, 0, 0, hwndDlg, (HMENU)IDC_ENQUEUE, plugin.hDllInstance, 0 );
		// make sure we're using an appropriate font for the display (may need to review this...)
		SendMessage( newnd, WM_SETFONT, (WPARAM)SendDlgItemMessage( hwndDlg, IDC_PLAY, WM_GETFONT, 0, 0 ), 0 );
		SetWindowPos( newnd, GetDlgItem( hwndDlg, IDC_PLAY ), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW );
	}

	// v5.66+ - re-use the old predixis parts so the button can be used functionally via ml_enqplay
	//			pass the hwnd, button id and plug-in id so the ml plug-in can check things as needed
	pluginMessage p = { ML_MSG_VIEW_BUTTON_HOOK, (INT_PTR)hwndDlg, (INT_PTR)MAKELONG( IDC_CUSTOM, IDC_ENQUEUE ), (INT_PTR)L"ml_playlists" };
	wchar_t *pszTextW = (wchar_t *)SENDMLIPC( plugin.hwndLibraryParent, ML_IPC_SEND_PLUGIN_MESSAGE, (WPARAM)&p );
	if ( pszTextW && pszTextW[ 0 ] != 0 )
	{
		// set this to be a bit different so we can just use one button and not the
		// mixable one as well (leaving that to prevent messing with the resources)
		customAllowed = TRUE;
		SetDlgItemTextW( hwndDlg, IDC_CUSTOM, pszTextW );
	}
	else
	{
		customAllowed = FALSE;
	}

	// override the button text to save a bit more space
	if ( !g_config->ReadInt( L"pltextbuttons", 0 ) )
	{
		SetDlgItemText( hwndDlg, IDC_ADD, L"+" );
		SetDlgItemText( hwndDlg, IDC_REM, L"\u2212" );
	}

	/* skin dialog */
	MLSKINWINDOW sw = {0};
	sw.skinType   = SKINNEDWND_TYPE_DIALOG;
	sw.style      = SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWS_USESKINFONT;
	sw.hwndToSkin = hwndDlg;
	MLSkinWindow( plugin.hwndLibraryParent, &sw );

	/* skin status bar */
	sw.hwndToSkin = GetDlgItem( hwndDlg, IDC_PLSTATUS );
	sw.skinType   = SKINNEDWND_TYPE_STATIC;
	sw.style      = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
	MLSkinWindow( plugin.hwndLibraryParent, &sw );

	/* skin listview */
	HWND list = sw.hwndToSkin = GetDlgItem(hwndDlg, IDC_PLAYLIST_EDITOR);
	sw.skinType   = SKINNEDWND_TYPE_LISTVIEW;
	sw.style      = SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS | SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS;
	MLSkinWindow( plugin.hwndLibraryParent, &sw );
	MLSkinnedScrollWnd_ShowHorzBar( sw.hwndToSkin, FALSE );

	/* skin buttons */
	sw.skinType   = SKINNEDWND_TYPE_BUTTON;
	sw.style      = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | ( groupBtn ? SWBS_SPLITBUTTON : 0 );
	const int buttonidz[] = { IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM };
	for ( size_t i = 0; i != sizeof( buttonidz ) / sizeof( buttonidz[ 0 ] ); i++ )
	{
		sw.hwndToSkin = GetDlgItem( hwndDlg, buttonidz[ i ] );
		if ( IsWindow( sw.hwndToSkin ) )
			MLSkinWindow( plugin.hwndLibraryParent, &sw );
	}

	/* skin dropdown buttons */
	sw.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWBS_DROPDOWNBUTTON;
	const int buttonids[] = {IDC_BURN, IDC_ADD, IDC_REM, IDC_SEL, IDC_MISC, IDC_LIST};
	for ( size_t i = 0; i != sizeof( buttonids ) / sizeof( buttonids[ 0 ] ); i++ )
	{
		sw.hwndToSkin = GetDlgItem( hwndDlg, buttonids[ i ] );
		if ( IsWindow( sw.hwndToSkin ) )
			MLSkinWindow( plugin.hwndLibraryParent, &sw );
	}

	sw.style      -= SWBS_DROPDOWNBUTTON;
	sw.hwndToSkin  = GetDlgItem( hwndDlg, IDC_SAVE_PL );
	MLSkinWindow( plugin.hwndLibraryParent, &sw );

	// REVIEW: it'd be really nice to pass in a pointer to an ifc_playlist instead...
	// at this point, the main issue is how to delete/release it when we're done
	playlist_guid = tree_to_guid_map[ lParam ];


	{ // scope for lock
		AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );

		PlaylistInfo info( playlist_guid );
		if ( info.Valid() )
		{
			// will check if the playlist file exists and update the view as needed
			const wchar_t *filename = info.GetFilename();
			if ( !PathFileExistsW( filename ) )
			{
				opened = false;

				RECT r      = { 0 };
				HWND status = GetDlgItem( hwndDlg, IDC_PLSTATUS );

				GetWindowRect( hwndDlg, &r );
				MoveWindow( status, 20, 0, r.right - r.left - 40, r.bottom - r.top, FALSE );

				const int ids[] = { IDC_PLAYLIST_EDITOR, IDC_PLAY, IDC_BURN, IDC_ADD, IDC_REM, IDC_SEL, IDC_MISC, IDC_LIST, IDC_SAVE_PL };
				for ( size_t i = 0; i < sizeof( ids ) / sizeof( ids[ 0 ] ); i++ )
					ShowWindow( GetDlgItem( hwndDlg, ids[ i ] ), FALSE );

				// adjust the styles without needing extra resources, etc
				DWORD style = GetWindowLongPtr( status, GWL_STYLE );
				if ( style & SS_ENDELLIPSIS )
					style -= SS_ENDELLIPSIS;

				if ( style & SS_CENTERIMAGE )
					style -= SS_CENTERIMAGE;

				SetWindowLongPtr( status, GWL_STYLE, style | SS_CENTER | 0x2000 );
				wchar_t buf[ 1024 ] = { 0 };
				StringCchPrintfW( buf, 1024, WASABI_API_LNGSTRINGW( IDS_SOURCE_PL_MISSING ), filename );
				SetWindowTextW( status, buf );
			}

			lstrcpynW( currentPlaylistFilename, filename, MAX_PATH );
			playlist_Load( currentPlaylistFilename );

			lstrcpynW( currentPlaylistTitle, info.GetName(), FILETITLE_SIZE );
		}
	}

	SetPropW( hwndDlg, L"TITLE", currentPlaylistTitle );

	playlist_list.setwnd( list );

	playlist_list.AddCol( WASABI_API_LNGSTRINGW( IDS_TITLE ), 400 );

	int width = 27;
	MLCloudColumn_GetWidth( plugin.hwndLibraryParent, &width );
	playlist_list.AddCol( L"", ( cloud_avail ? width : 0 ) );
	playlist_list.AddAutoCol( WASABI_API_LNGSTRINGW( IDS_TIME ) );
	playlist_list.JustifyColumn( 2, LVCFMT_RIGHT );

	MLSkinnedHeader_SetCloudColumn( ListView_GetHeader( playlist_list.getwnd() ), ( cloud_avail ? 1 : -1 ) );

	if (!GetPropW(list, L"cloud_list_proc"))
		SetPropW( list, L"cloud_list_proc", (HANDLE)SetWindowLongPtrW( list, GWLP_WNDPROC, (LONG_PTR)playlist_cloud_listview ) );

	playlist_UpdateButtonText( hwndDlg, enqueuedef == 1 );

	SyncPlaylist();
	SetWindowRedraw( playlist_list.getwnd(), FALSE );
}

static void playlist_Paint( HWND hwndDlg )
{
	int tab[] = { IDC_PLAYLIST_EDITOR | DCW_SUNKENBORDER };
	dialogSkinner.Draw( hwndDlg, tab, ( opened ? 1 : 0 ) );
}

static void AutoSizePlaylistColumns()
{
	playlist_list.AutoSizeColumn( 2 );
	RECT channelRect;
	GetClientRect( playlist_list.getwnd(), &channelRect );
	ListView_SetColumnWidth( playlist_list.getwnd(), 0, channelRect.right - playlist_list.GetColumnWidth( 1 ) - playlist_list.GetColumnWidth( 2 ) );
}

enum
{
	BPM_ECHO_WM_COMMAND = 0x1, // send WM_COMMAND and return value
	BPM_WM_COMMAND      = 0x2, // just send WM_COMMAND
};

BOOL playlist_ButtonPopupMenu( HWND hwndDlg, int buttonId, HMENU menu, int flags = 0 )
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

static void playlist_Burn( HWND hwndDlg )
{
	HMENU blah = CreatePopupMenu();

	sendToIgnoreID = lastActiveID;
	sendTo.AddHere( hwndDlg, blah, ML_TYPE_FILENAMES );

	int x = playlist_ButtonPopupMenu( hwndDlg, IDC_BURN, blah );
	if ( sendTo.WasClicked( x ) )
	{
		int is_all = playlist_list.GetSelectedCount() == 0;
		wchar_t *names = BuildFilenameList( is_all );
		sendTo.SendFilenames( names );
		free( names );
	}

	sendTo.Cleanup();

	sendToIgnoreID = 0;
}

static void playlist_Play( HWND hwndDlg, HWND from, UINT idFrom )
{
	HMENU listMenu = GetSubMenu( g_context_menus2, 0 );
	int   count    = GetMenuItemCount( listMenu );

	if ( count > 2 )
	{
		for ( int i = 2; i < count; i++ )
			DeleteMenu( listMenu, 2, MF_BYPOSITION );
	}

	playlist_ButtonPopupMenu( hwndDlg, idFrom, listMenu, BPM_WM_COMMAND );
	UpdatePlaylistTime( hwndDlg );
}

static void playlist_Sel( HWND hwndDlg, HWND from )
{
	HMENU listMenu = GetSubMenu( GetSubMenu( g_context_menus, 3 ), 1 );
	UINT menuStatus;

	if ( playlist_list.GetNextSelected( -1 ) == -1 )
		menuStatus = MF_BYCOMMAND | MF_GRAYED;
	else
		menuStatus = MF_BYCOMMAND | MF_ENABLED;

	EnableMenuItem( listMenu, IDC_PLAYLIST_INVERT_SELECTION, menuStatus );

	if ( playlist_list.GetCount() > 0 )
		menuStatus = MF_BYCOMMAND | MF_ENABLED;
	else
		menuStatus = MF_BYCOMMAND | MF_GRAYED;

	EnableMenuItem( listMenu, IDC_PLAYLIST_SELECT_ALL, menuStatus );
	playlist_ButtonPopupMenu( hwndDlg, IDC_SEL, listMenu, BPM_WM_COMMAND );
	UpdatePlaylistTime( hwndDlg );
}

static void playlist_Rem( HWND hwndDlg, HWND from )
{
	HMENU listMenu = GetSubMenu( GetSubMenu( g_context_menus, 3 ), 2 );
	UINT menuStatus;
	if ( playlist_list.GetNextSelected( -1 ) == -1 )
		menuStatus = MF_BYCOMMAND | MF_GRAYED;
	else
		menuStatus = MF_BYCOMMAND | MF_ENABLED;

	EnableMenuItem( listMenu, IDC_DELETE, menuStatus );
	EnableMenuItem( listMenu, IDC_CROP, menuStatus );

	if ( playlist_list.GetCount() > 0 )
		menuStatus = MF_BYCOMMAND | MF_ENABLED;
	else
		menuStatus = MF_BYCOMMAND | MF_GRAYED;

	EnableMenuItem( listMenu, IDC_PLAYLIST_REMOVE_DEAD, menuStatus );
	EnableMenuItem( listMenu, IDC_PLAYLIST_REMOVE_ALL, menuStatus );

	playlist_ButtonPopupMenu( hwndDlg, IDC_REM, listMenu, BPM_WM_COMMAND );
	UpdatePlaylistTime( hwndDlg );
}

static void playlist_Add( HWND hwndDlg, HWND from )
{
	HMENU listMenu = GetSubMenu( GetSubMenu( g_context_menus, 3 ), 3 );

	playlist_ButtonPopupMenu( hwndDlg, IDC_ADD, listMenu, BPM_WM_COMMAND );
	UpdatePlaylistTime( hwndDlg );
}

static void playlist_Misc( HWND hwndDlg, HWND from )
{
	HMENU listMenu = GetSubMenu( GetSubMenu( g_context_menus, 3 ), 4 );

	UINT menuStatus;
	if ( playlist_list.GetCount() > 0 )
		menuStatus = MF_BYCOMMAND | MF_ENABLED;
	else
		menuStatus = MF_BYCOMMAND | MF_GRAYED;

	EnableMenuItem( listMenu, IDC_PLAYLIST_RANDOMIZE,     menuStatus );
	EnableMenuItem( listMenu, IDC_PLAYLIST_REVERSE,       menuStatus );
	EnableMenuItem( listMenu, IDC_PLAYLIST_SORT_PATH,     menuStatus );
	EnableMenuItem( listMenu, IDC_PLAYLIST_SORT_FILENAME, menuStatus );
	EnableMenuItem( listMenu, IDC_PLAYLIST_SORT_TITLE,    menuStatus );
	EnableMenuItem( listMenu, IDC_PLAYLIST_RESET_CACHE,   menuStatus );

	playlist_ButtonPopupMenu( hwndDlg, IDC_MISC, listMenu, BPM_WM_COMMAND );

	UpdatePlaylistTime( hwndDlg );
}

static void playlist_List( HWND hwndDlg, HWND from )
{
	sendToIgnoreID = lastActiveID;
	HMENU listMenu = GetSubMenu( GetSubMenu( g_context_menus, 3 ), 5 );
	sendTo.AddHere( hwndDlg, GetSubMenu( listMenu, 1 ), ML_TYPE_FILENAMES );

	int x = playlist_ButtonPopupMenu( hwndDlg, IDC_LIST, listMenu, BPM_ECHO_WM_COMMAND );
	if ( sendTo.WasClicked( x ) )
	{
		wchar_t *names = BuildFilenameList( 1 );
		sendTo.SendFilenames( names );

		free( names );
	}

	sendTo.Cleanup();
	UpdatePlaylistTime( hwndDlg );
	sendToIgnoreID = 0;
}

static void playlist_Command( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	switch ( LOWORD( wParam ) )
	{
		case IDC_BURN:
			playlist_Burn( hwndDlg );
			break;
		case IDC_PLAY:
		case IDC_ENQUEUE:
		{
			if ( HIWORD( wParam ) == MLBN_DROPDOWN )
			{
				playlist_Play( hwndDlg, (HWND)lParam, LOWORD( wParam ) );
				break;
			}
			else
			{
				// if it'l_enqueue_file from an accelerator, use the appropriate setting
				int action;
				if ( LOWORD( wParam ) == IDC_PLAY )
					action = ( HIWORD( wParam ) == 1 ) ? g_config->ReadInt( L"enqueuedef", 0 ) == 1 : 0;
				else
					action = ( HIWORD( wParam ) == 1 ) ? g_config->ReadInt( L"enqueuedef", 0 ) != 1 : 1;

				PlaySelection( action, playlist_list.GetSelectedCount() == 0 );
			}
		}
			break;
		case IDC_SEL:
			playlist_Sel( hwndDlg, (HWND)lParam );
			break;
		case IDC_REM:
			playlist_Rem( hwndDlg, (HWND)lParam );
			break;
		case IDC_ADD:
			playlist_Add( hwndDlg, (HWND)lParam );
			break;
		case IDC_MISC:
			playlist_Misc( hwndDlg, (HWND)lParam );
			break;
		case IDC_LIST:
			playlist_List( hwndDlg, (HWND)lParam );
			break;
		case IDC_DELETE:
			Playlist_DeleteSelected( 1 );
			break;
		case IDC_CROP:
			Playlist_DeleteSelected( 0 );
			break;
		case IDC_PLAYLIST_EXPLOREITEMFOLDER:
			Playlist_FindSelected();
			break;
		case IDC_PLAYLIST_VIEW_FILE_INFO:
			TagEditor( hwndDlg );
			break;
		case IDC_PLAYLIST_EDIT_ENTRY:
			EditEntry( hwndDlg );
			break;
		case IDC_PLAYLIST_DOWNLOAD_ENTRY:
			DownloadSelectedEntries( hwndDlg );
			break;
		case IDC_PLAYLIST_RANDOMIZE:
			AGAVE_API_PLAYLISTMANAGER->Randomize( &currentPlaylist );
			playlist_list.RefreshAll();
			Changed();
			break;
		case IDC_PLAYLIST_REVERSE:
			AGAVE_API_PLAYLISTMANAGER->Reverse( &currentPlaylist );
			playlist_list.RefreshAll();
			Changed();
			break;
		case IDC_PLAYLIST_RESET_CACHE:
			Playlist_ResetSelected();
			break;
		case IDC_PLAYLIST_INVERT_SELECTION: 
			playlist_list.InvertSelection(); 
			break;
		case IDC_PLAYLIST_SELECT_ALL: 
			playlist_list.SelectAll(); 
			break;
		case IDC_ADD_FILES:
			if ( CurrentPlaylist_AddFiles( hwndDlg ) ) Changed();
			SyncPlaylist();
			break;
		case IDC_ADD_DIRECTORY:
			if ( CurrentPlaylist_AddDirectory( hwndDlg ) ) Changed();
			SyncPlaylist();
			break;
		case IDC_ADD_LOCATION:
			if ( CurrentPlaylist_AddLocation( hwndDlg ) ) Changed();
			SyncPlaylist();
			break;
		case IDC_PLAYLIST_SELECT_NONE: 
			playlist_list.UnselectAll();	
			break;
		case IDC_PLAYLIST_REMOVE_DEAD:
			if ( CurrentPlaylist_DeleteMissing() ) Changed();
			SyncPlaylist();
			break;
		case IDC_PLAYLIST_REMOVE_ALL:
			currentPlaylist.Clear();
			Changed();
			SyncPlaylist();
			break;
		case IDC_PLAYLIST_RECYCLE_SELECTED:
			Playlist_RecycleSelected( hwndDlg, 1 );
			break;
		case IDC_PLAYLIST_SORT_PATH:
			currentPlaylist.SortByDirectory();
			Changed();
			playlist_list.RefreshAll();
			break;
		case IDC_PLAYLIST_SORT_FILENAME:
			currentPlaylist.SortByFilename();
			Changed();
			playlist_list.RefreshAll();
			break;
		case IDC_PLAYLIST_SORT_TITLE:
			currentPlaylist.SortByTitle();
			Changed();
			playlist_list.RefreshAll();
			break;
		case IDC_EXPORT_PLAYLIST:
			CurrentPlaylist_Export(hwndDlg);
			break;
		case IDC_IMPORT_PLAYLIST_FROM_FILE:
			if ( currentPlaylist_ImportFromDisk( hwndDlg ) )
				Changed();
			SyncPlaylist();
			break;
		case IDC_IMPORT_WINAMP_PLAYLIST:
			if ( currentPlaylist_ImportFromWinamp( hwndDlg ) ) Changed();
			SyncPlaylist();
			break;
		case ID_PLAYLIST_GENERATE_HTML:
			Playlist_GenerateHtmlPlaylist();
			break;
		case IDC_SAVE_PL:
			if ( changed )
			{
				playlist_Save( hwndDlg );
				Changed( false );
			}
			break;
	}
}

void playlist_Save( HWND hwndDlg )
{
	if ( opened )
	{
		if ( currentPlaylistFilename[ 0 ] )
		{
			if ( AGAVE_API_PLAYLISTMANAGER->Save( currentPlaylistFilename, &currentPlaylist ) == PLAYLISTMANAGER_FAILED )
			{
				wchar_t msg[ 512 ] = { 0 };
				MessageBox( hwndDlg, WASABI_API_LNGSTRINGW_BUF( IDS_PLAYLIST_ERROR, msg, 512 ), WASABI_API_LNGSTRINGW( IDS_PLAYLIST_ERROR_TITLE ), MB_OK | MB_ICONWARNING );
			}
		}

		PlaylistInfo info( playlist_guid );
		info.SetSize( currentPlaylist.GetNumItems() );
		info.SetLength( GetTotalLength() );
		info.IssueSaveCallback();
	}
}

void playlist_SaveGUID( GUID _guid )
{
	if ( playlist_guid == _guid )
	{
		if ( currentPlaylistFilename[ 0 ] )
			AGAVE_API_PLAYLISTMANAGER->Save( currentPlaylistFilename, &currentPlaylist );

		PlaylistInfo info( playlist_guid );
		info.SetSize( currentPlaylist.GetNumItems() );
		info.SetLength( GetTotalLength() );
		info.IssueSaveCallback();
	}
}

void playlist_Destroy( HWND hwndDlg )
{
	WASABI_API_APP->app_removeAccelerators( hwndDlg );

	if ( changed )
		playlist_Save( hwndDlg );

	current_playing[ 0 ]         = 0;
	currentPlaylistFilename[ 0 ] = 0;

	currentPlaylist.Clear();
	playlist_list.setwnd( NULL );

	RemovePropW( hwndDlg, L"TITLE" );

	opened     = false;
	activeHWND = 0;
}

void SwapPlayEnqueueInMenu( HMENU listMenu )
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
			playPos        = i;

			GetMenuItemInfoW( listMenu, i, TRUE, &playItem );
		}
		else if ( id == IDC_ENQUEUE )
		{
			enqueueItem.fMask = MIIM_ID;
			enqueuePos        = i;

			GetMenuItemInfoW( listMenu, i, TRUE, &enqueueItem );
		}
	}

	playItem.wID    = IDC_ENQUEUE;
	enqueueItem.wID = IDC_PLAY;

	SetMenuItemInfoW( listMenu, playPos, TRUE, &playItem );
	SetMenuItemInfoW( listMenu, enqueuePos, TRUE, &enqueueItem );
}

void playlist_ContextMenu( HWND hwndDlg, HWND from, int x, int y )
{
	if ( from != playlist_list.getwnd() )
		return;

	POINT pt = { x,y };

	if ( x == -1 || y == -1 ) // x and y are -1 if the user invoked a shift-f10 popup menu
	{
		RECT channelRect = { 0 };
		int selected = playlist_list.GetNextSelected();
		if ( selected != -1 ) // if something is selected we'll drop the menu from there
		{
			playlist_list.GetItemRect( selected, &channelRect );
			ClientToScreen( hwndDlg, (POINT *)&channelRect );
		}
		else // otherwise we'll drop it from the top-left corner of the listview, adjusting for the header location
		{
			GetWindowRect( hwndDlg, &channelRect );

			HWND hHeader = (HWND)SNDMSG( from, LVM_GETHEADER, 0, 0L );
			RECT headerRect;
			if ( ( WS_VISIBLE & GetWindowLongPtr( hHeader, GWL_STYLE ) ) && GetWindowRect( hHeader, &headerRect ) )
				channelRect.top += ( headerRect.bottom - headerRect.top );
		}

		x = channelRect.left;
		y = channelRect.top;
	}

	HWND hHeader = (HWND)SNDMSG(from, LVM_GETHEADER, 0, 0L);
	RECT headerRect;
	if ( 0 == ( WS_VISIBLE & GetWindowLongPtr( hHeader, GWL_STYLE ) ) || FALSE == GetWindowRect( hHeader, &headerRect ) )
		SetRectEmpty( &headerRect );

	if ( FALSE != PtInRect( &headerRect, pt ) )
		return;

	sendToIgnoreID = lastActiveID;
	HMENU listMenu = GetSubMenu( GetSubMenu( g_context_menus, 3 ), 0 );

	menufucker_t mf = { sizeof( mf ),MENU_MLPLAYLIST,listMenu,0x3000,0x4000,0 };

	UINT menuStatus, do_mf = 0;
	if ( playlist_list.GetNextSelected( -1 ) == -1 )
	{
		menuStatus = MF_BYCOMMAND | MF_GRAYED;
		EnableMenuItem( listMenu, 2, MF_BYPOSITION | MF_GRAYED );
	}
	else
	{
		menuStatus = MF_BYCOMMAND | MF_ENABLED;
		EnableMenuItem( listMenu, 2, MF_BYPOSITION | MF_ENABLED );
		sendTo.AddHere( hwndDlg, GetSubMenu( listMenu, 2 ), ML_TYPE_FILENAMES, 1 );

		mf.extinf.mlplaylist.pl   = &currentPlaylist;
		mf.extinf.mlplaylist.list = playlist_list.getwnd();

		pluginMessage message_build = { SendMessage( plugin.hwndWinampParent, WM_WA_IPC, ( WPARAM ) & "menufucker_build", IPC_REGISTER_WINAMP_IPCMESSAGE ),(intptr_t)&mf,0 };
		SendMessage( plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&message_build, ML_IPC_SEND_PLUGIN_MESSAGE );

		do_mf = 1;
	}

	EnableMenuItem( listMenu, IDC_PLAYLIST_EXPLOREITEMFOLDER, menuStatus );
	EnableMenuItem( listMenu, IDC_PLAYLIST_VIEW_FILE_INFO,    menuStatus );
	EnableMenuItem( listMenu, IDC_PLAYLIST_EDIT_ENTRY,        menuStatus );

	int l_mark = playlist_list.GetSelectionMark();

	if ( l_mark != -1 && !currentPlaylist.entries[ l_mark ]->isLocal() )
		EnableMenuItem( listMenu, IDC_PLAYLIST_DOWNLOAD_ENTRY, menuStatus );
	else
		EnableMenuItem( listMenu, IDC_PLAYLIST_DOWNLOAD_ENTRY, MF_BYCOMMAND | MF_GRAYED );

	EnableMenuItem( listMenu, IDC_DELETE,                     menuStatus );
	EnableMenuItem( listMenu, IDC_CROP,                       menuStatus );

	HMENU cloud_hmenu = 0;
	if ( playlists_CloudAvailable() )
	{
		int mark = playlist_list.GetSelectionMark();
		if ( mark != -1 )
		{
			wchar_t filename[ 1024 ] = { 0 };
			currentPlaylist.entries[ mark ]->GetFilename( filename, 1024 );

			cloud_hmenu = CreatePopupMenu();
			WASABI_API_SYSCB->syscb_issueCallback( api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_GET_CLOUD_STATUS, (intptr_t)&filename, (intptr_t)&cloud_hmenu );
			if ( cloud_hmenu )
			{
				MENUITEMINFOW m = { sizeof( m ), MIIM_TYPE | MIIM_ID | MIIM_SUBMENU, MFT_SEPARATOR, 0 };
				m.wID        = CLOUD_SOURCE_MENUS - 1;
				InsertMenuItemW( listMenu, 3, TRUE, &m );

				wchar_t a[ 100 ] = { 0 };
				m.fType      = MFT_STRING;
				m.dwTypeData = WASABI_API_LNGSTRINGW_BUF( IDS_CLOUD_SOURCES, a, 100 );
				m.wID        = CLOUD_SOURCE_MENUS;
				m.hSubMenu   = cloud_hmenu;

				InsertMenuItemW( listMenu, 4, TRUE, &m );
			}
		}
	}

	UpdateMenuItems(hwndDlg, listMenu);
	int r = Menu_TrackPopup( plugin.hwndLibraryParent, listMenu, TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD, x, y, hwndDlg, NULL );

	if ( r )
		SendMessage( hwndDlg, WM_COMMAND, MAKEWPARAM( r, 0 ), 0 );

	if ( do_mf )
	{
		pluginMessage message_result = { SendMessage( plugin.hwndWinampParent, WM_WA_IPC, ( WPARAM ) & "menufucker_result", IPC_REGISTER_WINAMP_IPCMESSAGE ), (intptr_t)&mf, r, 0 };
		SendMessage( plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&message_result, ML_IPC_SEND_PLUGIN_MESSAGE );
	}

	switch ( r )
	{
		case 0:
			break;
		case IDC_PLAYLIST_EXPLOREITEMFOLDER:
		case IDC_PLAYLIST_VIEW_FILE_INFO:
		case IDC_PLAYLIST_EDIT_ENTRY:
			SendMessage( hwndDlg, WM_NEXTDLGCTL, (WPARAM)from, (LPARAM)TRUE );
			break;
		case IDC_PLAYLIST_DOWNLOAD_ENTRY:

			break;
		default:
			if ( !( menuStatus & MF_GRAYED ) && sendTo.WasClicked( r ) )
			{
				wchar_t *names = BuildFilenameList( 0 );
				sendTo.SendFilenames( names );
				free( names );
			}
			else
			{
				if ( r >= CLOUD_SOURCE_MENUS && r < CLOUD_SOURCE_MENUS_PL_UPPER )	// deals with cloud specific menus
				{
					// 0 = no change
					// 1 = adding to cloud
					// 2 = added locally
					// 4 = removed
					int mode = 0;	// deals with cloud specific menus
					WASABI_API_SYSCB->syscb_issueCallback( api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_PROCESS_CLOUD_STATUS, (intptr_t)r, (intptr_t)&mode );
					// TODO
					/*switch (mode)
					{
						case 1:
							setCloudValue(&itemCache.Items[pnmitem->iItem], L"5");
						break;

						case 2:
							setCloudValue(&itemCache.Items[pnmitem->iItem], L"4");
						break;

						case 4:
							setCloudValue(&itemCache.Items[pnmitem->iItem], L"4");
						break;
					}
					InvalidateRect(resultlist.getwnd(), NULL, TRUE);*/
				}
			}
			break;
	}

	if (!(menuStatus & MF_GRAYED))
		sendTo.Cleanup();

	sendToIgnoreID = 0;

	if ( cloud_hmenu )
	{
		DeleteMenu( listMenu, CLOUD_SOURCE_MENUS - 1, MF_BYCOMMAND );
		DeleteMenu( listMenu, CLOUD_SOURCE_MENUS, MF_BYCOMMAND );
		DestroyMenu( cloud_hmenu );
	}
}

static void playlist_LeftButtonUp( HWND hwndDlg, WPARAM wParam, POINTS pts )
{
	if ( SCROLLDIR_NONE != scrollDirection )
	{
		KillTimer( hwndDlg, SCROLLTIMER_ID );
		scrollDirection = SCROLLDIR_NONE;
	}

	if ( we_are_drag_and_dropping && GetCapture() == hwndDlg )
	{
		ReleaseCapture();

		BOOL handled = FALSE;
		POINT pt;
		POINTSTOPOINT( pt, pts );

		MapWindowPoints( hwndDlg, HWND_DESKTOP, &pt, 1 );
		HWND hTarget = WindowFromPoint( pt );

		if ( hTarget == playlist_list.getwnd() )
		{
			LVHITTESTINFO hitTest = { 0 };
			POINTSTOPOINT( hitTest.pt, pts );
			MapWindowPoints( hwndDlg, playlist_list.getwnd(), &hitTest.pt, 1 );
			ListView_HitTest( playlist_list.getwnd(), &hitTest );

			size_t position = hitTest.iItem;
			if ( ( hitTest.flags & ( LVHT_ONITEM ) ) );
			else if ( hitTest.flags & LVHT_ABOVE )
				position = 0;
			else if ( hitTest.flags & ( LVHT_BELOW | LVHT_NOWHERE ) )
				position = playlist_list.GetCount();

			if ( position != -1 )
			{
				RECT itemRect;
				playlist_list.GetItemRect( position, &itemRect );
				if ( hitTest.pt.y > ( itemRect.bottom + ( itemRect.top - itemRect.bottom ) / 2 ) )
					position++;

				Playlist tempList;
				size_t selected = -1, numDeleted = 0;
				// first, make a temporary list with all the selected items
				// being careful to deal with the discrepancy between the listview and the real playlist
				// as we remove items
				while ( ( selected = playlist_list.GetNextSelected( selected ) ) != -1 )
				{
					tempList.entries.push_back(currentPlaylist.entries.at(selected - numDeleted));
					currentPlaylist.entries.erase(currentPlaylist.entries.begin() + (selected - numDeleted));
					if ((selected - numDeleted) < position)
						position--;

					numDeleted++;
				}
				playlist_list.UnselectAll();
				// if dragging to the end of the playlist, handle things a bit differently from normal
				if ( position > currentPlaylist.entries.size() )
				{
					position--;
					while ( numDeleted-- )
					{
						currentPlaylist.entries.insert(currentPlaylist.entries.end(), tempList.entries.at(0));
						playlist_list.SetSelected(position++); // we want the same filenames to be selected
						tempList.entries.erase(tempList.entries.begin());
					}
				}
				else
				{
					while ( numDeleted-- )
					{
						playlist_list.SetSelected(position); // we want the same filenames to be selected
						currentPlaylist.entries.insert(currentPlaylist.entries.begin() + position, tempList.entries.at(0));
						position++;
						tempList.entries.erase(tempList.entries.begin());
					}
				}

				Changed();
				SyncPlaylist();

				handled = TRUE;
			}
		}

		we_are_drag_and_dropping = 0;

		if ( !handled )
		{
			mlDropItemStruct m = { 0 };
			m.type = ML_TYPE_FILENAMESW;
			m.p    = pt;

			pluginHandleIpcMessage( ML_IPC_HANDLEDRAG, (WPARAM)&m );

			if ( m.result > 0 )
			{
				wchar_t *names = BuildFilenameList( 0 );
				m.flags  = 0;
				m.result = 0;
				m.data   = (void *)names;

				pluginHandleIpcMessage( ML_IPC_HANDLEDROP, (WPARAM)&m );

				free( names );
			}
		}
	}
}

static void CALLBACK playlist_OnScrollTimer( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	HWND hList = GetDlgItem( hwnd, IDC_PLAYLIST_EDITOR );

	if ( SCROLLDIR_NONE == scrollDirection || NULL == hList )
	{
		KillTimer( hwnd, idEvent );

		return;
	}

	RECT rc;
	rc.left = LVIR_BOUNDS;

	if ( SendMessage( hList, LVM_GETITEMRECT, (WPARAM)0, (LPARAM)&rc ) )
	{
		INT height = rc.bottom - rc.top;
		if ( SCROLLDIR_UP == scrollDirection )
			height = -height;

		SendMessage( hList, LVM_SCROLL, 0, (LPARAM)height );
	}

	if ( scrollTimerElapse == scrollDelay )
	{
		static INT scrollInterval = 0;
		if ( 0 == scrollInterval )
			scrollInterval = GetProfileInt( TEXT( "windows" ), TEXT( "DragScrollInterval" ), DD_DEFSCROLLINTERVAL );

		if ( 0 != scrollInterval )
			SetTimer( hwnd, idEvent, scrollTimerElapse, playlist_OnScrollTimer );
		else
			KillTimer( hwnd, idEvent );
	}
}

static INT playlist_GetScrollDirection( HWND hList, POINT pt )
{
	static INT scrollZone = 0;
	if ( 0 == scrollZone )
		scrollZone = GetProfileInt( TEXT( "windows" ), TEXT( "DragScrollInset" ), DD_DEFSCROLLINSET );

	RECT rc, rcTest;
	if ( 0 == scrollZone || !GetClientRect( playlist_list.getwnd(), &rc ) )
		return SCROLLDIR_NONE;

	CopyRect( &rcTest, &rc );

	rcTest.top = rcTest.bottom - scrollZone;
	if ( PtInRect( &rcTest, pt ) )
		return SCROLLDIR_DOWN;

	rcTest.top    = rc.top;
	rcTest.bottom = rcTest.top + scrollZone;

	if ( 0 == ( LVS_NOCOLUMNHEADER & GetWindowLongPtr( hList, GWL_STYLE ) ) )
	{
		HWND hHeader = (HWND)SendMessage( hList, LVM_GETHEADER, 0, 0L );
		if ( NULL != hHeader && 0 != ( WS_VISIBLE & GetWindowLongPtr( hHeader, GWL_STYLE ) ) )
		{
			RECT rcHeader;
			if ( GetWindowRect( hHeader, &rcHeader ) )
			{
				MapWindowPoints( HWND_DESKTOP, hList, ( (POINT *)&rcHeader ) + 1, 1 );

				INT offset = rcHeader.bottom - rc.top;
				if ( 0 != offset )
					OffsetRect( &rcTest, 0, offset );
			}
		}
	}

	if ( PtInRect( &rcTest, pt ) )
		return SCROLLDIR_UP;

	return SCROLLDIR_NONE;
}

static void playlist_MouseMove( HWND hwndDlg, POINTS pts )
{
	if ( we_are_drag_and_dropping && GetCapture() == hwndDlg )
	{
		BOOL handled = FALSE;
		POINT pt;
		POINTSTOPOINT( pt, pts );

		MapWindowPoints( hwndDlg, HWND_DESKTOP, &pt, 1 );
		HWND hTarget = WindowFromPoint( pt );

		INT scroll = SCROLLDIR_NONE;

		if ( hTarget == playlist_list.getwnd() )
		{
			LVHITTESTINFO hitTest = { 0 };
			POINTSTOPOINT( hitTest.pt, pts );
			MapWindowPoints( hwndDlg, playlist_list.getwnd(), &hitTest.pt, 1 );

			int position = ListView_HitTest( playlist_list.getwnd(), &hitTest );

			if ( position != -1 )
			{
				scroll = playlist_GetScrollDirection( playlist_list.getwnd(), hitTest.pt );
				handled = TRUE;
			}
		}

		if ( scroll != scrollDirection )
		{
			if ( SCROLLDIR_NONE == scroll )
			{
				KillTimer( hwndDlg, SCROLLTIMER_ID );
			}
			else
			{
				if ( SCROLLDIR_NONE == scrollDirection )
				{
					if ( 0 == scrollDelay )
						scrollDelay = GetProfileInt( TEXT( "windows" ), TEXT( "DragScrollDelay" ), DD_DEFSCROLLDELAY );

					if ( 0 != scrollDelay )
					{
						scrollTimerElapse = scrollDelay;
						SetTimer( hwndDlg, SCROLLTIMER_ID, scrollTimerElapse, playlist_OnScrollTimer );
					}
				}
			}

			scrollDirection = scroll;
		}
		if ( !handled )
		{
			mlDropItemStruct m = { 0 };
			m.type  = ML_TYPE_FILENAMES;
			m.p     = pt;
			m.flags = 0; //ML_HANDLEDRAG_FLAG_NOCURSOR;

			pluginHandleIpcMessage( ML_IPC_HANDLEDRAG, (WPARAM)&m );
		}
		else
			SetCursor( hDragNDropCursor );
	}
}

void playlist_Reload( bool forced )
{
	if ( opened || forced )
	{
		if ( !opened && forced )
		{
			PostMessage( plugin.hwndLibraryParent, WM_USER + 30, 0, 0 );

			return;
		}

		playlist_Load( currentPlaylistFilename );
		// reverted back to known state so any
		// of our current changes are now gone
		Changed( false );
		SyncPlaylist();
	}
}

void playlist_ReloadGUID( GUID _guid )
{
	if ( playlist_guid == _guid )
		playlist_Reload( true );
}

void playlist_Unload( HWND hwndDlg )
{
	currentPlaylist.Clear();

	currentPlaylistFilename[ 0 ] = 0x0000;

	ListView_SetItemCount( playlist_list.getwnd(), 0 );
	ListView_RedrawItems( playlist_list.getwnd(), 0, 0 );

	UpdatePlaylistTime( hwndDlg );
}

void playlist_DropFiles( HDROP hDrop )
{
	wchar_t temp[ 2048 ] = { 0 };

	int x;
	int y = DragQueryFileW( hDrop, 0xffffffff, temp, 2048 );

	Playlist newPlaylist;
	for ( x = 0; x < y; x++ )
	{
		DragQueryFileW( hDrop, x, temp, 2048 );
		if ( PathIsDirectory( temp ) )
		{
			PlaylistDirectoryCallback dirCallback( mediaLibrary.GetExtensionList() );
			AGAVE_API_PLAYLISTMANAGER->LoadDirectory( temp, &newPlaylist, &dirCallback );
		}
		else
		{
			if ( AGAVE_API_PLAYLISTMANAGER->Load( temp, &newPlaylist ) != PLAYLISTMANAGER_SUCCESS )
			{
				//wchar_t title[400];
				//int length;
				//mediaLibrary.GetFileInfo(temp, title, 400, &length);
				//newPlaylist.AppendWithInfo(temp, title, length*1000);
				newPlaylist.AppendWithInfo( temp, 0, 0 ); // add with NULL info, will be fetched as needed
			}
		}
	}

	LVHITTESTINFO hitTest;
	DragQueryPoint( hDrop, &hitTest.pt );

	ListView_HitTest( playlist_list.getwnd(), &hitTest );

	if ( hitTest.iItem != -1 )
	{
		RECT itemRect;
		playlist_list.GetItemRect( hitTest.iItem, &itemRect );
		if ( hitTest.pt.y > ( itemRect.bottom + ( itemRect.top - itemRect.bottom ) / 2 ) )
		{
			hitTest.iItem++;
			if ( hitTest.iItem >= playlist_list.GetCount() )
				hitTest.iItem = -1;
		}
	}

	if ( hitTest.flags & LVHT_BELOW || hitTest.iItem == -1 )
		currentPlaylist.AppendPlaylist( newPlaylist );
	else
		currentPlaylist.InsertPlaylist( newPlaylist, hitTest.iItem );

	DragFinish( hDrop );
	Changed();
	SyncPlaylist();
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

static void LayoutWindows( HWND hwnd, BOOL fRedraw, BOOL fUpdateAll = FALSE )
{
	static INT controls[] =
	{
		GROUP_STATUSBAR, IDC_PLAY, IDC_ENQUEUE, IDC_CUSTOM, IDC_BURN, IDC_ADD, IDC_REM, IDC_SEL, IDC_MISC, IDC_LIST, IDC_SAVE_PL, IDC_PLSTATUS,
		GROUP_MAIN, IDC_PLAYLIST_EDITOR
	};

	INT index;
	RECT rc, rg, ri;
	LAYOUT layout[ sizeof( controls ) / sizeof( controls[ 0 ] ) ], *pl;
	BOOL skipgroup;
	HRGN rgn = NULL;

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
					if ( opened )
					{
						wchar_t buffer[ 128 ] = { 0 };
						GetDlgItemTextW( hwnd, IDC_PLAY, buffer, ARRAYSIZE( buffer ) );
						LRESULT idealSize = MLSkinnedButton_GetIdealSize( GetDlgItem( hwnd, IDC_PLAY ), buffer );

						SetRect( &rg, rc.left + WASABI_API_APP->getScaleX( 1 ), rc.bottom - WASABI_API_APP->getScaleY( HIWORD( idealSize ) ), rc.right, rc.bottom );
						rc.bottom = rg.top - WASABI_API_APP->getScaleY( 3 );
					}
					else
					{
						SetRect( &rg, rc.left, rc.top, rc.right, rc.bottom );
					}
					break;
				case GROUP_MAIN:
					if ( opened )
						SetRect( &rg, rc.left + WASABI_API_APP->getScaleX( 1 ), rc.top, rc.right, rc.bottom );
					else
						skipgroup = TRUE;
					break;
			}

			continue;
		}

		if (skipgroup)
			continue;

		pl->id   = controls[index];
		pl->hwnd = GetDlgItem(hwnd, pl->id);
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
			case IDC_BURN:
			case IDC_ADD:
			case IDC_SEL:
			case IDC_REM:
			case IDC_MISC:
			case IDC_LIST:
			case IDC_SAVE_PL:
				if ( opened && ( IDC_CUSTOM != pl->id || customAllowed ) )
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

					wchar_t buffer[ 128 ] = { 0 };
					GetWindowText( pl->hwnd, buffer, ARRAYSIZE( buffer ) );

					LRESULT idealSize = MLSkinnedButton_GetIdealSize( pl->hwnd, buffer );
					LONG    width     = LOWORD( idealSize ) + ( pl->id == IDC_PLAY || pl->id == IDC_ENQUEUE || pl->id == IDC_CUSTOM || pl->id == IDC_SAVE_PL ? WASABI_API_APP->getScaleX( 6 ) : 0 );
					
					SETLAYOUTPOS( pl, rg.left, rg.bottom - WASABI_API_APP->getScaleY( HIWORD( idealSize ) ), width, WASABI_API_APP->getScaleY( HIWORD( idealSize ) ) );
					pl->flags |= ( ( rg.right - rg.left ) > width ) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
					if ( SWP_SHOWWINDOW & pl->flags )
						rg.left += ( pl->cx + WASABI_API_APP->getScaleX( 4 ) );
				}
				else
				{
					pl->flags |= SWP_HIDEWINDOW;
				}
				break;
			case IDC_PLSTATUS:
				if ( opened )
				{
					wchar_t buffer[ 128 ] = { 0 };
					GetWindowTextW( pl->hwnd, buffer, ARRAYSIZE( buffer ) );
					LRESULT idealSize = MLSkinnedStatic_GetIdealSize( pl->hwnd, buffer );

					SETLAYOUTPOS( pl, rg.left, rg.bottom - WASABI_API_APP->getScaleY( HIWORD( idealSize ) ), rg.right - rg.left, WASABI_API_APP->getScaleY( HIWORD( idealSize ) ) );
					if ( SWP_SHOWWINDOW & pl->flags )
						rg.top = ( pl->y + pl->cy + WASABI_API_APP->getScaleY( 1 ) );
				}
				else
				{
					SETLAYOUTPOS( pl, rg.left + WASABI_API_APP->getScaleY( 20 ),
								  rg.top + WASABI_API_APP->getScaleY( 1 ),
								  rg.right - rg.left - WASABI_API_APP->getScaleY( 40 ),
								  rg.bottom - WASABI_API_APP->getScaleY( 2 ) );
				}
				break;
			case IDC_PLAYLIST_EDITOR:
				if ( opened )
				{
					pl->flags |= ( rg.top < rg.bottom ) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
					SETLAYOUTPOS( pl, rg.left, rg.top + WASABI_API_APP->getScaleY( 1 ), rg.right - rg.left + WASABI_API_APP->getScaleY( 1 ), ( rg.bottom - rg.top ) - WASABI_API_APP->getScaleY( 2 ) );
				}
				else
				{
					pl->flags |= SWP_HIDEWINDOW;
				}

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
				case IDC_PLAYLIST_EDITOR:
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
		{
			if ( pc->rgn )
				DeleteObject( pc->rgn );
		}
	}

	if ( rgn )
		DeleteObject( rgn );

	ValidateRgn( hwnd, NULL );
}

INT_PTR CALLBACK view_playlistDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR a = dialogSkinner.Handle( hwndDlg, uMsg, wParam, lParam );
	if ( a )
		return a;

	switch (uMsg)
	{
		case WM_INITMENUPOPUP:
			sendTo.InitPopupMenu( wParam );
			return 0;
		case WM_MOUSEMOVE:
			playlist_MouseMove( hwndDlg, MAKEPOINTS( lParam ) );
			return 0;
		case WM_LBUTTONUP:
			playlist_LeftButtonUp( hwndDlg, wParam, MAKEPOINTS( lParam ) );
			return 0;
		case WM_PAINT:
			playlist_Paint( hwndDlg );
			return 0;
		case WM_INITDIALOG:
			playlist_Init( hwndDlg, lParam );
			return TRUE;
		case WM_DESTROY:
			playlist_Destroy( hwndDlg );
			return 0;
		case WM_COMMAND:
			playlist_Command( hwndDlg, wParam, lParam );
			return 0;
		case WM_NOTIFY:
			return playlist_Notify( hwndDlg, wParam, lParam );
		case WM_CONTEXTMENU:
			playlist_ContextMenu( hwndDlg, (HWND)wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
			return 0;
		case WM_ERASEBKGND:
			return 1; //handled by WADlg_DrawChildWindowBorders in WM_PAINT
		case WM_PLAYLIST_RELOAD:
			playlist_Reload();
			return 0;
		case WM_PLAYLIST_UNLOAD:
			playlist_Unload( hwndDlg );
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
		case WM_APP + 103:
		{
			int current = -1;
			if ( wParam )
			{
				// TODO need to get this done in the background so as not to lock up the ui on large playlists
				wchar_t fn[ 1024 ] = { 0 };
				int v = playlist_list.GetCount();
				for ( int x = 0; x < v; x++ )
				{
					currentPlaylist.GetItem( x, fn, 1024 );
					if ( !lstrcmpi( fn, ( !lParam ? (LPWSTR)wParam : current_playing ) ) )
					{
						current = x;
						break;
					}
				}
			}

			PostMessage( playlist_list.getwnd(), WM_ML_IPC, current, ML_IPC_SKINNEDLISTVIEW_SETCURRENT );
		}
			return 0;
		case WM_DROPFILES: playlist_DropFiles((HDROP)wParam); return 0;
		case WM_APP+102:
		{
			if ( cloud_avail )
			{
				int width = 27;
				MLCloudColumn_GetWidth( plugin.hwndLibraryParent, &width );
				playlist_list.SetColumnWidth( 1, width );
				MLSkinnedHeader_SetCloudColumn( ListView_GetHeader( playlist_list.getwnd() ), 1 );
			}
		}
		case WM_APP+100:
			AutoSizePlaylistColumns();
			if ( !loaded )
			{
				loaded = true;
				SetWindowRedraw( playlist_list.getwnd(), TRUE );
			}
			return 0;
		case WM_WINDOWPOSCHANGED:
			if ( ( SWP_NOSIZE | SWP_NOMOVE ) != ( ( SWP_NOSIZE | SWP_NOMOVE ) & ( (WINDOWPOS *)lParam )->flags ) || ( SWP_FRAMECHANGED & ( (WINDOWPOS *)lParam )->flags ) )
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

static wchar_t entryFN[ FILENAME_SIZE ];
static wchar_t titleFN[ FILETITLE_SIZE ];

static INT_PTR CALLBACK entryProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage( hwndDlg, IDC_OLD, EM_SETLIMITTEXT, FILENAME_SIZE, 0 );
			SendDlgItemMessage( hwndDlg, IDC_NEW, EM_SETLIMITTEXT, FILENAME_SIZE, 0 );

			SendDlgItemMessage( hwndDlg, IDC_OLD_TITLE, EM_SETLIMITTEXT, FILETITLE_SIZE, 0 );
			SendDlgItemMessage( hwndDlg, IDC_NEW_TITLE, EM_SETLIMITTEXT, FILETITLE_SIZE, 0 );

			SetDlgItemTextW( hwndDlg, IDC_OLD, entryFN );
			SetDlgItemTextW( hwndDlg, IDC_OLD_TITLE, titleFN );

			SetDlgItemTextW( hwndDlg, IDC_NEW, entryFN );
			SetDlgItemTextW( hwndDlg, IDC_NEW_TITLE, titleFN );

			return TRUE;
		}
		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case IDOK:
					GetDlgItemTextW( hwndDlg, IDC_NEW, entryFN, FILENAME_SIZE );
					GetDlgItemTextW( hwndDlg, IDC_NEW_TITLE, titleFN, FILETITLE_SIZE );
					EndDialog( hwndDlg, 1 );
					return 0;
				case IDCANCEL:
					EndDialog( hwndDlg, 0 );
					return 0;
				case IDC_PLAYLIST_EDIT_ENTRY_BROWSE:
				{
					wchar_t buf[ FILENAME_SIZE ] = { 0 };
					UINT len = GetDlgItemTextW( hwndDlg, IDC_NEW, buf, FILENAME_SIZE ) + 1;
					OPENFILENAME of = { 0 };
					of.lStructSize    = sizeof( OPENFILENAME );
					of.hwndOwner      = hwndDlg;
					of.nMaxFileTitle  = 32;
					of.lpstrFilter    = (wchar_t *)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 1, IPC_GET_EXTLISTW );
					of.nMaxCustFilter = 1024;
					of.lpstrFile      = buf;
					of.nMaxFile       = FILENAME_SIZE;
					of.lpstrTitle     = WASABI_API_LNGSTRINGW( IDS_BROWSE_FOR_PLEDIT_ENTRY );
					of.Flags          = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_ENABLESIZING;

					if ( GetOpenFileName( &of ) )
					{
						wchar_t title[ FILETITLE_SIZE ] = { 0 };
						mediaLibrary.GetFileInfo( buf, title, FILETITLE_SIZE, NULL );

						SetDlgItemTextW( hwndDlg, IDC_NEW_TITLE, title );
						SetDlgItemTextW( hwndDlg, IDC_NEW, buf );
					}

					GlobalFree( (void *)of.lpstrFilter );
					break;
				}
			}

			return 0;
	}

	return 0;
}

void EditEntry( HWND parent )
{
	int i = playlist_list.GetCount();
	while ( i-- )
	{
		if ( playlist_list.GetSelected( i ) )
		{
			currentPlaylist.GetItem( i, entryFN, FILENAME_SIZE );
			currentPlaylist.GetItemTitle( i, titleFN, FILETITLE_SIZE );

			INT_PTR res = WASABI_API_DIALOGBOXW( IDD_EDIT_FN, parent, entryProc );
			if ( res == 1 )
			{
				// if the file has changed, force an update of the item'l_enqueue_file file time since we're not
				// going to allow the view to automatically do such things since with the changes to
				// support editing the title the automatic title lookup after an edit was disabled

				wchar_t filepath[ FILENAME_SIZE ] = { 0 };
				currentPlaylist.GetItem( i, filepath, FILENAME_SIZE );
				if ( lstrcmpi( filepath, entryFN ) )
				{
					int length = -1;
					mediaLibrary.GetFileInfo( entryFN, NULL, NULL, &length );
					currentPlaylist.SetItemLengthMilliseconds( i, length * 1000 );
				}

				currentPlaylist.SetItemFilename( i, entryFN );
				currentPlaylist.SetItemTitle( i, titleFN );

				Changed();
			}
			else
				break;
		}
	}

	SyncPlaylist();
}



class DownloadEntry : public ifc_downloadManagerCallback
{
public:
	DownloadEntry( const wchar_t *p_url, const wchar_t *p_destination_filepath, const wchar_t *p_title, const wchar_t *p_source )
	{
		this->_url                   = _wcsdup( p_url );
		this->_destination_filepath  = _wcsdup( p_destination_filepath );
		this->_title                 = _wcsdup( p_title );
		this->_source                = _wcsdup( p_source );	
	}

	void OnInit( DownloadToken p_token )
	{
		api_httpreceiver *l_http = WAC_API_DOWNLOADMANAGER->GetReceiver( p_token );
		if ( l_http )
		{
			l_http->AllowCompression();
			l_http->addheader( "Accept: */*" );
		}
	}

	void OnConnect( DownloadToken p_token )
	{
		// ---- create file handle
		_hFile = CreateFileW( _destination_filepath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if ( _hFile == INVALID_HANDLE_VALUE )
			this->cancelDownload( p_token );

		this->getContentLength( p_token );
	}

	void OnTick( DownloadToken p_token )
	{
		static bool l_was_renamed = false;

		const char* l_file_extention = WAC_API_DOWNLOADMANAGER->GetExtention(p_token);

		if (l_file_extention && *l_file_extention)
		{
			wa::strings::wa_string l_destination_filepath_old( _destination_filepath );
			wa::strings::wa_string l_destination_filepath_new( _destination_filepath );
			
			if ( !l_destination_filepath_new.contains(l_file_extention) )
			{
				l_destination_filepath_new.append( "." );
				l_destination_filepath_new.append(l_file_extention);

				if ( wa::files::file_exists( _destination_filepath.c_str() ) )
				{
					if ( _hFile != INVALID_HANDLE_VALUE )
						CloseHandle( _hFile );
				}
				
				if ( std::rename( l_destination_filepath_old.GetA().c_str(), l_destination_filepath_new.GetA().c_str() ) )
				{
					_destination_filepath = l_destination_filepath_new.GetW();

					_hFile = CreateFileW( _destination_filepath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
				}
			}
		}
	}

	void OnData( DownloadToken p_token, void *p_data, size_t p_data_size )
	{
		this->getContentLength( p_token );

		if ( _hFile == INVALID_HANDLE_VALUE )
		{
			// ---- get the file extention 
			const char* l_file_extention = WAC_API_DOWNLOADMANAGER->GetExtention(p_token);

			bool l_extension_found = false;

			wa::strings::wa_string l_destination_filepath_old( _destination_filepath );
			wa::strings::wa_string l_destination_filepath_new( _destination_filepath );

			if ( l_file_extention && *l_file_extention )
			{
				if ( !l_destination_filepath_new.contains( l_file_extention ) )
				{
					l_destination_filepath_new.append( "." );
					l_destination_filepath_new.append( l_file_extention );

					l_extension_found = true;
				}
			}
			else
			{
				std::string l_filepath(l_file_extention);

				int l_position = l_filepath.find_first_of( "." );

				if ( l_filepath.size() - l_position > 5 )
				{
					std::string l_url( WAC_API_DOWNLOADMANAGER->GetUrl( p_token ) );

					l_position = l_url.find_last_of( "." );

					if ( l_url.size() - l_position < 5 )
					{
						l_destination_filepath_new.append( "." );
						l_destination_filepath_new.append( l_url.substr( l_position + 1 ) );

						l_extension_found = true;
					}
				}
			}

			if ( l_extension_found )
			{
				if ( wa::files::file_exists( _destination_filepath.c_str() ) )
				{
					if ( _hFile != INVALID_HANDLE_VALUE )
						CloseHandle( _hFile );
				}

				if ( std::rename( l_destination_filepath_old.GetA().c_str(), l_destination_filepath_new.GetA().c_str() ) )
				{
					_destination_filepath = l_destination_filepath_new.GetW();
				}
			}

			// ---- create file handle
			_hFile = CreateFileW( _destination_filepath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			// ---- OnConnect to be removed once dlmgr is fixed
		}

		// ---- OnData
		// ---- if file handle is invalid, then cancel download
		if ( _hFile == INVALID_HANDLE_VALUE )
		{
			this->cancelDownload( p_token );

			return;
		}

		this->_downloaded = (size_t)WAC_API_DOWNLOADMANAGER->GetBytesDownloaded( p_token );

		if ( p_data_size > 0 )
		{
			// ---- hFile is valid handle, and write to disk
			DWORD numWritten = 0;
			WriteFile( _hFile, p_data, (DWORD)p_data_size, &numWritten, FALSE );

			// ---- failed writing the number of datalen characters, cancel download
			if ( numWritten != p_data_size )
			{
				this->cancelDownload( p_token );

				return;
			}
		}
	}

	void OnCancel( DownloadToken p_token )
	{
		if ( _hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( _hFile );
			DeleteFileW( _destination_filepath.c_str() );
		}

		this->resumeNextPendingDownload( p_token );

		this->Release();
	}

	void OnError( DownloadToken p_token, int p_error )
	{
		wsprintfW( _log_message_w, L"An error occurs when downloading the file '%s' ( %d ) !", WAC_API_DOWNLOADMANAGER->GetTitle( p_token ), p_error );

		LOG_ERROR( _log_message_w );

		if ( _hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( _hFile );
			DeleteFileW( _destination_filepath.c_str() );
		}

		this->resumeNextPendingDownload( p_token );

		this->Release();
	}

	void OnFinish( DownloadToken p_token )
	{
		if ( _hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( _hFile );

			_hFile = INVALID_HANDLE_VALUE;
		}

		// Update the playlist with the local path
		const wchar_t *l_file_location  = WAC_API_DOWNLOADMANAGER->GetLocation( p_token );

		wa::strings::wa_string l_new_filename( l_file_location );

		l_new_filename.append( "." );
		l_new_filename.append( WAC_API_DOWNLOADMANAGER->GetExtention( p_token ) );

		WAC_API_DOWNLOADMANAGER->SetLocation( p_token, l_new_filename.GetW().c_str() );

		for ( pl_entry *l_pl_entry : currentPlaylist.entries )
		{
			if ( wcscmp( l_pl_entry->filetitle, WAC_API_DOWNLOADMANAGER->GetTitle( p_token ) ) == 0 )
			{
				l_pl_entry->SetFilename( l_file_location );

				if ( AGAVE_API_PLAYLISTMANAGER->Save( currentPlaylistFilename, &currentPlaylist ) != PLAYLISTMANAGER_FAILED )
					playlist_list.RefreshAll();

				break;
			}
		}


		this->resumeNextPendingDownload( p_token );

		this->Release();
	}


	int GetSource( wchar_t *source, size_t source_cch )
	{
		if ( !this->_source.empty() )
			return wcscpy_s( source, source_cch, this->_source.c_str() );
		else
			return 1;
	}

	int GetTitle( wchar_t *title, size_t title_cch )
	{
		return wcscpy_s( title, title_cch, _title.c_str() );
	}

	int GetLocation( wchar_t *location, size_t location_cch )
	{
		return wcscpy_s( location, location_cch, this->_destination_filepath.c_str() );
	}


	size_t AddRef()
	{
		return _ref_count.fetch_add( 1 );
	}

	size_t Release()
	{
		if ( _ref_count.load() == 0 )
			return _ref_count.load();

		std::size_t r = _ref_count.fetch_sub( 1 );
		if ( r == 0 )
			delete( this );

		return r;
	}


protected:
	RECVS_DISPATCH;

private:
	inline void resumeNextPendingDownload( DownloadToken p_token )
	{
		{
			AutoLock lock( itemsPlaylistQueueLock );

			size_t l_index = 0;
			for ( DownloadToken &l_download_token : plDownloads )
			{
				if ( l_download_token == p_token )
				{
					plDownloads.erase( plDownloads.begin() + l_index );
					break;
				}

				++l_index;
			}
		}

		for ( DownloadToken &l_download_token : plDownloads )
		{
			if ( WAC_API_DOWNLOADMANAGER->IsPending( l_download_token ) )
			{
				WAC_API_DOWNLOADMANAGER->ResumePendingDownload( l_download_token );
				break;
			}
		}
	}

	inline void cancelDownload( DownloadToken p_token )
	{
		wsprintfW( _log_message_w, L"The file '%s' was cancelled !", WAC_API_DOWNLOADMANAGER->GetTitle( p_token ) );

		LOG_ERROR( _log_message_w );

		WAC_API_DOWNLOADMANAGER->CancelDownload( p_token );

		this->resumeNextPendingDownload( p_token );
	}

	inline void getContentLength( DownloadToken p_token )
	{
		if ( p_token == NULL )
			return;

		// ---- retrieve total size
		api_httpreceiver* l_http = WAC_API_DOWNLOADMANAGER->GetReceiver(p_token);
		if ( l_http )
			this->_totalSize = l_http->content_length();
	}


	std::wstring _url;
	std::wstring _destination_filepath;
	std::wstring _title;
	std::wstring _source;

	HANDLE       _hFile      = INVALID_HANDLE_VALUE;
	size_t       _totalSize  = 0;
	size_t       _downloaded = 0;

	std::atomic<std::size_t> _ref_count  = 1;
};

#define CBCLASS DownloadEntry
START_DISPATCH;
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONINIT,      OnInit )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT,   OnConnect )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONTICK,      OnTick )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONDATA,      OnData )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL,    OnCancel )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONFINISH,    OnFinish )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONERROR,     OnError )
CB(  IFC_DOWNLOADMANAGERCALLBACK_GETSOURCE,   GetSource )
CB(  IFC_DOWNLOADMANAGERCALLBACK_GETTITLE,    GetTitle )
CB(  IFC_DOWNLOADMANAGERCALLBACK_GETLOCATION, GetLocation )
CB(  ADDREF, AddRef )
CB(  RELEASE, Release )
END_DISPATCH;
#undef CBCLASS


void DownloadSelectedEntries( HWND parent )
{
	int l_count = playlist_list.GetCount();

	pl_entry *l_pl_entry = NULL;

	for ( int i = 0; i < l_count; ++i )
	{
		if ( playlist_list.GetSelected( i ) )
		{
			if ( !currentPlaylist.IsLocal( i ) )
			{
				currentPlaylist.GetItem( i, entryFN, FILENAME_SIZE );
				currentPlaylist.GetItemTitle( i, titleFN, FILETITLE_SIZE );

				if ( WAC_API_DOWNLOADMANAGER )
				{
					wa::strings::wa_string l_url( entryFN );

					TCHAR szPath[ MAX_PATH ];

					if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_MYMUSIC, NULL, 0, szPath ) ) )
					{
						l_pl_entry = currentPlaylist.entries[ i ];

						bool l_ext_is_find = false;

						std::string l_s_url( l_url.GetA() );

						wa::strings::wa_string l_full_local_filename( szPath );
						l_full_local_filename.append( PLAYLIST_DOWNLOAD_SUBFOLDER );

						if ( !wa::files::folder_exists( l_full_local_filename.GetA().c_str() ) )
							_mkdir( l_full_local_filename.GetA().c_str() );

						wa::strings::wa_string l_title_filename( titleFN );
						l_title_filename.replaceAll( "/", "_" );
						l_title_filename.replaceAll( "\\", "_" );
						l_title_filename.replaceAll( ":", "_" );
						l_title_filename.replaceAll( "*", "_" );
						l_title_filename.replaceAll( "?", "_" );
						l_title_filename.replaceAll( "\"", "_" );
						l_title_filename.replaceAll( "<", "_" );
						l_title_filename.replaceAll( ">", "_" );
						l_title_filename.replaceAll( "|", "_" );

						l_full_local_filename.append( l_title_filename.GetW() );

						l_ext_is_find = ( l_pl_entry->_extended_infos.count( L"ext" ) == 1 );

						if ( l_ext_is_find )
						{
							wa::strings::wa_string l_file_to_verify( l_full_local_filename.GetW() );

							l_file_to_verify.append( "." );
							l_file_to_verify.append( (*l_pl_entry->_extended_infos.find(L"ext")).second );

							if ( wa::files::file_exists( l_file_to_verify.GetW().c_str() ) )
							{
								l_pl_entry->SetFilename( l_file_to_verify.GetW().c_str() );

								if ( AGAVE_API_PLAYLISTMANAGER->Save( currentPlaylistFilename, &currentPlaylist ) != PLAYLISTMANAGER_FAILED )
									playlist_list.RefreshAll();

								return;
							}
						}

						wa::strings::wa_string l_source( l_s_url );

						l_source.replace( "http://",  "" );
						l_source.replace( "https://", "" );

						l_source = l_source.mid( 0, l_source.findFirst( "/" ) );

						wa::strings::wa_string l_redirect_url( WINAMP_REDIRECT_LINK_PROXY_FILE );
						l_redirect_url.append( l_url.GetW() );

						if ( !l_ext_is_find )
							l_redirect_url = l_url.GetW();


						DownloadEntry *_downloadEntry = new DownloadEntry( l_redirect_url.GetW().c_str(), l_full_local_filename.GetW().c_str(), titleFN, l_source.GetW().c_str() );

						if ( plDownloads.size() < SIMULTANEOUS_DOWNLOADS )
						{
							DownloadToken dt = WAC_API_DOWNLOADMANAGER->DownloadEx( l_redirect_url.GetA().c_str(), _downloadEntry, api_downloadManager::DOWNLOADEX_CALLBACK | api_downloadManager::DOWNLOADEX_UI );
							plDownloads.push_back( dt );
						}
						else
						{
							DownloadToken dt = WAC_API_DOWNLOADMANAGER->DownloadEx( l_redirect_url.GetA().c_str(), _downloadEntry, api_downloadManager::DOWNLOADEX_CALLBACK | api_downloadManager::DOWNLOADEX_PENDING | api_downloadManager::DOWNLOADEX_UI );
							plDownloads.push_back( dt );
						}
					}
				}
			}
		}
	}
}