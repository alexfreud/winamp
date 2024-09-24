#pragma once
#if defined(__ANDROID__) && defined(__ARM_ARCH_7A__)
#include "android-armv7/LockFreeLIFO.h"
#elif defined(__ANDROID__)
#include "android-arm/LockFreeLIFO.h"
#elif defined(_WIN32) && defined(_M_IX86)
#include "win-x86/LockFreeLIFO.h"
#elif defined(_WIN32) && defined(_M_X64)
#include "win-amd64/LockFreeLIFO.h"
#elif defined(__APPLE__) && defined(__amd64__)
#include "osx-amd64/LockFreeLIFO.h"
#elif defined(__APPLE__) && defined(__i386__)
#include "osx-x86/LockFreeLIFO.h"
#elif defined(__linux__)
#include "linux/LockFreeLIFO.h"
#else
#error port me!
#endif
