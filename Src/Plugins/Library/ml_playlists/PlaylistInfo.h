#ifndef NULLSOFT_ML_PLAYLISTS_PLAYLIST_INFO_H
#define NULLSOFT_ML_PLAYLISTS_PLAYLIST_INFO_H

#include <windows.h> // for MAX_PATH
#include <iostream>  // for std::wstring

// REVIEW: what if we want this to be an ifc_playlist * from elsewhere instead of a physical m3u file
// maybe this should be a playlist factory instead?
class PlaylistInfo
{
	/* --- Methods --- */
public:
	PlaylistInfo();
	PlaylistInfo( GUID _playlist_guid );
	PlaylistInfo( size_t p_index ); // as a pre-condition, AGAVE_API_PLAYLISTS needs to be locked
	PlaylistInfo( const PlaylistInfo &copy );

	bool Valid();

	bool Associate( INT_PTR _treeId );

	size_t         GetIndex();
	const wchar_t *GetFilename();
	const wchar_t *GetName();

	size_t GetLength();
	void   SetLength( size_t newLength );

	size_t GetSize();
	void   SetSize( size_t newSize );

	size_t GetCloud();
	void   SetCloud( size_t newCloud );

	void   Refresh();

	void   IssueSaveCallback();

private:
	void Clear();

	/* --- Data --- */
public:
	INT_PTR treeId; // if it's being displayed as a media library tree, the id is here (0 otherwise)
	GUID    playlist_guid;

private:
	size_t index;
	size_t iterator;
	size_t length; // in seconds
	size_t numItems;
	size_t cloud;

	std::wstring _title;
	std::wstring _filename;
};

#endif