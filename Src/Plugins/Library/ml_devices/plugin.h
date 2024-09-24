#ifndef _NULLSOFT_WINAMP_ML_DEVICES_PLUGIN_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_PLUGIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../Plugins/General/gen_ml/ml.h"
#include "./imageCache.h"
#include "./deviceManagerHandler.h"
#include "./deviceHandler.h"

#define PLUGIN_VERSION_MAJOR		1
#define PLUGIN_VERSION_MINOR		35

typedef void (CALLBACK *PluginUnloadCallback)(void);

HINSTANCE Plugin_GetInstance(void);
HWND Plugin_GetWinampWindow(void);
HWND Plugin_GetLibraryWindow(void);
BOOL Plugin_RegisterUnloadCallback(PluginUnloadCallback callback);
DeviceImageCache *Plugin_GetImageCache();
HWND Plugin_GetEventRelayWindow();
const wchar_t *Plugin_GetDefaultDeviceImage(unsigned int width, unsigned int height);
HRESULT Plugin_EnsurePathExist(const wchar_t *path);
BOOL Plugin_GetResourceString(const wchar_t *resourceName, const wchar_t *resourceType, wchar_t *buffer, size_t bufferMax);
HMENU Plugin_LoadMenu();
BOOL Plugin_ShowHelp();
BOOL Plugin_BeginDiscovery();
BOOL Plugin_OpenUrl(HWND ownerWindow, const wchar_t *url, BOOL forceExternal);

#endif //_NULLSOFT_WINAMP_ML_DEVICES_PLUGIN_HEADER