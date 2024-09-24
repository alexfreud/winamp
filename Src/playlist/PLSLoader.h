#ifndef NULLSOFT_PLAYLIST_PLSLOADER_H
#define NULLSOFT_PLAYLIST_PLSLOADER_H

#include "ifc_playlistloader.h"
#include <windows.h>

class PLSLoader : public ifc_playlistloader
{
public:
	PLSLoader()                                                       {}
	virtual ~PLSLoader()                                              {}

	int Load( const wchar_t *filename, ifc_playlistloadercallback *playlist );

protected:
	RECVS_DISPATCH;

private:
	int OnFileHelper( ifc_playlistloadercallback *playlist, const wchar_t *trackName, const wchar_t *title, int length, const wchar_t *rootPath, ifc_plentryinfo *extraInfo );
};

#endif