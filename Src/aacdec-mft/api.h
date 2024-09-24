#pragma once

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager


#include "../Agave/Config/api_config.h"

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgrApi;
#define WASABI_API_MEMMGR memmgrApi
