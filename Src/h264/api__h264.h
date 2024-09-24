#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH

#include <api/service/api_service.h>

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgrApi;
#define WASABI_API_MEMMGR memmgrApi

#include "../winamp/api_winamp.h"
extern api_winamp *winampApi;
#define AGAVE_API_WINAMP winampApi
#endif