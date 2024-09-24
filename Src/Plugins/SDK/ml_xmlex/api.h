#ifndef NULLSOFT_API_H
#define NULLSOFT_API_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include <api/service/waServiceFactory.h>

#include "../Agave/Config/api_config.h"
extern api_config *configApi;
#define AGAVE_API_CONFIG configApi

#include "../Agave/Language/api_language.h"

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManagerApi;
#define AGAVE_API_PLAYLISTMANAGER playlistManagerApi

HRESULT WasabiApi_Initialize(HINSTANCE hInstance, api_service *serviceApi);
HRESULT WasabiApi_LoadDefaults();
ULONG WasabiApi_AddRef(void);
ULONG WasabiApi_Release(void);

#endif
