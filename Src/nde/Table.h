#pragma once
#ifdef __APPLE__
#include "osx/Table.h"
#elif defined(_WIN32)
#include "win/Table.h"
#elif defined(__ANDROID__)
#include "android/Table.h"
#endif
