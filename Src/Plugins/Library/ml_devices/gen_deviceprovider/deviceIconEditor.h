#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_ICON_EDITOR_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_ICON_EDITOR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef struct DeviceIconInfo
{
	int width;
	int height;
	wchar_t *path;
} DeviceIconInfo;

INT_PTR 
DeviceIconEditor_Show(HWND parentWindow, 
							 DeviceIconInfo *iconInfo);


#endif //_NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_ICON_EDITOR_HEADER