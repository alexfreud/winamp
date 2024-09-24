#include "ThreadFunctions.h"
#include "threadpool_types.h"

ThreadFunctions::ThreadFunctions(int create_function_list)
{
	if (create_function_list)
	{
		functions_semaphore = CreateSemaphore(0, 0, ThreadPoolTypes::MAX_SEMAPHORE_VALUE, 0);
		InitializeCriticalSectionAndSpinCount(&functions_guard, 200);
	}
	else
		functions_semaphore = 0;
}

ThreadFunctions::~ThreadFunctions()
{
	if (functions_semaphore)
	{
		CloseHandle(functions_semaphore);
		DeleteCriticalSection(&functions_guard);
	}
}

void ThreadFunctions::Add(HANDLE handle, api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id)
{
	Nullsoft::Utility::AutoLock l(guard);
	Data *new_data = (Data *)calloc(1, sizeof(Data));
	new_data->func = func;
	new_data->user_data = user_data;
	new_data->id = id;
	data[handle] = new_data;
}

bool ThreadFunctions::Get(HANDLE handle, api_threadpool::ThreadPoolFunc *func, void **user_data, intptr_t *id)
{
	Nullsoft::Utility::AutoLock l(guard);
	DataMap::iterator found = data.find(handle);
	if (found == data.end())
		return false;

	const Data *d = found->second;
	*func = d->func;
	*user_data = d->user_data;
	*id = d->id;
	return true;
}

void ThreadFunctions::QueueFunction(api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id)
{
	Data *new_data = (Data *)calloc(1, sizeof(Data));
	new_data->func = func;
	new_data->user_data = user_data;
	new_data->id = id;
	EnterCriticalSection(&functions_guard);
	functions_list.push_front(new_data);
	LeaveCriticalSection(&functions_guard); // unlock before releasing the semaphore early so we don't lock convoy
	ReleaseSemaphore(functions_semaphore, 1, 0);
}

bool ThreadFunctions::PopFunction(api_threadpool::ThreadPoolFunc *func, void **user_data, intptr_t *id)
{
	EnterCriticalSection(&functions_guard);
	if (!functions_list.empty())
	{
		ThreadFunctions::Data *data = functions_list.back();
		functions_list.pop_back();
		LeaveCriticalSection(&functions_guard);
		*func = data->func;
		*user_data = data->user_data;
		*id = data->id;
		free(data);
		return true;
	}
	else
	{
		LeaveCriticalSection(&functions_guard);
		return false;
	}
}
