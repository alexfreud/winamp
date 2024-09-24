#include "nxthread.h"
#include "foundation/error.h"

int NXThreadCreate(nx_thread_t *thread, nx_thread_func_t thread_function, nx_thread_parameter_t parameter)
{
	*thread = (nx_thread_t)CreateThread(0, 0, thread_function, parameter, 0, 0);
	return NErr_Success;
}

int NXThreadJoin(nx_thread_t t, nx_thread_return_t *retval)
{
	if (!t)
		return NErr_NullPointer;
	WaitForSingleObject((HANDLE)t, INFINITE);
	if (retval)
		GetExitCodeThread((HANDLE)t, retval);
	CloseHandle((HANDLE)t);
	return NErr_Success;
}

int NXThreadCurrentSetPriority(int priority)
{
	SetThreadPriority(GetCurrentThread(), priority);
	return NErr_Success;
}