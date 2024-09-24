#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH

#include "../Agave/Config/api_config.h"
extern api_config *AGAVE_API_CONFIG;

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgrApi;
#define WASABI_API_MEMMGR memmgrApi

#include "../Agave/Language/api_language.h"

#include "../Components/wac_downloadManager/wac_downloadManager_api.h"

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi

#endif