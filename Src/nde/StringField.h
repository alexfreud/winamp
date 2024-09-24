#pragma once
#ifdef __APPLE__
#include "osx/StringField.h"
#elif defined(_WIN32)
#include "win/StringField.h"
#elif defined(__ANDROID__)
#include "android/StringField.h"
#endif
