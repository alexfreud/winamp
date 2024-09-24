#ifndef NULLSOFT_WEBDEV_PLUGIN_MENU_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_MENU_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ifc_omservice;

#define MENU_SERVICECONTEXT		0

HMENU Menu_GetMenu(UINT menuKind, ifc_omservice *service);
BOOL Menu_ReleaseMenu(HMENU hMenu, UINT menuKind);

#endif //NULLSOFT_WEBDEV_PLUGIN_MENU_HEADER