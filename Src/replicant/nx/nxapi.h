#pragma once
#if defined(__ANDROID__)
#include "android/nxapi.h"
#elif   defined(__linux__)
#include "linux/nxapi.h"
#elif defined(_WIN32)
#include "win/nxapi.h"
#elif defined(__APPLE__)
#include "osx/nxapi.h"
#else
#error port me!
#endif
