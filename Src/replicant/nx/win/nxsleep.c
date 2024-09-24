#include "nxsleep.h"
#include "foundation/error.h"
int NXSleep(unsigned int milliseconds)
{
	Sleep(milliseconds);
	return NErr_Success;
}

int NXSleepYield(void)
{
	Sleep(0);
	return NErr_Success;
}

