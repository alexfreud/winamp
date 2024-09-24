#ifndef NULLSOFT_ML_PLAYLISTS_PLAYLIST_H
#define NULLSOFT_ML_PLAYLISTS_PLAYLIST_H

#include "playlist/ifc_playlist.h"

#include "bfc/multipatch.h"
#include "playlist/pl_entry.h"
#include "playlist/ifc_playlistloadercallback.h"

enum
{ 
	patch_playlist, 
	patch_playlistloadercallback
};

class Playlist : public MultiPatch<patch_playlist, ifc_playlist>, public MultiPatch<patch_playlistloadercallback, ifc_playlistloadercallback>
{
public:
	Playlist()                                                        {}
	virtual ~Playlist();

	void           Clear();
	void           OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info );
	
	void           AppendWithInfo( const wchar_t *filename, const wchar_t *title, int lengthInMS );
	void           AppendWithInfo( const wchar_t *filename, const wchar_t *title, int lengthInMS, std::map<std::wstring, std::wstring> &p_extended_infos );

	size_t         GetNumItems();

	size_t         GetItem( size_t item, wchar_t *filename, size_t filenameCch );
	size_t         GetItemTitle( size_t item, wchar_t *title, size_t titleCch );
	const wchar_t *ItemTitle( size_t item );
	const wchar_t *ItemName( size_t item );
	int            GetItemLengthMilliseconds( size_t item ); // TODO: maybe microsecond for better resolution?
	size_t         GetItemExtendedInfo( size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch );

	bool           IsCached( size_t item );
	bool           IsLocal( size_t item );
	void           ClearCache( size_t item );

	void           SetItemFilename( size_t item, const wchar_t *filename );
	void           SetItemTitle( size_t item, const wchar_t *title );
	void           SetItemLengthMilliseconds( size_t item, int length );

	int            Reverse();
	int            Swap( size_t item1, size_t item2 );
	int            Randomize( int ( *generator )( ) );
	void           Remove( size_t item );

	int            SortByTitle();
	int            SortByFilename();
	int            SortByDirectory(); //sorts by directory and then by filename

	void           InsertPlaylist( Playlist &copy, size_t index );
	void           AppendPlaylist( Playlist &copy );

protected:
	RECVS_MULTIPATCH;

public:
	typedef std::vector<pl_entry*> PlaylistEntries;
	PlaylistEntries entries;
	uint64_t        lengthInMS = 0;
};

#endif // !NULLSOFT_ML_PLAYLISTS_PLAYLIST_H