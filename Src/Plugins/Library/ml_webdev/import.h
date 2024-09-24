#ifndef NULLSOFT_WEBDEV_PLUGIN_IMPORT_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_IMPORT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

HRESULT ImportService_GetFileSupported();
HRESULT ImportService_GetUrlSupported();

HRESULT ImportService_FromFile(HWND hOwner);
HRESULT ImportService_FromUrl(HWND hOwner);

void ImportService_SaveRecentUrl();

#endif //NULLSOFT_WEBDEV_PLUGIN_IMPORT_HEADER