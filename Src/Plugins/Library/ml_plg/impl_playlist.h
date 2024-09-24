#ifndef NULLSOFT_ML_PLG_IMPL_PLAYLIST_H
#define NULLSOFT_ML_PLG_IMPL_PLAYLIST_H

#include "../playlist/ifc_playlist.h"
#include <vector>
#include <windows.h> // for MAX_PATH
#include "../playlist/pl_entry.h"
#include <bfc/multipatch.h>
#include <bfc/platform/types.h>
#include "../playlist/ifc_playlistloadercallback.h"

enum
{
	patch_playlist, 
	patch_playlistloadercallback
};

class Playlist : public MultiPatch<patch_playlist, ifc_playlist>, public MultiPatch<patch_playlistloadercallback, ifc_playlistloadercallback>
{
public:
	~Playlist();

	void           Clear();
	void           OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, int sizeInKB, ifc_plentryinfo *info );
	void           AppendWithInfo( const wchar_t *filename, const wchar_t *title, int lengthInMS, int sizeInBytes );

	size_t         GetNumItems();

	size_t         GetItem( size_t item, wchar_t *filename, size_t filenameCch );
	size_t         GetItemTitle( size_t item, wchar_t *title, size_t titleCch );
	const wchar_t *ItemTitle( size_t item );
	const wchar_t *ItemName( size_t item );
	int            GetItemLengthMilliseconds( size_t item );
	int            GetItemSizeBytes( size_t item );
	size_t         GetItemExtendedInfo( size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch );
	uint64_t       GetPlaylistSizeBytes( void );
	uint64_t       GetPlaylistLengthMilliseconds( void );

	bool           IsCached( size_t item );
	void           ClearCache( size_t item );

	void           SetItemFilename( size_t item, const wchar_t *filename );
	void           SetItemTitle( size_t item, const wchar_t *title );
	void           SetItemLengthMilliseconds( size_t item, int length );
	void           SetItemSizeBytes( size_t item, int size );

	int            Reverse();
	int            Swap( size_t item1, size_t item2 );
	int            Randomize( int ( *generator )( ) );
	void           Remove( size_t item );

	int            SortByTitle();
	int            SortByFilename();
	int            SortByDirectory(); //sorts by directory and then by filename

	void           InsertPlaylist( Playlist &copy, size_t index );
	void           AppendPlaylist( Playlist &copy );

	typedef std::vector<pl_entry*> PlaylistEntries;
	PlaylistEntries entries;

protected:
	RECVS_MULTIPATCH;
};
#endif