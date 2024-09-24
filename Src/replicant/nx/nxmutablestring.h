#pragma once
#if defined(__ANDROID__)
#include "android/nxmutablestring.h"
#elif   defined(__linux__)
#include "linux/nxmutablestring.h"
#elif defined(_WIN32)
#include "win/nxmutablestring.h"
#elif defined(__APPLE_)
#include "osx/nxmutablestring.h"
#endif
