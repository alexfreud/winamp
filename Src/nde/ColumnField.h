#pragma once
#ifdef __APPLE__
#include "osx/ColumnField.h"
#elif defined(_WIN32)
#include "win/ColumnField.h"
#elif defined(__ANDROID__)
#include "android/ColumnField.h"
#endif
