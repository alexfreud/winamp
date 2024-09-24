#pragma once
#include "nx/nxapi.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nx_condition_struct_t 
{ 
	CONDITION_VARIABLE condition;
	CRITICAL_SECTION mutex;
} nx_condition_value_t, *nx_condition_t;

NX_API int NXConditionInitialize(nx_condition_t condition);
NX_API int NXConditionDestroy(nx_condition_t condition);
NX_API int NXConditionLock(nx_condition_t condition);
NX_API int NXConditionUnlock(nx_condition_t condition);
NX_API int NXConditionWait(nx_condition_t condition);
NX_API int NXConditionTimedWait(nx_condition_t condition, unsigned int milliseconds);
NX_API int NXConditionSignal(nx_condition_t condition);

#ifdef __cplusplus
}
#endif