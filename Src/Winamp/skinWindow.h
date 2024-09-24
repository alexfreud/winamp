#ifndef NULLOSFT_WINAMP_SKINNEDWINDOW_HEADER
#define NULLOSFT_WINAMP_SKINNEDWINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

BOOL SkinWindow(HWND hwndToSkin, REFGUID windowGuid, UINT flagsEx, FFCALLBACK callbackFF);

#endif //NULLOSFT_WINAMP_SKINNEDWINDOW_HEADER