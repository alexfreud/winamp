#pragma once
#include "foundation/types.h"
#include "nu/queue_node.h"
#include "foundation/align.h"
/* lock free stack object 
multiple threads can push and pop without locking
note that order is not guaranteed.  that is, if Thread 1 calls Insert before Thread 2, Thread 2's item might still make it in before.
*/
#ifdef __cplusplus
extern "C" {
#endif

	typedef NALIGN(8) struct lifo_struct_t
	{
		volatile queue_node_t *head;
		uint32_t aba;
	} lifo_t;

	/* use this to allocate an object that will go into this */
	queue_node_t *lifo_malloc(size_t bytes); 
	void lifo_free(queue_node_t *ptr);

	void lifo_init(lifo_t *lifo);
	void lifo_push(lifo_t *lifo, queue_node_t *cl);
	queue_node_t *lifo_pop(lifo_t *lifo);

	

#ifdef __cplusplus
}
#endif
