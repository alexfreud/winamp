#pragma once
#include "foundation/types.h"
#include "nx/nxapi.h"

#ifdef __cplusplus
extern "C" {
#endif

NX_API void NXLog(int priority, char *fmt, ...);

#ifdef __cplusplus
}
#endif
