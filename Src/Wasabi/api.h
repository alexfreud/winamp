#ifndef NULLSOFT_WASABI_API_H
#define NULLSOFT_WASABI_API_H

#include <api/syscb/api_syscb.h>
#define WASABI_API_SYSCB sysCallbackApi

#include <api/wndmgr/api_wndmgr.h>
#define WASABI_API_WNDMGR wndManagerApi

#include "api/config/api_config.h"
#define WASABI_API_CONFIG configApi

#include "api/service/api_service.h"

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include <api/skin/api_skin.h>
#define WASABI_API_SKIN skinApi

#include <api/script/api_maki.h>
#define WASABI_API_MAKI makiApi

#include <api/wnd/api_wnd.h>
#define WASABI_API_WND wndApi

#include <api/timer/api_timer.h>
#define WASABI_API_TIMER timerApi

#include <api/font/api_font.h>
#define WASABI_API_FONT fontApi

#include <api/imgldr/api_imgldr.h>
#define WASABI_API_IMGLDR imgLoaderApi

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgrApi;
#define WASABI_API_MEMMGR memmgrApi

#include <api/filereader/api_filereader.h>
#define WASABI_API_FILE fileApi

#include <api/skin/api_colorthemes.h>
#define WASABI_API_COLORTHEMES colorThemesApi

#include <api/skin/api_palette.h>
extern api_palette *paletteManagerApi;
#define WASABI_API_PALETTE paletteManagerApi

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi

#endif