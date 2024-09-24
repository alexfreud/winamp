#ifndef NULLSOFT_ML_PMP_API_H
#define NULLSOFT_ML_PMP_API_H

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManager;
#define AGAVE_API_PLAYLISTMANAGER playlistManager

#include "../playlist/api_playlists.h"
extern api_playlists *playlistsApi;
#define AGAVE_API_PLAYLISTS playlistsApi

#include "../ml_local/api_mldb.h"
extern api_mldb *mldbApi;
#define AGAVE_API_MLDB mldbApi

#include <api/syscb/api_syscb.h>
extern api_syscb *sysCallbackApi;
#define WASABI_API_SYSCB sysCallbackApi

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../Agave/Language/api_language.h"

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memoryManager;
#define WASABI_API_MEMMGR memoryManager

#include "../ml_wire/api_podcasts.h"
extern api_podcasts *podcastsApi;
#define AGAVE_API_PODCASTS podcastsApi

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi

#include "../devices/api_devicemanager.h"
extern api_devicemanager *deviceManagerApi;
#define AGAVE_API_DEVICEMANAGER deviceManagerApi

#include "../Agave/AlbumArt/api_albumart.h"
extern api_albumart *albumArtApi;
#define AGAVE_API_ALBUMART albumArtApi

#include "../Agave/Metadata/api_metadata.h"
extern api_metadata *metadataApi;
#define AGAVE_API_METADATA metadataApi

#include <api/service/svcs/svc_imgload.h>

#endif // !NULLSOFT_ML_PMP_API_H
