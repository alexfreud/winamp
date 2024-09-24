#ifndef NULLSOFT_API_H
#define NULLSOFT_API_H

#include "../Agave/Agave.h"
#include <api/wnd/api_wnd.h>
#include <api/skin/api_skin.h>
#include <api/skin/api_palette.h>
#include "../Winamp/api_decodefile.h"
#ifndef IGNORE_API_GRACENOTE
#include "../gracenote/api_gracenote.h"
#endif


#include "api\service\api_service.h"
extern api_service *serviceApi;
#define WASABI_API_SVC serviceApi

#include "api\syscb\api_syscb.h"
extern api_syscb *syscbApi;
#define WASABI_API_SYSCB syscbApi

#include "..\Agave\Language\api_language.h"
//DECLARE_EXTERNAL_SERVICE(api_language,         WASABI_API_LNG);

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../Plugins/Library/ml_local/api_mldb.h"
extern api_mldb *mldbApi;
#define AGAVE_API_MLDB mldbApi

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi

#include <omBrowser\obj_ombrowser.h>
extern obj_ombrowser *browserManager;
#define OMBROWSERMNGR browserManager

#include "../Winamp/api_decodefile.h"
extern api_decodefile *decodeApi;
#define AGAVE_API_DECODE decodeApi

#include "api/wnd/wndapi.h"
extern wnd_api *wndApi;
#define WASABI_API_WND wndApi

#include <api/skin/api_skin.h>
extern api_skin *skinApi;
#define WASABI_API_SKIN skinApi

#include "../Agave/Config/api_config.h"

#include <api/skin/api_palette.h>
extern api_palette *paletteManagerApi;
#define WASABI_API_PALETTE paletteManagerApi


#ifndef IGNORE_API_GRACENOTE
DECLARE_EXTERNAL_SERVICE(api_gracenote,        AGAVE_API_GRACENOTE);
#endif

#include "../Winamp/JSAPI2_api_security.h"
extern JSAPI2::api_security *jsapi2_security;
#define AGAVE_API_JSAPI2_SECURITY jsapi2_security

#endif