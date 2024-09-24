#pragma once
#if defined(__ANDROID__)
#include "android/nxuri.h"
#elif   defined(__linux__)
#include "linux/nxuri.h"
#elif defined(_WIN32)
#include "win/nxuri.h"
#elif defined(__APPLE__)
#include "osx/nxuri.h"
#endif
