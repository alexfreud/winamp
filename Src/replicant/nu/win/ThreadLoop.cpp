#include "ThreadLoop.h"
#include <limits.h>

lifo_t ThreadLoop::procedure_cache = {0,};
lifo_t ThreadLoop::cache_bases= {0,};

#define PROCEDURE_CACHE_SEED 64
ThreadLoop::ThreadLoop()
{
	mpscq_init(&procedure_queue);
	procedure_notification = CreateSemaphoreW(0, 0, LONG_MAX, 0);
	kill_switch = CreateEvent(0, TRUE, FALSE, 0);
}

ThreadLoop::~ThreadLoop()
{
	CloseHandle(procedure_notification);
	CloseHandle(kill_switch);
}

void ThreadLoop::RefillCache()
{
	threadloop_node_t *cache_seed = (threadloop_node_t *)malloc(PROCEDURE_CACHE_SEED*sizeof(threadloop_node_t));
	
	if (cache_seed)
	{
		int i=PROCEDURE_CACHE_SEED;
		while (--i)
		{
			lifo_push(&procedure_cache, (queue_node_t *)&cache_seed[i]);
		}
		lifo_push(&cache_bases, (queue_node_t *)cache_seed);
	}
	else
	{
		Sleep(0); // yield and hope that someone else pops something off soon		
	}
}

void ThreadLoop::Run()
{
	HANDLE events[] = {kill_switch, procedure_notification};
	while (WaitForMultipleObjects(2, events, FALSE, INFINITE) == WAIT_OBJECT_0 + 1)
	{
		for (;;)
		{
			threadloop_node_t *apc = (threadloop_node_t *)mpscq_pop(&procedure_queue);
			if (apc == (threadloop_node_t *)1) /* special return value that indicates a busy list */
			{
				Sleep(0); // yield so that the thread that got pre-empted during push can finish
			}
			else
			{
				if (apc)
				{
					apc->func(apc->param1, apc->param2, apc->real_value);
					lifo_push(&procedure_cache, apc);
				}
				else
				{
					break;
				}
			}
		}
	}
}

void ThreadLoop::Step(unsigned int milliseconds)
{
	HANDLE events[] = {kill_switch, procedure_notification};
	if (WaitForMultipleObjects(2, events, FALSE, milliseconds) == WAIT_OBJECT_0 + 1)
	{
		for (;;)
		{
			threadloop_node_t *apc = (threadloop_node_t *)mpscq_pop(&procedure_queue);
			if (apc == (threadloop_node_t *)1) /* special return value that indicates a busy list */
			{
				Sleep(0); // yield so that the thread that got pre-empted during push can finish
			}
			else
			{
				if (apc)
				{
					apc->func(apc->param1, apc->param2, apc->real_value);
					lifo_push(&procedure_cache, apc);
				}
				else
				{
					break;
				}
			}
		}
	}
}

void ThreadLoop::Step()
{
	HANDLE events[] = {kill_switch, procedure_notification};
	if (WaitForMultipleObjects(2, events, FALSE, INFINITE) == WAIT_OBJECT_0 + 1)
	{
		for (;;)
		{
			threadloop_node_t *apc = (threadloop_node_t *)mpscq_pop(&procedure_queue);
			if (apc == (threadloop_node_t *)1) /* special return value that indicates a busy list */
			{
				Sleep(0); // yield so that the thread that got pre-empted during push can finish
			}
			else
			{
				if (apc)
				{
					apc->func(apc->param1, apc->param2, apc->real_value);
					lifo_push(&procedure_cache, apc);
				}
				else
				{
					break;
				}
			}
		}
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
	if (mpscq_push(&procedure_queue, apc) == 0)
		ReleaseSemaphore(procedure_notification, 1, 0);
}

void ThreadLoop::Kill()
{
	SetEvent(kill_switch);
}
