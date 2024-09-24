#ifndef NULLSOFT_GEN_FF_API_H
#define NULLSOFT_GEN_FF_API_H

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgrApi;
#define WASABI_API_MEMMGR memmgrApi

#include <api/skin/api_colorthemes.h>
#define WASABI_API_COLORTHEMES colorThemesApi

#include <api/skin/api_palette.h>
extern api_palette *paletteManagerApi;
#define WASABI_API_PALETTE paletteManagerApi

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi


#endif // !NULLSOFT_GEN_FF_API_H
