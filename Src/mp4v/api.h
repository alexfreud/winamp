#pragma once

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include "../winamp/api_winamp.h"
extern api_winamp *winampApi;
#define AGAVE_API_WINAMP winampApi