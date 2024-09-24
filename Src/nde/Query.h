#pragma once
#ifdef __APPLE__
#include "osx/Query.h"
#elif defined(_WIN32)
#include "win/Query.h"
#elif defined(__ANDROID__)
#include "android/Query.h"
#endif
