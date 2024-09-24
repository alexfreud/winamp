#ifndef NULLSOFT_ML_WIRE_API_H
#define NULLSOFT_ML_WIRE_API_H

#include "../Winamp/JSAPI2_api_security.h"
extern JSAPI2::api_security *jsapi2_security;
#define AGAVE_API_JSAPI2_SECURITY jsapi2_security

#include "../Agave/Language/api_language.h"

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../omBrowser/obj_ombrowser.h"
extern obj_ombrowser *browserManager;
#define OMBROWSERMNGR browserManager

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi

#include "../Agave/ExplorerFindFile/api_explorerfindfile.h"

#endif // !NULLSOFT_ML_WIRE_API_H