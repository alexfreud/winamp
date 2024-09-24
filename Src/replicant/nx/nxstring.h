#pragma once
#if defined(__ANDROID__)
#include "android/nxstring.h"
#elif   defined(__linux__)
#include "linux/nxstring.h"
#elif defined(_WIN32)
#include "win/nxstring.h"
#elif defined(__APPLE__)
#include "osx/nxstring.h"
#endif
