#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/service/waServiceFactory.h>

#include "../Agave/language/api_language.h"

#include "../Agave/queue/api_queue.h"

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgrApi;
#define WASABI_API_MEMMGR memmgrApi

#include <api/service/svcs/svc_imgload.h>
#include <api/service/svcs/svc_imgwrite.h>

#include "../Agave/AlbumArt/api_albumart.h"

#include "../Agave/ExplorerFindFile/api_explorerfindfile.h"

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID = 
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

#include "..\Agave/Config/api_config.h"
extern api_config *configApi;
#define AGAVE_API_CONFIG configApi

#include "..\..\..\Components\wac_network\wac_network_http_receiver_api.h"
#include "..\..\..\Components\wac_downloadManager\wac_downloadManager_api.h"

#endif