#pragma once

#include <api/service/api_service.h>

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManagerApi;
#define AGAVE_API_PLAYLISTMANAGER playlistManagerApi
