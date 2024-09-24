#include "main.h"
#include "replicant/nu/AutoWide.h"
#include "replicant/nu/AutoLock.h"
#include <algorithm>
#include <strsafe.h>

using namespace Nullsoft::Utility;

TREE_TO_GUID_MAP tree_to_guid_map;

bool FindTreeItem( INT_PTR treeId )
{
	TREE_TO_GUID_MAP::iterator itr = tree_to_guid_map.find( treeId );

	return itr != tree_to_guid_map.end();
}

void MakeTree( PlaylistInfo &playlist )
{
	NAVINSERTSTRUCT nis = { 0 };
	nis.item.cbSize  = sizeof( NAVITEM );
	nis.item.pszText = const_cast<wchar_t *>( AGAVE_API_PLAYLISTS->GetName( playlist.GetIndex() ) );
	nis.item.mask    = NIMF_TEXT | NIMF_IMAGE | NIMF_IMAGESEL | NIMF_ITEMID;
	nis.item.id      = playlist.treeId = 3002 + playlist.GetIndex();
	nis.hParent      = playlistItem;

	if ( playlists_CloudInstalled() )
		nis.item.iImage = nis.item.iSelectedImage = ( !playlist.GetCloud() ? normalimage : cloudImage );
	else
		nis.item.iImage = nis.item.iSelectedImage = normalimage;

	if ( MLNavCtrl_InsertItem( plugin.hwndLibraryParent, &nis ) )
		tree_to_guid_map[ playlist.treeId ] = playlist.playlist_guid;
}

void UpdateTree( PlaylistInfo &playlist, int tree_id )
{
	AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );
	size_t index = playlist.GetIndex();

	MLTREEITEMW updatedItem = { NULL,MLTI_TEXT,NULL };
	updatedItem.id         = tree_id;
	updatedItem.title      = const_cast<wchar_t *>( AGAVE_API_PLAYLISTS->GetName( index ) );
	updatedItem.imageIndex = ( !playlist.GetCloud() ? imgPL : imgCloudPL );

	mediaLibrary.SetTreeItem( updatedItem );

	tree_to_guid_map[ tree_id ] = playlist.playlist_guid;
}

void AddPlaylist( int callback, const wchar_t *title, const wchar_t *filename, bool makeTree, int cloud, size_t numItems, uint64_t length )
{
	AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );
	wchar_t fullFilename[ MAX_PATH ] = { 0 };

	if ( PathIsFileSpecW( filename ) )
		PathCombineW( fullFilename, g_path, filename );
	else
		lstrcpynW( fullFilename, filename, MAX_PATH );

	size_t newIndex = AGAVE_API_PLAYLISTS->AddPlaylist_NoCallback( fullFilename, title );

	// try to get a valid length of the playlist time
	// (important for the playlists view otherwise looks silly with just the number shown)
	if ( !length )
	{
		length = AGAVE_API_PLAYLISTMANAGER->GetLongLengthMilliseconds( fullFilename );
		if ( length > 0 ) length /= 1000;
		else length = 0;
	}

	if ( cloud )
		AGAVE_API_PLAYLISTS->SetInfo( newIndex, api_playlists_cloud, &cloud, sizeof( cloud ) );

	if ( numItems > 0 )
		AGAVE_API_PLAYLISTS->SetInfo( newIndex, api_playlists_itemCount, &numItems, sizeof( numItems ) );

	if ( length > 0 )
		AGAVE_API_PLAYLISTS->SetInfo( newIndex, api_playlists_totalTime, &length, sizeof( length ) );

	if ( callback )
		WASABI_API_SYSCB->syscb_issueCallback( api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_ADDED, newIndex, ( callback - 1 ) );
}

bool LoadOldPlaylists()
{
	bool erased = false;
	int nb = g_config->ReadInt( L"query_num", 0 );
	for ( int i = 0; i < nb; i++ )
	{
		wchar_t qn[ 128 ] = { 0 }, qv[ 128 ] = { 0 }, qm[ 128 ] = { 0 }, qmet[ 128 ] = { 0 };
		StringCchPrintfW( qn,   128, L"query%i_name", i + 1 );
		StringCchPrintfW( qv,   128, L"query%i_val",  i + 1 );
		StringCchPrintfW( qm,   128, L"query%i_mode", i + 1 );
		StringCchPrintfW( qmet, 128, L"query%i_meta", i + 1 );

		int queryMode = g_config->ReadInt( qm, 0 );
		if ( queryMode == 32 )
		{
			wchar_t *name = g_config->ReadString( qn, NULL );
			if ( !name )
				continue;

			name = _wcsdup( name );

			wchar_t *val = g_config->ReadString( qv, NULL );
			if ( val )
				val = _wcsdup( val );

			wchar_t filename[ MAX_PATH ] = { 0 };
			PathCombineW( filename, g_path, val );

			size_t numItems = AGAVE_API_PLAYLISTMANAGER->CountItems( filename );
			AddPlaylist( true, name, filename, ADD_TO_TREE, AddToCloud(), numItems );

			g_config->WriteString( qn,   NULL );
			g_config->WriteString( qv,   NULL );
			g_config->WriteString( qm,   NULL );
			g_config->WriteString( qmet, NULL );

			erased = true;

			free( name );
			free( val );
		}
	}

	return erased;
}

void LoadPlaylists()
{
	bool loadedOld = LoadOldPlaylists();

	AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );
	size_t count = AGAVE_API_PLAYLISTS->GetCount();
	normalimage = mediaLibrary.AddTreeImageBmp( IDB_TREEITEM_PLAYLIST );
	cloudImage  = mediaLibrary.AddTreeImageBmp( IDB_TREEITEM_CLOUD_PLAYLIST );

	for ( size_t i = 0; i != count; i++ )
	{
		PlaylistInfo info( i );
		if ( info.Valid() )
			MakeTree( info );
	}

	if ( loadedOld )
		AGAVE_API_PLAYLISTS->Flush();
}

void UpdatePlaylists()
{
	AutoLockT<api_playlists> lock( AGAVE_API_PLAYLISTS );
	size_t count = AGAVE_API_PLAYLISTS->GetCount();
	normalimage = mediaLibrary.AddTreeImageBmp( IDB_TREEITEM_PLAYLIST );
	cloudImage  = mediaLibrary.AddTreeImageBmp( IDB_TREEITEM_CLOUD_PLAYLIST );

	for ( size_t i = 0; i != count; i++ )
	{
		PlaylistInfo info( i );
		if ( info.Valid() )
			UpdateTree( info, info.treeId );
	}

	if ( IsWindow( currentView ) )
		PostMessage( currentView, WM_APP + 101, 0, 0 );
}

void Playlist_importFromWinamp()
{
	SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_WRITEPLAYLIST );
	const  wchar_t *m3udir = (const wchar_t *)SendMessage( plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETM3UDIRECTORYW );
	wchar_t s[ MAX_PATH ] = { 0 };
	PathCombineW( s, m3udir, L"winamp.m3u8" );

	wchar_t filename[ 1024 + 256 ] = { 0 };
	wchar_t *filenameptr = createPlayListDBFileName( filename );

	wchar_t gs[ MAX_PATH ] = { 0 };
	PathCombineW( gs, g_path, filenameptr );
	size_t numItems = AGAVE_API_PLAYLISTMANAGER->Copy( gs, s );

	AddPlaylist( true, WASABI_API_LNGSTRINGW( IDS_IMPORTED_PLAYLIST ), gs, ADD_TO_TREE, ( !( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) ? AddToCloud() : 0 ), numItems );
	AGAVE_API_PLAYLISTS->Flush();
}