#pragma once

#include "../Agave/Language/api_language.h"

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../Components/wac_downloadManager/wac_downloadManager_api.h"

#include "../Agave/Config/api_config.h"
extern api_config *config;
#define AGAVE_API_CONFIG config

#include "../devices/api_devicemanager.h"
extern api_devicemanager *deviceManagerApi;
#define AGAVE_API_DEVICEMANAGER deviceManagerApi

#include "../Agave/Metadata/api_metadata.h"
extern api_metadata *metadataApi;
#define AGAVE_API_METADATA metadataApi


#include "../Agave/AlbumArt/api_albumart.h"
extern api_albumart *albumArtApi;
#define AGAVE_API_ALBUMART albumArtApi


#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgr;
#define WASABI_API_MEMMGR memmgr


namespace Wasabi2
{
#include "service/api_service.h"
#include "ssdp/api_ssdp.h"
}

extern Wasabi2::api_service *WASABI2_API_SVC;
extern Wasabi2::api_ssdp *REPLICANT_API_SSDP;

void WasabiInit();
void WasabiQuit();
