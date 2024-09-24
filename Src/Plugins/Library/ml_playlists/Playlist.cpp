#include "main.h"
#include "./playlist.h"
#include <algorithm>
#include "nu/MediaLibraryInterface.h"
#include "Winamp/strutil.h"

Playlist::~Playlist()
{
	Clear();
}


void Playlist::Clear()
{
	for ( pl_entry *entry : entries )
		delete entry;

	entries.clear();

	lengthInMS = 0;
}

void Playlist::OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info )
{
	entries.push_back( new pl_entry( filename, title, lengthInMS, info ) );

	this->lengthInMS += lengthInMS;
}

void Playlist::AppendWithInfo( const wchar_t *filename, const wchar_t *title, int lengthInMS )
{
	this->lengthInMS += lengthInMS;
	entries.push_back( new pl_entry( filename, title, lengthInMS ) );
}

void Playlist::AppendWithInfo( const wchar_t *filename, const wchar_t *title, int lengthInMS, std::map<std::wstring, std::wstring> &p_extended_infos )
{
	this->lengthInMS += lengthInMS;

	pl_entry *l_new_pl_entry = new pl_entry( filename, title, lengthInMS );

	if ( !p_extended_infos.empty() )
	{
		for ( auto l_extended_info : p_extended_infos )
			l_new_pl_entry->_extended_infos.emplace( _wcsdup( l_extended_info.first.c_str() ), _wcsdup( l_extended_info.second.c_str() ) );
	}

	entries.push_back( l_new_pl_entry );
}

size_t Playlist::GetNumItems()
{
	return entries.size();
}

size_t Playlist::GetItem( size_t item, wchar_t *filename, size_t filenameCch )
{
	if ( item >= entries.size() )
		return 0;

	return entries[ item ]->GetFilename( filename, filenameCch );
}

size_t Playlist::GetItemTitle( size_t item, wchar_t *title, size_t titleCch )
{
	if ( item >= entries.size() )
		return 0;

	return entries[ item ]->GetTitle( title, titleCch );
}

const wchar_t *Playlist::ItemTitle( size_t item )
{
	if ( item >= entries.size() )
		return 0;

	return entries[ item ]->filetitle;
}

const wchar_t *Playlist::ItemName( size_t item )
{
	if ( item >= entries.size() )
		return 0;

	return entries[ item ]->filename;
}

int Playlist::GetItemLengthMilliseconds( size_t item )
{
	if ( item >= entries.size() )
		return -1;

	return entries[ item ]->GetLengthInMilliseconds();
}

size_t Playlist::GetItemExtendedInfo( size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch )
{
	if ( item >= entries.size() )
		return 0;

	return entries[ item ]->GetExtendedInfo( metadata, info, infoCch );
}

int Playlist::Reverse()
{
	// TODO: keep a bool flag and just do size-item-1 every time a GetItem* function is called
	std::reverse( entries.begin(), entries.end() );

	return PLAYLIST_SUCCESS;
}

int Playlist::Swap( size_t item1, size_t item2 )
{
	std::swap( entries[ item1 ], entries[ item2 ] );

	return PLAYLIST_SUCCESS;
}

class RandMod
{
public:
	RandMod( int ( *_generator )( ) ) : generator( _generator )          {}
	int operator ()( int n )                                             { return generator() % n; }
	int ( *generator )( );
};

int Playlist::Randomize( int ( *generator )( ) )
{
	RandMod randMod( generator );
	std::random_shuffle( entries.begin(), entries.end(), randMod );

	return PLAYLIST_SUCCESS;
}

void Playlist::Remove( size_t item )
{
	lengthInMS -= entries[item]->length;
	delete entries[item];
	entries.erase(entries.begin() + item);
}

void Playlist::SetItemFilename( size_t item, const wchar_t *filename )
{
	if ( item < entries.size() )
		entries[ item ]->SetFilename( filename );
}

void Playlist::SetItemTitle( size_t item, const wchar_t *title )
{
	if ( item < entries.size() )
		entries[ item ]->SetTitle( title );
}

void Playlist::SetItemLengthMilliseconds( size_t item, int length )
{
	if ( item < entries.size() )
	{
		lengthInMS -= entries[ item ]->length;
		entries[ item ]->SetLengthMilliseconds( length );
		lengthInMS += length;
	}
}


void GetTitle( pl_entry *&a )
{
	if ( !a->cached )
	{
		wchar_t title[ FILETITLE_SIZE ] = { 0 };
		int     length                  = -1;

		mediaLibrary.GetFileInfo( a->filename, title, FILETITLE_SIZE, &length );

		a->SetLengthMilliseconds( length * 1000 );
		a->SetTitle( title );
	}
}

static bool PlayList_sortByTitle( pl_entry *&a, pl_entry *&b )
{
	GetTitle( a );
	GetTitle( b );

	int comp = CompareStringW( LOCALE_USER_DEFAULT, NORM_IGNORECASE /*|NORM_IGNOREKANATYPE*/ | NORM_IGNOREWIDTH, a->filetitle, -1, b->filetitle, -1 );

	return comp == CSTR_LESS_THAN;
	// TODO: grab this function from winamp - return CompareStringLogical(a.strTitle, b.strTitle)<0;
}

static bool PlayList_sortByFile( pl_entry *&a, pl_entry *&b ) //const void *a, const void *b)
{
	const wchar_t *file1 = PathFindFileNameW( a->filename );
	const wchar_t *file2 = PathFindFileNameW( b->filename );

	int comp = CompareStringW( LOCALE_USER_DEFAULT, NORM_IGNORECASE |    /*NORM_IGNOREKANATYPE |*/ NORM_IGNOREWIDTH, file1, -1, file2, -1 );

	return comp == CSTR_LESS_THAN;
	// TODO: grab this function from winamp - return FileCompareLogical(file1, file2)<0;
}

static bool PlayList_sortByDirectory( pl_entry *&a, pl_entry *&b ) // by dir, then by title
{
	const wchar_t *directory1    = a->filename;
	const wchar_t *directory2    = b->filename;

	const wchar_t *directoryEnd1 = scanstr_backcW( directory1, L"\\", 0 );
	const wchar_t *directoryEnd2 = scanstr_backcW( directory2, L"\\", 0 );

	size_t         dirLen1       = directoryEnd1 - directory1;
	size_t         dirLen2       = directoryEnd2 - directory2;

	if ( !dirLen1 && !dirLen2 )             // both in the current directory?
		return PlayList_sortByFile( a, b ); // not optimized, because the function does another scanstr_back, but easy for now :)

	if ( !dirLen1 )                         // only the first dir is empty?
		return true;                        // sort it first

	if ( !dirLen2 )                         // only the second dir empty?
		return false;                       // empty dirs go first

#if 0 // TODO: grab this function from winamp
	int comp = FileCompareLogicalN( directory1, dirLen1, directory2, dirLen2 );
	if ( comp == 0 )
		return PlayList_sortByFile( a, b );
	else
		return comp < 0;
#endif

	int comp = CompareStringW( LOCALE_USER_DEFAULT, NORM_IGNORECASE |   /*NORM_IGNOREKANATYPE | */NORM_IGNOREWIDTH, directory1, dirLen1, directory2, dirLen2 );
	if ( comp == CSTR_EQUAL )               // same dir
		return PlayList_sortByFile( a, b ); // do second sort
	else                                    // different dirs
		return comp == CSTR_LESS_THAN;
}

int Playlist::SortByTitle()
{
	std::sort( entries.begin(), entries.end(), PlayList_sortByTitle );

	return 1;
}

int Playlist::SortByFilename()
{
	std::sort( entries.begin(), entries.end(), PlayList_sortByFile );

	return 1;
}

int Playlist::SortByDirectory()
{
	std::sort( entries.begin(), entries.end(), PlayList_sortByDirectory );

	return 1;
}
/*
int Playlist::Move(size_t itemSrc, size_t itemDest)
{
	if (itemSrc < itemDest)
		std::rotate(&entries[itemSrc], &entries[itemSrc], &entries[itemDest]);
	else 
		if (itemSrc > itemDest)
		std::rotate(&entries[itemDest], &entries[itemSrc], &entries[itemSrc]);
	return 1;
}*/

bool Playlist::IsCached( size_t item )
{
	return entries[ item ]->cached;
}

bool Playlist::IsLocal( size_t item )
{
	return entries[ item ]->isLocal();
}

void Playlist::ClearCache( size_t item )
{
	entries[ item ]->cached = false;
}

void Playlist::InsertPlaylist( Playlist &copy, size_t index )
{
	for ( pl_entry *l_entry : copy.entries )
	{
		entries.insert( entries.begin() + index, l_entry );
		++index;
		lengthInMS += l_entry->length;
	}

	copy.entries.clear();
}

void Playlist::AppendPlaylist( Playlist &copy )
{
	for ( pl_entry *l_entry : copy.entries )
	{
		this->entries.push_back( l_entry );
		lengthInMS += l_entry->length;
	}

	copy.entries.clear();
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS Playlist

START_MULTIPATCH;
START_PATCH( patch_playlist )
M_VCB( patch_playlist, ifc_playlist, IFC_PLAYLIST_CLEAR,                     Clear )
//M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_APPENDWITHINFO, AppendWithInfo)
//M_VCB(patch_playlist, ifc_playlist, IFC_PLAYLIST_APPEND, Append)
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_GETNUMITEMS,               GetNumItems )
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_GETITEM,                   GetItem )
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_GETITEMTITLE,              GetItemTitle )
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMilliseconds )
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_GETITEMEXTENDEDINFO,       GetItemExtendedInfo )
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_REVERSE,                   Reverse )
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_SWAP,                      Swap )
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_RANDOMIZE,                 Randomize )
M_VCB( patch_playlist, ifc_playlist, IFC_PLAYLIST_REMOVE,                    Remove )
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_SORTBYTITLE,               SortByTitle )
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_SORTBYFILENAME,            SortByFilename )
M_CB( patch_playlist,  ifc_playlist, IFC_PLAYLIST_SORTBYDIRECTORY,           SortByDirectory )
NEXT_PATCH( patch_playlistloadercallback )
M_VCB( patch_playlistloadercallback, ifc_playlistloadercallback, IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile );
END_PATCH
END_MULTIPATCH;