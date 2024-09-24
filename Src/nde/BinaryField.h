#pragma once
#ifdef __APPLE__
#include "osx/BinaryField.h"
#elif defined(_WIN32)
#include "win/BinaryField.h"
#elif defined(__ANDROID__)
#include "android/BinaryField.h"
#endif
