#ifndef NULLSOFT_WINAMP_PLAYLIST_H
#define NULLSOFT_WINAMP_PLAYLIST_H

#include "../playlist/ifc_playlistloadercallback.h"

class Playlist : public ifc_playlistloadercallback
{
public:
    void OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info );
};

#endif