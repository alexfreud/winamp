#pragma once
#ifdef __APPLE__
#include "osx/Binary32Field.h"
#elif defined(_WIN32)
#include "win/Binary32Field.h"
#elif defined(__ANDROID__)
#include "android/Binary32Field.h"
#endif
