#ifndef __WASABI_API_H
#define __WASABI_API_H

#include "wasabicfg.h"
#include "ServiceManager.h"
#include "SysCallbacks.h"
#include "MemoryManager.h"
#include "URLManager.h"
#include "PaletteManager.h"
#include "GammaManagerAPI.h"

#define WASABI_API_SVC serviceManager
#define WASABI_API_SYSCB sysCallbacks
#define WASABI_API_VIDEOPREFERENCES videoPreferences
#define WASABI_API_MEMMGR memoryManager
extern PaletteManager *paletteManager;
#define WASABI_API_PALETTE paletteManager
extern GammaManagerAPI *gammaManager;
#define WASABI_API_COLORTHEMES gammaManager

#include "../tagz/api_tagz.h"
extern api_tagz *tagz;
#define WINAMP5_API_TAGZ tagz

#include "feeds.h"
extern VideoTextFeed *videoTextFeed;
extern PlaylistTextFeed *playlistTextFeed;

#include "application.h"
#define WASABI_API_APP application

#include "Metadata.h"
#define WASABI_API_METADATA metadata

#include "../nu/threadpool/ThreadPool.h"
extern ThreadPool *threadPool;
#define WASABI_API_THREADPOOL threadPool

#include "stats.h"
#define AGAVE_API_STATS (&stats)

/* Services we need from W5S services */
#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManager;
#define AGAVE_API_PLAYLISTMANAGER playlistManager

#include "../playlist/api_playlists.h"
extern api_playlists *playlistsManager;
#define AGAVE_API_PLAYLISTS playlistsManager

#include "../Components/wac_downloadManager/wac_downloadManager_api.h"

extern URLManager *urlmanagerApi;
#define AGAVE_API_URLMANAGER urlmanagerApi

#include "./winampApi.h"
extern WinampApi *winampApi;
#define WASABI_API_WINAMP winampApi

#include "../jpeg/amg.h"
extern api_amgsucks *amgSucks;
#define AGAVE_API_AMGSUCKS amgSucks

#include "../Agave/AlbumArt/api_albumart.h"
extern api_albumart *albumArtApi;
#define AGAVE_API_ALBUMART albumArtApi

/* Services we need to watch for */
#include <api/skin/api_skin.h>
#define WASABI_API_SKIN skinApi

#endif