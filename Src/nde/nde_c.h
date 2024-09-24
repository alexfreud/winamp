#pragma once
#ifdef __APPLE__
#include "osx/nde_c.h"
#elif defined(_WIN32)
#include "win/nde_c.h"
#elif defined(__ANDROID__)
#include "android/nde_c.h"
#endif
