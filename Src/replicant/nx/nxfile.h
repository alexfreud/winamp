#pragma once
#if defined(__ANDROID__)
#include "android/nxfile.h"
#elif   defined(__linux__)
#include "linux/nxfile.h"
#elif defined(_WIN32)
#include "win/nxfile.h"
#elif defined(__APPLE__)
#include "osx/nxfile.h"
#endif
