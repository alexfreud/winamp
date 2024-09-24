#pragma once
#if defined(__ANDROID__) && defined(__ARM_ARCH_7A__)
#include "android-armv7/MessageLoop.h"
#elif defined(__ANDROID__) && (defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5TE__))
#include "android-arm/MessageLoop.h"
#elif defined(__ANDROID__) && defined(__i386__)
#include "android-x86/MessageLoop.h"
#elif defined(_WIN32)
#include "win/MessageLoop.h"
#elif   defined(__linux__)
#include "linux/MessageLoop.h"
#elif defined(__APPLE__)
#include "osx/MessageLoop.h"
#else
#error port me!
#endif
