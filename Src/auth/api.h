#pragma once
#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../Agave/Config/api_config.h"
extern api_config *config;
#define AGAVE_API_CONFIG config

#include <api/syscb/api_syscb.h>
#define WASABI_API_SYSCB sysCallbackApi

#include "../Agave/Language/api_language.h"

#include "../winamp/api_winamp.h"
extern api_winamp *winampApi;
#define WASABI_API_WINAMP winampApi

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memManagerApi;
#define WASABI_API_MEMMNGR memManagerApi

#include <api/service/svcs/svc_imgload.h>
extern svc_imageLoader *pngLoaderApi;
#define WASABI_API_PNGLOADER pngLoaderApi
EXTERN_C const GUID pngLoaderGUID;

