#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager


#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi


#include <api/syscb/api_syscb.h>
#define WASABI_API_SYSCB sysCallbackApi

#include <api/service/waServiceFactory.h>

#include "../Agave/Language/api_language.h"

#endif