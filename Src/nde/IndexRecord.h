#pragma once
#ifdef __APPLE__
#include "osx/IndexRecord.h"
#elif defined(_WIN32)
#include "win/IndexRecord.h"
#elif defined(__ANDROID__)
#include "android/IndexRecord.h"
#endif
