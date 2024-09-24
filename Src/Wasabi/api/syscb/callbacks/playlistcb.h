#ifndef NULLSOFT_AGAVE_PLAYLISTCB_H
#define NULLSOFT_AGAVE_PLAYLISTCB_H

#include "../playlist/api_playlists.h"

#include "api/syscb/callbacks/syscbi.h"


#define PLAYLISTCALLBACKI_PARENT SysCallbackI
class PlaylistCallbackI : public PLAYLISTCALLBACKI_PARENT
{
public:
	virtual FOURCC syscb_getEventType()                               { return api_playlists::SYSCALLBACK; }

protected:
	// override these
	virtual int playlistcb_added( size_t index )                      { return 0; }
	virtual int playlistcb_saved( size_t index )                      { return 0; }

private:
	virtual int syscb_notify( int msg, intptr_t param1 = 0, intptr_t param2 = 0 );
};



#endif