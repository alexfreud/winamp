#ifndef NULLSOFT_ML_RG_API_H
#define NULLSOFT_ML_RG_API_H

#include "../Winamp/api_decodefile.h"
extern api_decodefile *decodeFile;
#define AGAVE_API_DECODE decodeFile

#include "api/application/api_application.h"
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManager;
#define AGAVE_API_PLAYLISTMANAGER playlistManager

#include "../Agave/Language/api_language.h"

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#include <api/service/waservicefactory.h>

#endif