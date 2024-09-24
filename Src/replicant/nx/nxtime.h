#pragma once
#if defined(__ANDROID__)
#include "android/nxtime.h"
#elif   defined(__linux__)
#include "linux/nxtime.h"
#elif defined(_WIN32)
#include "win/nxtime.h"
#elif defined(__APPLE__)
#include "osx/nxtime.h"
#endif
