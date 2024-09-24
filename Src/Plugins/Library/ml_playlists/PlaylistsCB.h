#ifndef NULLSOFT_ML_PLAYLISTS_PLAYLISTSCB_H
#define NULLSOFT_ML_PLAYLISTS_PLAYLISTSCB_H

#include <api/syscb/callbacks/syscb.h>
#include "../playlist/api_playlists.h"

class PlaylistsCB : public SysCallback
{
public:
	FOURCC getEventType() { return api_playlists::SYSCALLBACK; }
	int notify(int msg, intptr_t param1 = 0, intptr_t param2 = 0);

protected:
	RECVS_DISPATCH;
};
#endif