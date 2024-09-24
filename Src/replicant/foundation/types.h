#pragma once
#if defined(_WIN64) && defined(_M_X64)
#include "win-amd64/types.h"
#elif defined(_WIN32) && defined(_M_IX86)
#include "win-x86/types.h"
#elif defined(__APPLE__) && defined(__LP64__)
#include "osx-amd64/types.h"
#elif defined(__APPLE__) // TODO: && defined(__LP32__)
#include "osx-x86/types.h"
#elif defined(__ANDROID__)
#include "android-arm/types.h"
#elif defined(__linux__) && defined(__x86_64)
#include "linux-amd64/types.h"
#else
#error port me!
#endif
