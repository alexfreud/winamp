
#pragma once
#include "nx/nxapi.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct nx_thread_struct_t { size_t dummy; } *nx_thread_t;
	typedef DWORD nx_thread_return_t;
	typedef void *nx_thread_parameter_t;
#define NXTHREADCALL CALLBACK
	typedef nx_thread_return_t (NXTHREADCALL *nx_thread_func_t)(nx_thread_parameter_t parameter);

	// TODO: add parameters for things like stack size
NX_API int NXThreadCreate(nx_thread_t *thread, nx_thread_func_t thread_function, nx_thread_parameter_t parameter);
NX_API int NXThreadJoin(nx_thread_t t, nx_thread_return_t *retval);

enum
{
	NX_THREAD_PRIORITY_PLAYBACK=THREAD_PRIORITY_HIGHEST, 
};
// sets priority of current thread
NX_API int NXThreadCurrentSetPriority(int priority);

#ifdef __cplusplus
}
#endif