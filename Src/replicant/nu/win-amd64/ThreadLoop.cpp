#include "ThreadLoop.h"

lifo_t ThreadLoop::procedure_cache = {0,};
lifo_t ThreadLoop::cache_bases= {0,};

#define PROCEDURE_CACHE_SEED 64
ThreadLoop::ThreadLoop()
{
	mpscq_init(&procedure_queue);
	procedure_notification = CreateSemaphore(0, 0, LONG_MAX, 0);
	kill_switch = CreateEvent(0, TRUE, FALSE, 0);
}

void ThreadLoop::RefillCache()
{
	threadloop_node_t *cache_seed = (threadloop_node_t *)lifo_malloc(PROCEDURE_CACHE_SEED*sizeof(threadloop_node_t));
	if (cache_seed)
	{
		memset(cache_seed, 0, PROCEDURE_CACHE_SEED*sizeof(threadloop_node_t));
		for (int i=0;i<PROCEDURE_CACHE_SEED;i++)
		{			
			lifo_push(&procedure_cache, &cache_seed[i]);
		}
		lifo_push(&cache_bases, cache_seed);
	}
}

void ThreadLoop::Run()
{
	HANDLE events[] = {kill_switch, procedure_notification};
	while (WaitForMultipleObjects(2, events, FALSE, INFINITE) == WAIT_OBJECT_0 + 1)
	{
		threadloop_node_t *apc;
		for (;;)
		{
			apc = (threadloop_node_t *)mpscq_pop(&procedure_queue);
			if (!apc)
			{
				Sleep(0);
				continue;
			}
			apc->func(apc->param1, apc->param2, apc->real_value);
		}
		lifo_push(&procedure_cache, apc);
	}
}

threadloop_node_t *ThreadLoop::GetAPC()
{
	threadloop_node_t *apc = 0;

	do 
	{
		apc = (threadloop_node_t *)lifo_pop(&procedure_cache);
		if (!apc)
			RefillCache();
	} while (!apc);
	return apc;
}

void ThreadLoop::Schedule(threadloop_node_t *apc)
{
	mpscq_push(&procedure_queue, apc);
	ReleaseSemaphore(procedure_notification, 1, 0);
}

void ThreadLoop::Kill()
{
	SetEvent(kill_switch);
}
