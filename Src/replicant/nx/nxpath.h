#pragma once
#if   defined(__linux__)
#include "linux/nxpath.h"
#elif defined(_WIN32)
#include "win/nxpath.h"
#elif defined(__APPLE__)
#include "osx/nxpath.h"
#endif
