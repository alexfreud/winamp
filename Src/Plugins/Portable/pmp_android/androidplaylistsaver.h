#pragma once

#include <wtypes.h>
#include "../playlist/ifc_playlist.h"

class AndroidPlaylist;
 
class AndroidPlaylistSaver : public ifc_playlist
{

public:
	AndroidPlaylistSaver(LPCTSTR iFilename, LPCTSTR iTitle, AndroidPlaylist * iPlaylist);
	virtual ~AndroidPlaylistSaver();

public:
	/*** ifc_playlist ***/
	size_t GetNumItems();
	size_t GetItem(size_t item, wchar_t *filename, size_t filenameCch);
	size_t GetItemTitle(size_t item, wchar_t *title, size_t titleCch);
	int GetItemLengthMs(size_t item); // TODO: maybe microsecond for better resolution?
	size_t GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch);

	HRESULT Save();
protected:
	RECVS_DISPATCH;

protected:
	LPTSTR	title;
    LPTSTR	filename;
	AndroidPlaylist *playlist;
};


