#pragma once
#ifdef __APPLE__
#include "osx/IndexField.h"
#elif defined(_WIN32)
#include "win/IndexField.h"
#elif defined(__ANDROID__)
#include "android/IndexField.h"
#endif
