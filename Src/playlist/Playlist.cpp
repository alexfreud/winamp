#include "main.h"
#include "Playlist.h"
#include <algorithm>
#include "../nu/AutoChar.h"
#include "../Winamp/strutil.h"

void Playlist::Clear()
{
	for ( pl_entry *entry : entries )
		delete entry;

	entries.clear();
}

int Playlist::OnFile( const wchar_t *p_filename, const wchar_t *p_title, int p_lengthInMS, ifc_plentryinfo *p_info )
{
	entries.push_back( new pl_entry( p_filename, p_title, p_lengthInMS, p_info ) );

	return ifc_playlistloadercallback::LOAD_CONTINUE;
}

void Playlist::AppendWithInfo( const wchar_t *p_filename, const wchar_t *p_title, int p_lengthInMS )
{
	entries.push_back( new pl_entry( p_filename, p_title, p_lengthInMS ) );
}

void Playlist::Insert( size_t p_index, const wchar_t *p_filename, const wchar_t *p_title, int p_lengthInMS )
{
	entries.insert( entries.begin() + p_index, new pl_entry( p_filename, p_title, p_lengthInMS ) );
}

Playlist::~Playlist()
{
	Clear();
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
	RandMod( int ( *_generator )( ) ) : generator( _generator )       {}
	int operator ()( int n )                                          { return generator() % n; }
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
	if ( entries.size() > item )
		entries.erase( entries.begin() + item );
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
		entries[ item ]->SetLengthMilliseconds( length );
}

static bool PlayList_sortByTitle( pl_entry *&a, pl_entry *&b )
{
	int comp = CompareStringW( LOCALE_USER_DEFAULT, NORM_IGNORECASE /*|NORM_IGNOREKANATYPE*/ | NORM_IGNOREWIDTH, a->filetitle, -1, b->filetitle, -1 );

	return comp == CSTR_LESS_THAN;
	// TODO: grab this function from winamp - return CompareStringLogical(a.strTitle, b.strTitle)<0;
}

static bool PlayList_sortByFile( pl_entry *&a, pl_entry *&b ) //const void *a, const void *b)
{
	const wchar_t *file1 = PathFindFileNameW( a->filename );
	const wchar_t *file2 = PathFindFileNameW( b->filename );

	int comp = CompareStringW( LOCALE_USER_DEFAULT, NORM_IGNORECASE |   /*NORM_IGNOREKANATYPE |*/ NORM_IGNOREWIDTH, file1, -1, file2, -1 );

	return comp == CSTR_LESS_THAN;
	// TODO: grab this function from winamp - return FileCompareLogical(file1, file2)<0;
}

static bool PlayList_sortByDirectory( pl_entry *&a, pl_entry *&b ) // by dir, then by p_title
{
	const wchar_t *directory1    = a->filename;
	const wchar_t *directory2    = b->filename;

	const wchar_t *directoryEnd1 = scanstr_backcW( directory1, L"\\", 0 );
	const wchar_t *directoryEnd2 = scanstr_backcW( directory2, L"\\", 0 );

	int            dirLen1       = (int)( directoryEnd1 - directory1 );
	int            dirLen2       = (int)( directoryEnd2 - directory2 );

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

	int comp = CompareStringW( LOCALE_USER_DEFAULT, NORM_IGNORECASE |  /*NORM_IGNOREKANATYPE | */NORM_IGNOREWIDTH, directory1, dirLen1, directory2, dirLen2 );
	if ( comp == CSTR_EQUAL ) // same dir
		return PlayList_sortByFile( a, b ); // do second sort
	else // different dirs
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

