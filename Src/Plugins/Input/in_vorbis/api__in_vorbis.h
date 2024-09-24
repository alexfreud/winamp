#ifndef NULLSOFT_API_H
#define NULLSOFT_API_H

#include "../Agave/Config/api_config.h"
#include "../Agave/Language/api_language.h"

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include <api/service/svcs/svc_imgload.h>

#include <api/service/api_service.h>

#include <api/service/waServiceFactory.h>

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgrApi;
#define WASABI_API_MEMMGR memmgrApi

#endif