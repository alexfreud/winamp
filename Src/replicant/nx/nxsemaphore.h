#pragma once
#if   defined(__ANDROID__)
#include "android/nxsemaphore.h"
#elif defined(_WIN32)
#include "win/nxsemaphore.h"
#elif defined(__APPLE__)
#include "osx/nxsemaphore.h"
#elif   defined(__linux__)
#include "linux/nxsemaphore.h"
#endif
