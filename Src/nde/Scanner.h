#pragma once
#ifdef __APPLE__
#include "osx/Scanner.h"
#elif defined(_WIN32)
#include "win/Scanner.h"
#elif defined(__ANDROID__)
#include "android/Scanner.h"
#endif
