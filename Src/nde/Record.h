#pragma once
#ifdef __APPLE__
#include "osx/Record.h"
#elif defined(_WIN32)
#include "win/Record.h"
#elif defined(__ANDROID__)
#include "android/Record.h"
#endif
