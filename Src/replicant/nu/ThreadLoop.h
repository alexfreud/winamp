#pragma once
#if defined(__ANDROID__) && defined(__ARM_ARCH_7A__)
#include "android-armv7/ThreadLoop.h"
#elif defined(__ANDROID__) && (defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5TE__))
#include "android-arm/ThreadLoop.h"
#elif defined(__ANDROID__) && defined(__i386__)
#include "android-x86/ThreadLoop.h"
#elif defined(_WIN32)
#include "win/ThreadLoop.h"
#elif   defined(__linux__)
#include "linux/ThreadLoop.h"
#elif defined(__APPLE__)
#include "osx/ThreadLoop.h"
#else
#error port me!
#endif
