#ifndef _NULLSOFT_WINAMP_ML_DEVICES_NAVIGATION_ICONS_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_NAVIGATION_ICONS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

int
NavigationIcons_GetDeviceIconIndex(ifc_device *device);

BOOL
NavigationIcons_ReleaseIconIndex(int iconIndex);

void
NavigationIcons_ClearCache();



#endif //_NULLSOFT_WINAMP_ML_DEVICES_NAVIGATION_ICONS_HEADER