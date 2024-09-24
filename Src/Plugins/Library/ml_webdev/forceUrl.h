#ifndef NULLSOFT_WEBDEV_PLUGIN_FORCE_URL_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_FORCE_URL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#include <wtypes.h>

HRESULT ForceUrl_Set(UINT serviceId, LPCWSTR pszUrl);
HRESULT ForceUrl_Get(UINT serviceId, const wchar_t **ppszUrl);
void ForceUrl_Remove();

#endif // NULLSOFT_WEBDEV_PLUGIN_FORCE_URL_HEADER