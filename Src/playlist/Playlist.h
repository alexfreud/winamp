#ifndef NULLSOFT_ML_PLAYLISTS_PLAYLIST_H
#define NULLSOFT_ML_PLAYLISTS_PLAYLIST_H

#include "ifc_playlist.h"

#include <windows.h> // for MAX_PATH
#include "pl_entry.h"
#include "ifc_playlistT.h"
#include "ifc_playlistloadercallbackT.h"
#include <vector>


class Playlist : public ifc_playlistloadercallbackT<Playlist>, public ifc_playlistT<Playlist>
{
public:
	virtual       ~Playlist();

	void           Clear();
	int            OnFile( const wchar_t *p_filename, const wchar_t *p_title, int p_lengthInMS, ifc_plentryinfo *p_info );
	void           AppendWithInfo( const wchar_t *p_filename, const wchar_t *p_title, int p_lengthInMS );
	void           Insert( size_t p_index, const wchar_t *p_filename, const wchar_t *p_title, int p_lengthInMS );
	
	size_t         GetNumItems();

	size_t         GetItem( size_t item, wchar_t *filename, size_t filenameCch );
	size_t         GetItemTitle( size_t item, wchar_t *title, size_t titleCch );
	const wchar_t *ItemTitle( size_t item );
	const wchar_t *ItemName( size_t item );
	int            GetItemLengthMilliseconds( size_t item ); // TODO: maybe microsecond for better resolution?
	size_t         GetItemExtendedInfo( size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch );

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

private:
	typedef std::vector<pl_entry*> PlaylistEntries;
	PlaylistEntries entries;
};
#endif