#include "playlistcb.h"

int PlaylistCallbackI::syscb_notify(int msg, intptr_t param1, intptr_t param2) 
{
  switch (msg) {
    case api_playlists::PLAYLIST_ADDED: return playlistcb_added(static_cast<size_t>(param1));
			case api_playlists::PLAYLIST_SAVED: return playlistcb_saved(static_cast<size_t>(param1));
  }
  return 0;
}
