#ifndef NULLSOFT_PMP_IPOD_API_H
#define NULLSOFT_PMP_IPOD_API_H

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgr;
#define WASABI_API_MEMMGR memmgr

#include "../Agave/AlbumArt/api_albumart.h"
extern api_albumart *albumArtApi;
#define AGAVE_API_ALBUMART albumArtApi

#include "../Agave/Config/api_config.h"
extern api_config *agaveConfigApi;
#define AGAVE_API_CONFIG agaveConfigApi

#include "../Agave/Language/api_language.h"

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../devices/api_devicemanager.h"
extern api_devicemanager *deviceManagerApi;
#define AGAVE_API_DEVICEMANAGER deviceManagerApi

#endif