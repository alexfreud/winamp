#pragma once
#if defined(__ANDROID__) && defined(__ARM_ARCH_7A__)
#include "android-armv7/nxonce.h"
#elif defined(__ANDROID__)
#include "android-armv5/nxonce.h"
#elif defined(_WIN32)
#include "win/nxonce.h"
#elif defined(__APPLE__) && defined(__amd64__)
#include "osx-amd64/nxonce.h"
#elif defined(__APPLE__) && defined(__i386__)
#include "osx-x86/nxonce.h"
#elif   defined(__linux__)
#include "linux/nxonce.h"
#endif
