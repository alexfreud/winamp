#pragma once
#ifdef __APPLE__
#include "osx/Vfs.h"
#elif defined(_WIN32)
#include "win/Vfs.h"
#elif defined(__ANDROID__)
#include "android/vfs.h"
#endif
