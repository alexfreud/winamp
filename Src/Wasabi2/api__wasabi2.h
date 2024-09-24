#pragma once
#include <Wasabi/api__wasabi-replicant.h>

#include "application.h"
extern Application application;
#define WASABI2_API_APP (&application)

/* Wasabi 1 */
namespace Wasabi {
#include <api/service/api_service.h>
}
extern Wasabi::api_service *wasabi1_service_api;
#define WASABI_API_SVC wasabi1_service_api

#include <new>
namespace Wasabi {
#include <api/application/api_application.h>
#include "../Agave/AlbumArt/api_albumart.h"

#include <api/memmgr/api_memmgr.h>
}
extern Wasabi::api_application *wasabi1_application_api;
#define WASABI_API_APP wasabi1_application_api

extern Wasabi::api_albumart *wasabi1_albumart_api;
#define AGAVE_API_ALBUMART wasabi1_albumart_api

extern Wasabi::api_memmgr *wasabi1_memmgr_api;
#define WASABI_API_MEMMGR wasabi1_memmgr_api