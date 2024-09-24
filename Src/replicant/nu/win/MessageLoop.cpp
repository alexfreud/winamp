#include "MessageLoop.h"
#include <assert.h>

lifo_t nu::MessageLoop::message_cache = {0,};
lifo_t nu::MessageLoop::cache_bases= {0,};

#define MESSAAGE_CACHE_SEED 64

typedef uint8_t message_data_t[64]; // ensure all messages are this size

nu::MessageLoop::MessageLoop()
{
	mpscq_init(&message_queue);
	message_notification = CreateEvent(0, FALSE, FALSE, 0);
}

nu::MessageLoop::~MessageLoop()
{
	CloseHandle(message_notification);
}

void nu::MessageLoop::RefillCache()
{
	message_data_t *cache_seed = (message_data_t *)_aligned_malloc(MESSAAGE_CACHE_SEED*sizeof(message_data_t), 64);
	
	if (cache_seed)
	{
		int i=MESSAAGE_CACHE_SEED;
		while (--i)
		{
			lifo_push(&message_cache, (queue_node_t *)&cache_seed[i]);
		}
		lifo_push(&cache_bases, (queue_node_t *)cache_seed);
	}
	else
	{
		Sleep(0); // yield and hope that someone else pops something off soon		
	}
}

nu::message_node_t *nu::MessageLoop::AllocateMessage()
{
	message_node_t *apc = 0;

	do 
	{
		apc = (message_node_t *)lifo_pop(&message_cache);
		if (!apc)
			RefillCache();
	} while (!apc);
	return apc;
}

void nu::MessageLoop::PostMessage(nu::message_node_t *message)
{
	if (mpscq_push(&message_queue, message) == 0)
		SetEvent(message_notification);
}

void nu::MessageLoop::FreeMessage(nu::message_node_t *message)
{
	lifo_push(&message_cache, message);
}

nu::message_node_t *nu::MessageLoop::GetMessage()
{
	message_node_t *message = PeekMessage();
	if (message)
	{
		return message;
	}

	while (WaitForSingleObject(message_notification, INFINITE) ==  WAIT_OBJECT_0)
	{
		message = PeekMessage();
		if (message)
		{
			return message;
		}
	}
	return 0;
}

nu::message_node_t *nu::MessageLoop::PeekMessage()
{
	for (;;) // loop because we need to handle 'busy' from the queue
	{
		message_node_t *message = (message_node_t *)mpscq_pop(&message_queue);
		if (message == (message_node_t *)1) /* special return value that indicates a busy list */
		{
			// benski> although it's tempting to return 0 here, doing so will mess up the Event logic
			Sleep(0); // yield so that the thread that got pre-empted during push can finish
		}
		else
		{
			if (message)
			{
				return message;
			}
			else
			{
				return 0;
			}
		}
	}				
}

nu::message_node_t *nu::MessageLoop::PeekMessage(unsigned int milliseconds)
{
	message_node_t *message = PeekMessage();
	if (message)
		return message;

	if (WaitForSingleObject(message_notification, milliseconds) == WAIT_OBJECT_0)
	{
		message = PeekMessage();
		if (message)
			return message;
	}
	return 0;
}
