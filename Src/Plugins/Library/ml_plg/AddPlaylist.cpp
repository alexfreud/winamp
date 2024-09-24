#include <shlwapi.h>
#include <strsafe.h>	// Include this last
#include <algorithm>

#include "main.h"
#include "resource.h"

#include "../../General/gen_ml/config.h"
#include "../nu/autolock.h"
#include "../nu/autowide.h"
//#include "../playlist/api_playlists.h"
#include "../nu/MediaLibraryInterface.h"



wchar_t *createPlayListDBFileName( wchar_t *filename ) // filename is ignored but used for temp space, make sure it's 1024+256 chars =)
{
	wchar_t g_path[ MAX_PATH ] = { 0 };
	mediaLibrary.BuildPath( L"Plugins\\ml", g_path, MAX_PATH );

	wchar_t *filenameptr;
	int x = 32;
	for ( ;;)
	{
		GetTempFileNameW( g_path, L"plf", GetTickCount() + x * 5000, filename );

		if ( lstrlenW( filename ) > 4 )
		{
			int length = lstrlenW( filename );							// Get length
			StringCchCopyW( filename + length - 4, length, L".m3u" );		// Add the extension
		}

		HANDLE h = CreateFileW( filename, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_NEW, 0, 0 );
		if ( h != INVALID_HANDLE_VALUE )
		{
			filenameptr = filename + lstrlenW( g_path ) + 1;
			CloseHandle( h );
			break;
		}
		if ( ++x > 4096 )
		{
			filenameptr = L"error.m3u";
			break;
		}
	}

	// Cleanup

	return filenameptr;			// return pointer to just the base filename
}

void CreateAndAddPlaylist( const wchar_t *name )
{
	wchar_t filename[ MAX_PATH ] = { 0 }, dir[ MAX_PATH ] = { 0 };
	GetTempPathW( MAX_PATH, dir );
	GetTempFileNameW( dir, L"ml_playlist", 0, filename );
	StringCchPrintfW( filename, MAX_PATH, L"%s.m3u8", filename );

	int result = -1;

	// See if we want to include the seed tracks in our playlist
	if ( useSeed == TRUE )
	{
		seedPlaylist.AppendPlaylist( currentPlaylist );
		result = AGAVE_API_PLAYLISTMGR->Save( filename, &seedPlaylist );		// Save the seed playlist contents since we appended the regular playlist to it
	}
	else
		result = AGAVE_API_PLAYLISTMGR->Save( filename, &currentPlaylist );	// Save the current playlist contents to the file so they will be in ML view

	if ( result == PLAYLISTMANAGER_SUCCESS )
	{
		mlAddPlaylist p = { sizeof( p ),name,filename, PL_FLAGS_IMPORT,-1,-1 };
		SendMessage( plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&p, ML_IPC_PLAYLIST_ADD );
		DeleteFileW( filename );
	}
}

void TimeStamp( wchar_t *buf, int cch )
{
	wchar_t time_str[ 32 ] = { 0 }, date_str[ 32 ] = { 0 };

	GetDateFormat( 0, 0, 0, 0, date_str, 32 );
	GetTimeFormat( 0, 0, 0, 0, time_str, 32 );

	StringCchPrintfW( buf, cch, L"%s - %s", date_str, time_str );
}

INT_PTR CALLBACK AddPlaylistDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	Playlist *seedPlaylist = (Playlist *)lParam;

	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			//Add the default name here:
			wchar_t default_pl_name[ 1024 ] = { 0 }, timestamp_t[ 64 ] = { 0 }, title[ 256 ] = { 0 };
			TimeStamp( timestamp_t, 64 );				// Get the timestamp

			if ( seedPlaylist->entries[ 0 ] )			// Only grab the seed track if it is actually there
				seedPlaylist->entries[ 0 ]->GetTitle( title, 256 );

			StringCchPrintf( default_pl_name, 1024, L"%s'%s' @ %s", WASABI_API_LNGSTRINGW( IDS_PL_NAME_PREFIX ), title, timestamp_t );
			SetDlgItemText( hwndDlg, IDC_EDIT_NAME, default_pl_name );

			PostMessage( hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem( hwndDlg, IDC_EDIT_NAME ), TRUE );
			break;
		}
		case WM_COMMAND:
			switch ( LOWORD( wParam ) )
			{
				case IDOK:			// On OK create the playlist
				{
					wchar_t name[ 256 ] = { 0 };
					GetDlgItemText( hwndDlg, IDC_EDIT_NAME, name, 255 );
					if ( !name[ 0 ] )		// Error if a valid name is not provided
					{
						wchar_t titleStr[ 32 ] = { 0 };
						MessageBox( hwndDlg, WASABI_API_LNGSTRINGW( IDS_ENTER_A_NAME ),
									WASABI_API_LNGSTRINGW_BUF( IDS_ERROR, titleStr, 32 ), MB_OK );
						break;
					}

					CreateAndAddPlaylist( name );
					AGAVE_API_PLAYLISTS->Flush();

					//lParam = IDOK;					// Set the code to ok as a return to the calling function 
					EndDialog( hwndDlg, IDOK );
				}
				break;
				case IDCANCEL:
					//lParam = IDCANCEL;					// Set the code to cancel as a return to the calling function 
					EndDialog( hwndDlg, IDCANCEL );
					break;
			}
			break;
	}
	return FALSE;
};