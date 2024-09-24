#pragma once
#if defined(__ANDROID__)
#include "android/nxdata.h"
#elif   defined(__linux__)
#include "linux/nxdata.h"
#elif defined(_WIN32)
#include "win/nxdata.h"
#elif defined(__APPLE__)
#include "osx/nxdata.h"
#endif
