#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_MESSAGEBOX_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_MESSAGEBOX_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


INT OmMessageBox(HWND hParent, LPCWSTR pszText, LPCWSTR pszCaption, UINT uType, LPCWSTR pszCheck, INT *checked); 

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_MESSAGEBOX_HEADER