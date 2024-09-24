#include <algorithm>
#include "playlists.h"
#include "api__playlist.h"
#include "PlaylistsXML.h"
#include <shlwapi.h>
#include <limits.h>
#include <strsafe.h>
#pragma comment(lib, "Rpcrt4")
using namespace Nullsoft::Utility;

/*
benski> Notes to maintainers
be sure to call DelayLoad() before doing anything.
This is mainly done because the XML parsing service isn't guaranteed to be registered before this service.
It also improves load time.
*/

/* --------------------------------------------- */

PlaylistInfo::PlaylistInfo()
{
	filename[0] = 0;
	title[0]    = 0;
	length      = 0;
	numItems    = 0;
	iTunesID    = 0;
	cloud       = 0;

	UuidCreate(&guid);
}

PlaylistInfo::PlaylistInfo( const wchar_t *_filename, const wchar_t *_title, GUID playlist_guid )
{
	StringCbCopyW( filename, sizeof( filename ), _filename );
	if ( _title )
		StringCbCopyW( title, sizeof( title ), _title );
	else
		title[ 0 ] = 0;

	length   = 0;
	numItems = 0;

	if ( playlist_guid == INVALID_GUID )
		UuidCreate( &guid );
	else
		guid = playlist_guid;

	iTunesID = 0;
	cloud    = 0;
}

PlaylistInfo::PlaylistInfo( const PlaylistInfo &copy )
{
	StringCbCopyW( filename, sizeof( filename ), copy.filename );
	StringCbCopyW( title, sizeof( title ), copy.title );

	length   = copy.length;
	numItems = copy.numItems;
	guid     = copy.guid;
	iTunesID = copy.iTunesID;
	cloud    = copy.cloud;
}

/* --------------------------------------------- */
Playlists::Playlists()
{
	iterator    = 0;
	triedLoaded = false;
	loaded      = false;
	dirty       = false;
}

bool Playlists::DelayLoad()
{
	if ( triedLoaded )
		return loaded;

	PlaylistsXML loader( this );

	const wchar_t *g_path                           = WASABI_API_APP->path_getUserSettingsPath();
	wchar_t        playlistsFilename[ MAX_PATH ]    = { 0 };
	wchar_t        oldPlaylistsFilename[ MAX_PATH ] = { 0 };
	wchar_t        newPlaylistsFolder[ MAX_PATH ]   = { 0 };

	PathCombineW( playlistsFilename, g_path, L"plugins" );
	PathAppendW( playlistsFilename, L"ml" );
	PathAppendW( playlistsFilename, L"playlists" );
	CreateDirectoryW( playlistsFilename, NULL );
	lstrcpynW( newPlaylistsFolder, playlistsFilename, MAX_PATH );
	PathAppendW( playlistsFilename, L"playlists.xml" );

	PathCombineW( oldPlaylistsFilename, g_path, L"plugins" );
	PathAppendW( oldPlaylistsFilename, L"ml" );
	PathAppendW( oldPlaylistsFilename, L"playlists.xml" );

	bool migrated = false;
	if ( PathFileExistsW( oldPlaylistsFilename ) && !PathFileExistsW( playlistsFilename ) )
	{
		if ( MoveFileW( oldPlaylistsFilename, playlistsFilename ) )
		{
			migrated = true;
			PathRemoveFileSpecW( oldPlaylistsFilename );
		}
	}

	switch ( loader.LoadFile( playlistsFilename ) )
	{
		case PLAYLISTSXML_SUCCESS:
			loaded = true;
			triedLoaded = true;
			if ( AGAVE_API_STATS )
				AGAVE_API_STATS->SetStat( api_stats::PLAYLIST_COUNT, (int)playlists.size() );

			if ( playlists.size() && migrated )
			{
				for ( PlaylistInfo l_playlist : playlists )
				{
					wchar_t path[ MAX_PATH ] = { 0 }, file[ MAX_PATH ] = { 0 };
					lstrcpynW( file, l_playlist.filename, MAX_PATH );
					PathStripPathW( file );
					PathCombineW( path, oldPlaylistsFilename, file );
					if ( PathFileExistsW( path ) )
					{
						wchar_t new_path[ MAX_PATH ] = { 0 };
						PathCombineW( new_path, newPlaylistsFolder, file );
						MoveFileW( path, new_path );
					}
				}
				dirty = true;
				Flush();
			}

			break;
		case PLAYLISTSXML_NO_PARSER:
			// if there's XML parser, we'll try again on the off-chance it eventually gets loaded (we might still be in the midst of loading the w5s/wac components)
			break;
		default:
			loaded = true;
			triedLoaded = true;
			break;
	}

	return loaded;
}

void Playlists::Lock()
{
	playlistsGuard.Lock();
}

void Playlists::Unlock()
{
	playlistsGuard.Unlock();
}

size_t Playlists::GetIterator()
{
	return iterator;
}

static void WriteEscaped( FILE *fp, const wchar_t *str )
{
	// TODO: for speed optimization,
	// we should wait until we hit a special character
	// and write out everything else so before it,
	// like how ASX loader does it
	while ( str && *str )
	{
		switch ( *str )
		{
			case L'&':
				fputws( L"&amp;", fp );
				break;
			case L'>':
				fputws( L"&gt;", fp );
				break;
			case L'<':
				fputws( L"&lt;", fp );
				break;
			case L'\'':
				fputws( L"&apos;", fp );
				break;
			case L'\"':
				fputws( L"&quot;", fp );
				break;
			default:
				fputwc( *str, fp );
				break;
		}

		// write out the whole UTF-16 character
		wchar_t *next = CharNextW( str );
		while ( ++str != next )
			fputwc( *str, fp );
	}
}

bool TitleSortAsc( PlaylistInfo &item1, PlaylistInfo &item2 )
{
	int comp = CompareStringW( LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_IGNOREWIDTH, item1.title, -1, item2.title, -1 );

	return comp == CSTR_LESS_THAN;
}

bool TitleSortDesc( PlaylistInfo &item1, PlaylistInfo &item2 )
{
	int comp = CompareStringW( LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_IGNOREWIDTH, item1.title, -1, item2.title, -1 );

	return comp == CSTR_GREATER_THAN;
}

bool NumberOfEntrySortAsc( PlaylistInfo &item1, PlaylistInfo &item2 )
{
	return !!( item1.numItems < item2.numItems );
}

bool NumberOfEntrySortDesc( PlaylistInfo &item1, PlaylistInfo &item2 )
{
	return !!( item1.numItems > item2.numItems );
}

int Playlists::Sort( size_t sort_type )
{
	if ( !DelayLoad() )
		return 0;

	int sorted = 1;

	switch ( sort_type )
	{
		case SORT_TITLE_ASCENDING:
			std::sort( playlists.begin(), playlists.end(), TitleSortAsc );
			break;
		case SORT_TITLE_DESCENDING:
			std::sort( playlists.begin(), playlists.end(), TitleSortDesc );
			break;
		case SORT_NUMBER_ASCENDING:
			std::sort( playlists.begin(), playlists.end(), NumberOfEntrySortAsc );
			break;
		case SORT_NUMBER_DESCENDING:
			std::sort( playlists.begin(), playlists.end(), NumberOfEntrySortDesc );
			break;
		default:
			sorted = 0;
			break;
	}

	dirty = true;

	if ( sorted )
		Flush();

	return sorted;
}

void Playlists::Flush()
{
	AutoLockT<Playlists> lock( this );

	if ( !triedLoaded && !loaded )  // if the playlists.xml file was never even attempted to be loaded, don't overwrite
		return;

	if ( !dirty )	// if we've not seen any changes then no need to re-save
		return;

	const wchar_t *g_path = WASABI_API_APP->path_getUserSettingsPath();

	wchar_t rootPath[ MAX_PATH ]                = { 0 };
	wchar_t playlistsBackupFilename[ MAX_PATH ] = { 0 };
	wchar_t playlistsDestination[ MAX_PATH ]    = { 0 };

	PathCombineW( rootPath, g_path, L"plugins" );
	CreateDirectoryW( rootPath, NULL );
	PathAppendW( rootPath, L"ml" );
	CreateDirectoryW( rootPath, NULL );
	PathAppendW( rootPath, L"playlists" );
	CreateDirectoryW( rootPath, NULL );

	int g_path_size = wcslen( rootPath );
	PathCombineW( playlistsBackupFilename, rootPath, L"playlists.xml.backup" );
	PathCombineW( playlistsDestination, rootPath, L"playlists.xml" );

	CopyFileW( playlistsDestination, playlistsBackupFilename, FALSE );

	FILE *fp = _wfopen( playlistsDestination, L"wb" );
	if ( !fp ) // bah
	{
		dirty = false;
		return;
	}

	fseek( fp, 0, SEEK_SET );
	fputwc( L'\xFEFF', fp );
	fwprintf( fp, L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>" );
	fwprintf( fp, L"<playlists playlists=\"%u\">", (unsigned int)playlists.size() );

	if ( AGAVE_API_STATS )
		AGAVE_API_STATS->SetStat( api_stats::PLAYLIST_COUNT, (int)playlists.size() );

	for ( PlaylistInfo &l_play_list_info : playlists )
	{
		fputws( L"<playlist filename=\"", fp );
		const wchar_t *fn = l_play_list_info.filename;

		if ( !_wcsnicmp( rootPath, fn, g_path_size ) )
		{
			fn += g_path_size;

			if ( *fn == L'\\' )
				++fn;
		}

		WriteEscaped( fp, fn );

		fputws( L"\" title=\"", fp );
		WriteEscaped( fp, l_play_list_info.title );

		GUID guid = l_play_list_info.guid;

		fwprintf( fp, L"\" id=\"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\"",
				  (int)guid.Data1, (int)guid.Data2, (int)guid.Data3,
				  (int)guid.Data4[ 0 ], (int)guid.Data4[ 1 ],
				  (int)guid.Data4[ 2 ], (int)guid.Data4[ 3 ],
				  (int)guid.Data4[ 4 ], (int)guid.Data4[ 5 ],
				  (int)guid.Data4[ 6 ], (int)guid.Data4[ 7 ] );

		fwprintf( fp, L" songs=\"%u\" seconds=\"%u\"", l_play_list_info.numItems, l_play_list_info.length );

		if ( l_play_list_info.iTunesID )
			fwprintf( fp, L" iTunesID=\"%I64u\"", l_play_list_info.iTunesID );

		if ( l_play_list_info.cloud )
			fwprintf( fp, L" cloud=\"1\"" );

		fwprintf( fp, L"/>" );
	}

	fwprintf( fp, L"</playlists>" );
	fclose( fp );
	dirty = false;
}

size_t Playlists::GetCount()
{
	DelayLoad();

	return playlists.size();
}

const wchar_t *Playlists::GetFilename( size_t index )
{
	if ( !DelayLoad() || index >= playlists.size() )
		return 0;

	return playlists[ index ].filename;
}

const wchar_t *Playlists::GetName( size_t index )
{
	if ( !DelayLoad() || index >= playlists.size() )
		return 0;

	return playlists[ index ].title;
}

GUID Playlists::GetGUID( size_t index )
{
	if ( !DelayLoad() || index >= playlists.size() )
		return INVALID_GUID;

	return playlists[ index ].guid;
}

int Playlists::GetPosition( GUID playlist_guid, size_t *index )
{
	if ( !DelayLoad() )
		return API_PLAYLISTS_UNABLE_TO_LOAD_PLAYLISTS;

	size_t indexCount = 0;
	for ( PlaylistInfo &l_play_list_info : playlists )
	{
		if ( l_play_list_info.guid == playlist_guid )
		{
			*index = indexCount;
			return API_PLAYLISTS_SUCCESS;
		}

		++indexCount;
	}

	return API_PLAYLISTS_FAILURE;
}

template <class val_t>
static int GetWithSize( void *data, size_t dataLen, val_t value )
{
	switch ( dataLen )
	{
		case 1:
		{
			if ( value > _UI8_MAX ) // check for overflow
				return API_PLAYLISTS_BAD_SIZE;

			*(uint8_t *)data = (uint8_t)value;

			return API_PLAYLISTS_SUCCESS;
		}
		case 2:
		{
			if ( value > _UI16_MAX ) // check for overflow
				return API_PLAYLISTS_BAD_SIZE;

			*(uint16_t *)data = (uint16_t)value;

			return API_PLAYLISTS_SUCCESS;
		}
		case 4:
		{
			if ( value > _UI32_MAX )
				return API_PLAYLISTS_BAD_SIZE;

			*(uint32_t *)data = (uint32_t)value;

			return API_PLAYLISTS_SUCCESS;
		}
		case 8:
		{
			if ( value > _UI64_MAX )
				return API_PLAYLISTS_BAD_SIZE;

			*(uint64_t *)data = (uint64_t)value;

			return API_PLAYLISTS_SUCCESS;
		}
	}

	return API_PLAYLISTS_BAD_SIZE;
}

template <class val_t>
static int SetWithSize( void *data, size_t dataLen, val_t *value )
{
	switch ( dataLen )
	{
		case 1:
		{
			*value = ( val_t ) * (uint8_t *)data;
			return API_PLAYLISTS_SUCCESS;
		}
		case 2:
		{
			*value = ( val_t ) * (uint16_t *)data;
			return API_PLAYLISTS_SUCCESS;
		}
		case 4:
		{
			*value = ( val_t ) * (uint32_t *)data;
			return API_PLAYLISTS_SUCCESS;
		}
		case 8:
		{
			*value = ( val_t ) * (uint64_t *)data;
			return API_PLAYLISTS_SUCCESS;
		}
	}

	return API_PLAYLISTS_BAD_SIZE;
}

int Playlists::GetInfo( size_t index, GUID info, void *data, size_t dataLen )
{
	if ( !DelayLoad() )
		return API_PLAYLISTS_UNABLE_TO_LOAD_PLAYLISTS;

	if ( index >= playlists.size() )
		return API_PLAYLISTS_INVALID_INDEX;

	if ( info == api_playlists_itemCount )
		return GetWithSize( data, dataLen, playlists[ index ].numItems );
	else if ( info == api_playlists_totalTime )
		return GetWithSize( data, dataLen, playlists[ index ].length );
	else if ( info == api_playlists_iTunesID )
		return GetWithSize( data, dataLen, playlists[ index ].iTunesID );
	else if ( info == api_playlists_cloud )
		return GetWithSize( data, dataLen, playlists[ index ].cloud );

	return API_PLAYLISTS_UNKNOWN_INFO_GUID;
}

int Playlists::MoveBefore( size_t index1, size_t index2 )
{
	if ( !DelayLoad() )
		return API_PLAYLISTS_UNABLE_TO_LOAD_PLAYLISTS;

	if ( index1 >= playlists.size() )
		return API_PLAYLISTS_INVALID_INDEX;

	PlaylistInfo copy = playlists[ index1 ];
	if ( index2 >= playlists.size() )
	{
		playlists.push_back( copy );
		playlists.erase(playlists.begin() + index1 );
	}
	else
	{
		playlists.insert(playlists.begin() + index2, copy );
		if ( index1 >= index2 )
			index1++;
		playlists.erase(playlists.begin() + index1 );
	}

	dirty = true;
	++iterator;

	return API_PLAYLISTS_SUCCESS;
}

size_t Playlists::AddPlaylist( const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid )
{
	if ( !DelayLoad() )
		return -1;

	AutoLockT<Playlists> lock( this );

	if ( playlist_guid != INVALID_GUID )
	{
		for ( size_t index = 0; index < playlists.size(); index++ )
		{
			if ( playlists[ index ].guid == playlist_guid )
			{
				if ( lstrcmpiW( playlists[ index ].title, playlistName ) )
				{
					dirty = true;
					StringCbCopyW( playlists[ index ].title, sizeof( playlists[ index ].title ), playlistName );
					WASABI_API_SYSCB->syscb_issueCallback( api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_RENAMED, index, 0 );
				}

				return -2;
			}
		}
	}

	size_t newIndex = AddPlaylist_NoCallback( filename, playlistName, playlist_guid );
	WASABI_API_SYSCB->syscb_issueCallback( api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_ADDED, newIndex, 0 );

	return newIndex;
}

size_t Playlists::AddCloudPlaylist( const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid )
{
	if ( !DelayLoad() )
		return -1;

	AutoLockT<Playlists> lock( this );

	if ( playlist_guid != INVALID_GUID )
	{
		for ( size_t index = 0; index < playlists.size(); index++ )
		{
			if ( playlists[ index ].guid == playlist_guid )
			{
				// we make sure that this playlist has a 'cloud' flag
				// as without it, our detection of thing isn't ideal.
				if ( !playlists[ index ].cloud )
					playlists[ index ].cloud = 1;

				if ( lstrcmpW( playlists[ index ].title, playlistName ) )
				{
					dirty = true;
					StringCbCopyW( playlists[ index ].title, sizeof( playlists[ index ].title ), playlistName );
					WASABI_API_SYSCB->syscb_issueCallback( api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_RENAMED, index, 0 );
				}

				return -2;
			}
		}
	}

	size_t newIndex = AddPlaylist_NoCallback( filename, playlistName, playlist_guid );
	int cloud = 1;
	SetInfo( newIndex, api_playlists_cloud, &cloud, sizeof( cloud ) );
	WASABI_API_SYSCB->syscb_issueCallback( api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_ADDED, newIndex, 0 );

	return newIndex;
}

size_t Playlists::AddPlaylist_internal( const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid, size_t numItems, size_t length, uint64_t iTunesID, size_t cloud )
{
	PlaylistInfo newPlaylist( filename, playlistName, playlist_guid );
	newPlaylist.numItems = (int)numItems;
	newPlaylist.length   = (int)length;
	newPlaylist.iTunesID = iTunesID;
	newPlaylist.cloud    = (int)cloud;

	playlists.push_back( newPlaylist );
	size_t newIndex = playlists.size() - 1;

	return newIndex;
}

size_t Playlists::AddPlaylist_NoCallback( const wchar_t *filename, const wchar_t *playlistName, GUID playlist_guid )
{
	AutoLockT<Playlists> lock( this );
	PlaylistInfo newPlaylist( filename, playlistName, playlist_guid );

	dirty = true;
	playlists.push_back( newPlaylist );

	++iterator;

	size_t newIndex = playlists.size() - 1;

	return newIndex;
}

int Playlists::SetGUID( size_t index, GUID playlist_guid )
{
	if ( !DelayLoad() )
		return API_PLAYLISTS_UNABLE_TO_LOAD_PLAYLISTS;

	if ( index >= playlists.size() )
		return API_PLAYLISTS_INVALID_INDEX;

	dirty = true;
	playlists[ index ].guid = playlist_guid;

	return API_PLAYLISTS_SUCCESS;
}

int Playlists::RenamePlaylist( size_t index, const wchar_t *name )
{
	if ( !DelayLoad() )
		return API_PLAYLISTS_UNABLE_TO_LOAD_PLAYLISTS;

	if ( index >= playlists.size() )
		return API_PLAYLISTS_INVALID_INDEX;

	dirty = true;

	if ( lstrcmpW( playlists[ index ].title, name ) )
	{
		StringCbCopyW( playlists[ index ].title, sizeof( playlists[ index ].title ), name );
		WASABI_API_SYSCB->syscb_issueCallback( api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_RENAMED, index, 0 );
	}

	return API_PLAYLISTS_SUCCESS;
}

int Playlists::MovePlaylist( size_t index, const wchar_t *filename )
{
	if ( !DelayLoad() )
		return API_PLAYLISTS_UNABLE_TO_LOAD_PLAYLISTS;

	if ( index >= playlists.size() )
		return API_PLAYLISTS_INVALID_INDEX;

	dirty = true;
	StringCbCopyW( playlists[ index ].filename, sizeof( playlists[ index ].filename ), filename );
	iterator++;

	return API_PLAYLISTS_SUCCESS;
}

int Playlists::SetInfo( size_t index, GUID info, void *data, size_t dataLen )
{
	if ( !DelayLoad() )
		return API_PLAYLISTS_UNABLE_TO_LOAD_PLAYLISTS;

	if ( index >= playlists.size() )
		return API_PLAYLISTS_INVALID_INDEX;

	dirty = true;
	if ( info == api_playlists_itemCount )
		return SetWithSize( data, dataLen, &playlists[ index ].numItems );
	else if ( info == api_playlists_totalTime )
		return SetWithSize( data, dataLen, &playlists[ index ].length );
	else if ( info == api_playlists_iTunesID )
		return SetWithSize( data, dataLen, &playlists[ index ].iTunesID );
	else if ( info == api_playlists_cloud )
		return SetWithSize( data, dataLen, &playlists[ index ].cloud );

	dirty = false;

	return API_PLAYLISTS_UNKNOWN_INFO_GUID;
}

int Playlists::RemovePlaylist( size_t index )
{
	if ( !DelayLoad() )
		return API_PLAYLISTS_UNABLE_TO_LOAD_PLAYLISTS;

	if ( index >= playlists.size() )
		return API_PLAYLISTS_INVALID_INDEX;

	dirty = true;
	WASABI_API_SYSCB->syscb_issueCallback( api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_REMOVED_PRE, index, 0 );
	playlists.erase(playlists.begin() + index );
	iterator++;
	WASABI_API_SYSCB->syscb_issueCallback( api_playlists::SYSCALLBACK, api_playlists::PLAYLIST_REMOVED_POST, index, 0 );

	return API_PLAYLISTS_SUCCESS;
}

int Playlists::ClearPlaylists()
{
	AutoLockT<Playlists> lock( this );

	if ( !DelayLoad() )
		return API_PLAYLISTS_UNABLE_TO_LOAD_PLAYLISTS;

	dirty = true;
	playlists.clear();

	return API_PLAYLISTS_SUCCESS;
}

const PlaylistInfo &Playlists::GetPlaylistInfo(size_t i)
{
	return playlists[i];
}

#define CBCLASS Playlists
START_DISPATCH;
VCB( API_PLAYLISTS_LOCK,   Lock );
VCB( API_PLAYLISTS_UNLOCK, Unlock );
CB( API_PLAYLISTS_GETITERATOR,      GetIterator );
VCB( API_PLAYLISTS_FLUSH,           Flush );
CB( API_PLAYLISTS_GETCOUNT,         GetCount );
CB( API_PLAYLISTS_GETFILENAME,      GetFilename );
CB( API_PLAYLISTS_GETNAME,          GetName );
CB( API_PLAYLISTS_GETGUID,          GetGUID );
CB( API_PLAYLISTS_GETPOSITION,      GetPosition );
CB( API_PLAYLISTS_GETINFO,          GetInfo );
CB( API_PLAYLISTS_MOVEBEFORE,       MoveBefore );
CB( API_PLAYLISTS_ADDPLAYLIST,      AddPlaylist );
CB( API_PLAYLISTS_ADDPLAYLISTNOCB,  AddPlaylist_NoCallback );
CB( API_PLAYLISTS_ADDCLOUDPLAYLIST, AddCloudPlaylist );
CB( API_PLAYLISTS_SETGUID,          SetGUID );
CB( API_PLAYLISTS_RENAMEPLAYLIST,   RenamePlaylist );
CB( API_PLAYLISTS_MOVEPLAYLIST,     MovePlaylist );
CB( API_PLAYLISTS_SETINFO,          SetInfo );
CB( API_PLAYLISTS_REMOVEPLAYLIST,   RemovePlaylist );
CB( API_PLAYLISTS_CLEARPLAYLISTS,   ClearPlaylists );
CB( API_PLAYLISTS_SORT,             Sort );
END_DISPATCH;
#undef CBCLASS