#pragma once
#include "nxapi.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#if 0 && _WIN32_WINNT >= 0x600
	typedef INIT_ONCE nx_once_value_t;
	typedef INIT_ONCE *nx_once_t;
#define NX_ONCE_INITIALIZE INIT_ONCE_STATIC_INIT
#define NX_ONCE_API CALLBACK
	
NX_API void NXOnce(nx_once_t once, int (NX_ONCE_API *init_fn)(nx_once_t, void *, void **), void *param); 
NX_API void NXOnceInit(nx_once_t once);
#else
	typedef struct nx_once_s 
	{
		volatile int status;
		CRITICAL_SECTION critical_section;
	} nx_once_value_t, *nx_once_t;
	
#define NX_ONCE_API

	NX_API void NXOnce(nx_once_t once, int (NX_ONCE_API *init_fn)(nx_once_t, void *, void **), void *); 
	NX_API void NXOnceInit(nx_once_t once);

#endif
#ifdef __cplusplus
}
#endif