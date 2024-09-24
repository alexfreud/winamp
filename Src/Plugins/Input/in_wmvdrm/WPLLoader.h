#ifndef NULLSOFT_PLAYLIST_WPL_LOADER_H
#define NULLSOFT_PLAYLIST_WPL_LOADER_H

#include "../playlist/ifc_playlistloader.h"
#include "../playlist/ifc_playlistloadercallback.h"
#include <stdio.h>
class WPLLoader : public ifc_playlistloader
{
public:
	int Load(const wchar_t *filename, ifc_playlistloadercallback *playlist);

public:
	WPLLoader();
	virtual ~WPLLoader(void);

protected:
	RECVS_DISPATCH;
};
#endif