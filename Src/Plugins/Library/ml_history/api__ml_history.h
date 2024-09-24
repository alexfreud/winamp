#ifndef NULLSOFT_ML_HISTORY_API_H
#define NULLSOFT_ML_HISTORY_API_H

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../Winamp/JSAPI2_api_security.h"
extern JSAPI2::api_security *jsapi2_security;
#define AGAVE_API_JSAPI2_SECURITY jsapi2_security

#include <api/service/waServiceFactory.h>

#include "../Agave/Language/api_language.h"

#include "../Agave/ExplorerFindFile/api_explorerfindfile.h"

#endif  // !NULLSOFT_ML_HISTORY_API_H