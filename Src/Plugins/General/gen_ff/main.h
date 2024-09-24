#ifndef __GEN_FF_MAIN_H
#define __GEN_FF_MAIN_H

class ifc_window;

extern ifc_window *plWnd;
extern ifc_window *mbWnd;
extern ifc_window *vidWnd;

#define NUMSTATICWINDOWS 3

#include "../playlist/api_playlists.h"
extern api_playlists *playlistsApi;
#define AGAVE_API_PLAYLISTS playlistsApi

#include "../Agave/AlbumArt/api_albumart.h"
extern api_albumart *albumArtApi;
#define AGAVE_API_ALBUMART albumArtApi

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManagerApi;
#define AGAVE_API_PLAYLISTMANAGER playlistManagerApi

#include "../Components/wac_downloadManager/wac_downloadManager_api.h"

#include "../Winamp/gen.h"
extern "C" winampGeneralPurposePlugin plugin;

#endif