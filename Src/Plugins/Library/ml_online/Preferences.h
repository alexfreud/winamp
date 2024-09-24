#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_PREFERENCES_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_PREFERENCES_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define MAXBANDWIDTH  3500

BOOL Preferences_Register();
void Preferences_Unregister();
BOOL Preferences_Show();

#endif // NULLOSFT_ONLINEMEDIA_PLUGIN_PREFERENCES_HEADER
