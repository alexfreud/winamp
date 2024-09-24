#include "nxcondition.h"
#include "foundation/error.h"

int NXConditionInitialize(nx_condition_t condition)
{
	if (condition == 0)
		return NErr_NullPointer;

	InitializeCriticalSection(&condition->mutex);
	InitializeConditionVariable(&condition->condition);
	return NErr_Success;
}

int NXConditionDestroy(nx_condition_t condition)
{
	if (condition == 0)
		return NErr_NullPointer;

	DeleteCriticalSection(&condition->mutex);
	return NErr_Success;
}

int NXConditionLock(nx_condition_t condition)
{
	if (condition == 0)
		return NErr_NullPointer;

	EnterCriticalSection(&condition->mutex);
	return NErr_Success;
}

int NXConditionUnlock(nx_condition_t condition)
{
if (condition == 0)
		return NErr_NullPointer;

	LeaveCriticalSection(&condition->mutex);
	return NErr_Success;
}

int NXConditionWait(nx_condition_t condition)
{
	if (condition == 0)
		return NErr_NullPointer;

	SleepConditionVariableCS(&condition->condition, &condition->mutex, INFINITE);
	return NErr_Success;
}

int NXConditionTimedWait(nx_condition_t condition, unsigned int milliseconds)
{
	if (condition == 0)
		return NErr_NullPointer;

	SleepConditionVariableCS(&condition->condition, &condition->mutex, milliseconds);
	return NErr_Success;
}

int NXConditionSignal(nx_condition_t condition)
{
		if (condition == 0)
		return NErr_NullPointer;

	WakeConditionVariable(&condition->condition);
	return NErr_Success;
}