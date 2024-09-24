#pragma once
#if defined(_WIN64) && defined(_M_X64)
#include "win-amd64/atomics.h"
#elif defined(_WIN32) && defined(_M_IX86)
#include "win-x86/atomics.h"
#elif defined(__APPLE__) && defined(__amd64__)
#include "osx-amd64/atomics.h"
#elif defined(__APPLE__) && defined(__i386__)
#include "osx-x86/atomics.h"
#elif defined(__ANDROID__) && defined(__ARM_ARCH_7A__)
#include "android-armv7/atomics.h"
#elif defined(__ANDROID__) && (defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5TE__))
#include "android-arm/atomics.h"
#elif defined(__ANDROID__) && defined(__i386__)
#include "android-x86/atomics.h"
#elif defined(__linux__) && defined(__x86_64)
#include "linux-amd64/atomics.h"
#else
#error Port Me!
#endif
