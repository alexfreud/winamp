#include "nxsemaphore.h"
#include "foundation/error.h"
int NXSemaphoreCreate(nx_semaphore_t *sem)
{
	*sem = CreateSemaphore(0, 0, LONG_MAX, 0);
	return NErr_Success;
}

int NXSemaphoreRelease(nx_semaphore_t sem)
{
	ReleaseSemaphore(sem, 1, 0);
	return NErr_Success;
}

int NXSemaphoreWait(nx_semaphore_t sem)
{
	WaitForSingleObject(sem, INFINITE);
	return NErr_Success;
}

void NXSemaphoreClose(nx_semaphore_t sem)
{
	CloseHandle(sem);
}