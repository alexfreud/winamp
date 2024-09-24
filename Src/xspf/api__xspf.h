#ifndef NULLSOFT_XSPF_API_H
#define NULLSOFT_XSPF_API_H

// Service Manager
#include "api/service/api_service.h"

// Media Library API
#include "../Plugins/Library/ml_local/api_mldb.h"
extern api_mldb *mldbApi;
#define AGAVE_API_MLDB mldbApi

#include "../Agave/Language/api_language.h"

#include "../tagz/api_tagz.h"
extern api_tagz *tagzApi;
#define AGAVE_API_TAGZ tagzApi

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#endif