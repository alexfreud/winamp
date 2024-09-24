#ifndef _NULLSOFT_WINAMP_ML_DEVICES_MAIN_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_MAIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// #define _CRTDBG_MAP_ALLOC

#include <wtypes.h>

#include "./common.h"
#include "./strings.h"
#include "./plugin.h"
#include "./local_menu.h"
#include "./graphics.h"
#include "./image.h"
#include "./imageCache.h"
#include "./fillRegion.h"
#include "./backBuffer.h"
#include "./wasabi.h"
#include "./config.h"
#include "./navigation.h"
#include "./navigationIcons.h"
#include "./managerView.h"
#include "./statusBar.h"
#include "./widgetHost.h"
#include "./widgetStyle.h"
#include "./widget.h"
#include "./infoWidget.h"
#include "./welcomeWidget.h"
#include "./listWidget.h"
#include "./deviceManagerHandler.h"
#include "./deviceHandler.h"
#include "./eventRelay.h"
#include "./deviceCommands.h"
#include "./resource.h"
#include "./embeddedEditor.h"

#include "../../General/gen_ml/ml.h"
#include "../../General/gen_ml/menu.h"
#include "../../General/gen_ml/ml_ipc_0313.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/wa_dlg.h"
#include "../nu/trace.h"

#include <shlwapi.h>
#include <math.h>

#endif //_NULLSOFT_WINAMP_ML_DEVICES_MAIN_HEADER
