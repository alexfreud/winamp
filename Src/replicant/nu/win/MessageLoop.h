#pragma once
#include "foundation/types.h"
#include "nu/lfmpscq.h"
#include "nu/LockFreeLIFO.h"
#include <windows.h>

namespace nu
{

	/* you can inherit from message_node_t (or combine inside a struct)
	but make sure that your message isn't > 64 bytes */
	struct message_node_t : public queue_node_t
	{
		uint32_t message;
	};
	
	class MessageLoop
	{
	public:
		MessageLoop();
		~MessageLoop();
		/* API for Message senders */
		message_node_t *AllocateMessage(); // returns a message for you to fill out
		void PostMessage(message_node_t *message);	

		/* API for Message receivers */
		void FreeMessage(message_node_t *message);
		message_node_t *GetMessage(); // waits forever
		message_node_t *PeekMessage();
		message_node_t *PeekMessage(unsigned int milliseconds);
	private:
		void RefillCache();

		HANDLE message_notification;
		mpscq_t message_queue;

		/* Memory cache to be able to run APCs without having the memory manager lock 
		we'll allocate 100 at a time (#defined by MESSAGE_CACHE_SEED)
		and allocate new ones only if the cache is empty (which unfortunately will lock)
		cache_bases holds the pointers we've allocated (to free on destruction of this object)
		and message_cache holds the individual pointers */
		static lifo_t message_cache;
		static lifo_t cache_bases;
	};
}