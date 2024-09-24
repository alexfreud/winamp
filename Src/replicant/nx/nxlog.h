#pragma once
#if defined(__ANDROID__)
#include "android/nxlog.h"
#elif   defined(__linux__)
#include "linux/nxlog.h"
#elif defined(_WIN32)
#include "win/nxlog.h"
#elif defined(__APPLE__)
#include "osx/nxlog.h"
#endif
