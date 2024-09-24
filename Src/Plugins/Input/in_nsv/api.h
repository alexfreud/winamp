#ifndef NULLSOFT_API_H
#define NULLSOFT_API_H

#include "../Agave/Config/api_config.h"
extern api_config *configApi;
#define AGAVE_API_CONFIG configApi

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgrApi;
#define WASABI_API_MEMMGR memmgrApi

#include <api/service/waServiceFactory.h>

#include "../Agave/Language/api_language.h"

#endif