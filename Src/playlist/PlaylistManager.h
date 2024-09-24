#ifndef NULLSOFT_ML_PLAYLISTS_PLAYLIST_MANAGER_H
#define NULLSOFT_ML_PLAYLISTS_PLAYLIST_MANAGER_H

#include "api_playlistmanager.h"

class PlaylistManager : public api_playlistmanager
{
public:
	int            Load( const wchar_t *filename, ifc_playlistloadercallback *playlist );
	int            LoadAs( const wchar_t *filename, const wchar_t *ext, ifc_playlistloadercallback *playlist );
	int            LoadFromDialog( const wchar_t *fns, ifc_playlistloadercallback *playlist );
	int            LoadFromANSIDialog( const char *fns, ifc_playlistloadercallback *playlist );

	int            Save( const wchar_t *filename, ifc_playlist *playlist );

	size_t         Copy( const wchar_t *destFn, const wchar_t *srcFn ); // returns number of items copied

	size_t         CountItems( const wchar_t *filename );

	int            GetLengthMilliseconds( const wchar_t *filename );
	uint64_t       GetLongLengthMilliseconds( const wchar_t *filename );

	void           Randomize( ifc_playlist *playlist );
	void           Reverse( ifc_playlist *playlist );

	void           LoadDirectory( const wchar_t *directory, ifc_playlistloadercallback *callback, ifc_playlistdirectorycallback *dirCallback );
	
	bool           CanLoad( const wchar_t *filename );
	
	void           GetExtensionList( wchar_t *extensionList, size_t extensionListCch );
	void           GetFilterList( wchar_t *extensionList, size_t extensionListCch );
	
	const wchar_t *EnumExtensions( size_t num );

protected:
	RECVS_DISPATCH;

};

extern PlaylistManager playlistManager;

#endif