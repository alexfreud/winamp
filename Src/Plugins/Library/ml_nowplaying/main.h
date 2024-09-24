#ifndef NULLSOFT_NOWPLAYING_PLUGIN_MAIN_HEADER
#define NULLSOFT_NOWPLAYING_PLUGIN_MAIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../gen_ml/ml.h"
#include "./common.h"

#define PLUGIN_VERSION_MAJOR		4
#define PLUGIN_VERSION_MINOR		1

HINSTANCE Plugin_GetInstance(void);
HWND Plugin_GetWinamp(void);
HWND Plugin_GetLibrary(void);

#include "../ml_online/config.h"
extern C_Config *g_config;

#endif //NULLSOFT_NOWPLAYING_PLUGIN_MAIN_HEADER