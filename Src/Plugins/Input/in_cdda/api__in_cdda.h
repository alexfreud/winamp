#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../Agave/Config/api_config.h"
extern api_config *agaveConfigApi;
#define AGAVE_API_CONFIG agaveConfigApi

#include "../Agave/Language/api_language.h"

#ifndef IGNORE_API_GRACENOTE
#include "../gracenote/api_gracenote.h"
extern api_gracenote *gracenoteApi;
#define AGAVE_API_GRACENOTE gracenoteApi
#endif

#include <api/service/waServiceFactory.h>

#endif NULLSOFT_APIH