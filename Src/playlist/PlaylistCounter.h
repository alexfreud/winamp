#ifndef NULLSOFT_PLAYLIST_PLAYLISTCOUNTER_H
#define NULLSOFT_PLAYLIST_PLAYLISTCOUNTER_H

#include "ifc_playlistloadercallback.h"

class PlaylistCounter : public ifc_playlistloadercallback
{
public:
	PlaylistCounter()                                                 {}

	int OnFile( const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info );
	
	size_t   count  = 0;
	uint64_t length = 0;

protected:
	RECVS_DISPATCH;

};

#endif