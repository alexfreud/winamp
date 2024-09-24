#ifndef NULLSOFT_TATAKI_API_H
#define NULLSOFT_TATAKI_API_H

#include <api/service/api_service.h>
//extern api_service *serviceManager;
//#define WASABI_API_SVC serviceManager

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include <api/syscb/api_syscb.h>
#define WASABI_API_SYSCB sysCallbackApi

#include <api/font/api_font.h>
#define WASABI_API_FONT fontApi

#include <api/wnd/api_wnd.h>
#define WASABI_API_WND wndApi

#include <api/config/api_config.h>
#define WASABI_API_CONFIG configApi

#include <api/imgldr/api_imgldr.h>
#define WASABI_API_IMGLDR imgLoaderApi

#include <Agave/Random/api_random.h>
extern api_random *randomApi;
#define AGAVE_API_RANDOM randomApi

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memoryManager;
#define WASABI_API_MEMMGR memoryManager

#include <api/skin/api_skin.h>
#define WASABI_API_SKIN skinApi

#include <api/skin/api_palette.h>
extern api_palette *paletteManagerApi;
#define WASABI_API_PALETTE paletteManagerApi

#endif