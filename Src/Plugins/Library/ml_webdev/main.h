#ifndef NULLSOFT_WEBDEV_PLUGIN_MAIN_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_MAIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../General/gen_ml/ml.h"
#include "./common.h"

#define PLUGIN_VERSION_MAJOR		2
#define PLUGIN_VERSION_MINOR		34

// {BF4F80A7-7470-4b08-8A4C-34382C146202}
DEFINE_GUID(WebDevLangUid, 0xbf4f80a7, 0x7470, 0x4b08, 0x8a, 0x4c, 0x34, 0x38, 0x2c, 0x14, 0x62, 0x2);

HINSTANCE Plugin_GetInstance(void);
HWND Plugin_GetWinamp(void);
HWND Plugin_GetLibrary(void);

class Navigation;
HRESULT Plugin_GetNavigation(Navigation **instance);

#endif //NULLSOFT_WEBDEV_PLUGIN_MAIN_HEADER