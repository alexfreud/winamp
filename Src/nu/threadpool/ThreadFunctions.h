#pragma once
#include "api_threadpool.h"
#include <map>
#include <deque>
#include "../AutoLock.h"

class ThreadFunctions
{
public:
	struct Data
	{
		api_threadpool::ThreadPoolFunc func;
		void *user_data;
		intptr_t id;
	};
	ThreadFunctions(int create_function_list=1);
	~ThreadFunctions();
	void Add(HANDLE handle, api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id);
	bool Get(HANDLE handle, api_threadpool::ThreadPoolFunc *func, void **user_data, intptr_t *id);
	void QueueFunction(api_threadpool::ThreadPoolFunc func, void *user_data, intptr_t id);
	bool PopFunction(api_threadpool::ThreadPoolFunc *func, void **user_data, intptr_t *id);

	typedef std::map<HANDLE, const ThreadFunctions::Data*> DataMap;
	DataMap data;
	Nullsoft::Utility::LockGuard guard;

	typedef std::deque<ThreadFunctions::Data*> FuncList;
	FuncList functions_list;
	CRITICAL_SECTION functions_guard;
	HANDLE functions_semaphore;
};
