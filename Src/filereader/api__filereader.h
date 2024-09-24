#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH
#include <api/service/api_service.h>

extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../Agave/Config/api_config.h"
extern api_config *config;
#define AGAVE_API_CONFIG config
#endif