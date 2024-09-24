#pragma once
#include "nx/nxapi.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

NX_API int NXSleep(unsigned int milliseconds);
NX_API int NXSleepYield(void);

#ifdef __cplusplus
}
#endif
