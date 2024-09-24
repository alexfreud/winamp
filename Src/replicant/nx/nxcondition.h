#pragma once
#if   defined(__ANDROID__)
#include "android/nxcondition.h"
#elif defined(_WIN32)
#include "win/nxcondition.h"
#elif defined(__APPLE__)
#include "osx/nxcondition.h"
#elif   defined(__linux__)
#include "linux/nxcondition.h"
#endif
