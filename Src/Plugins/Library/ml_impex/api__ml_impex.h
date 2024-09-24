#ifndef NULLSOFT_ML_IMPEX_API_H
#define NULLSOFT_ML_IMPEX_API_H

#include <api/service/waservicefactory.h>

#include "../Agave/Language/api_language.h"

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManager;
#define AGAVE_API_PLAYLISTMANAGER playlistManager

#include "../playlist/api_playlists.h"
extern api_playlists *playlistsApi;
#define AGAVE_API_PLAYLISTS playlistsApi

#endif // !NULLSOFT_ML_IMPEX_API_H
