#ifndef NULLSOFT_NOWPLAYING_PLUGIN_MENU_HEADER
#define NULLSOFT_NOWPLAYING_PLUGIN_MENU_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define MENU_NAVIGATIONCONTEXT		0

HMENU Menu_GetMenu(UINT menuKind);
BOOL Menu_ReleaseMenu(HMENU hMenu, UINT menuKind);

#endif //NULLSOFT_NOWPLAYING_PLUGIN_MENU_HEADER