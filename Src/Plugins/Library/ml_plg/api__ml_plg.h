#ifndef NULLSOFT_ML_PLG_API_H
#define NULLSOFT_ML_PLG_API_H

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManagerApi;
#define AGAVE_API_PLAYLISTMGR playlistManagerApi

#include "../Agave/Config/api_config.h"
//extern api_config *agaveConfigApi;
//#define AGAVE_API_CONFIG agaveConfigApi

#include "../Winamp/api_decodefile.h"
extern api_decodefile *decodeApi;
#define AGAVE_API_DECODE decodeApi

#include "../gracenote/api_gracenote.h"
extern api_gracenote *gracenoteApi;
#define AGAVE_API_GRACENOTE gracenoteApi

#include "../Agave/Metadata/api_metadata.h"
extern api_metadata *metadataApi;
#define AGAVE_API_METADATA metadataApi

#include "../ml_local/api_mldb.h"
extern api_mldb *mldbApi;
#define AGAVE_API_MLDB mldbApi

#include <api/syscb/api_syscb.h>
extern api_syscb *sysCallbackApi;
#define WASABI_API_SYSCB sysCallbackApi

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi

// (BigG) Added for playlist export support
#include "../playlist/api_playlists.h"
extern api_playlists *playlistsApi;
#define AGAVE_API_PLAYLISTS playlistsApi

// Added for Stat collection
#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#endif