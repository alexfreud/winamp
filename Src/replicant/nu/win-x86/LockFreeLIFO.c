#include "LockFreeLIFO.h"
#include "foundation/atomics.h"

/* win32 implementation */

void lifo_init(lifo_t *lifo)
{
	lifo->head = 0;
	lifo->aba = 0;
}

#if 0 // defined in LockFreeLIFO.asm
void lifo_push(lifo_t *lifo, queue_node_t *cl)
{
	queue_node_t *new_head = cl;
	queue_node_t *old_head = 0;
	do
	{
		old_head = (queue_node_t *)lifo->head;
		new_head->Next = old_head;
	} while (!nx_atomic_cmpxchg_pointer(old_head, new_head, (void * volatile *)&lifo->head));
}

queue_node_t *lifo_pop(lifo_t *lifo)
{
	lifo_t old_head, new_head;
	do
	{
		old_head = *lifo;
		if (!old_head.head)
			return 0;

		new_head.head = old_head.head->Next;
		new_head.aba = old_head.aba+1;
	} while (!nx_atomic_cmpxchg2(*(int64_t *)&old_head, *(int64_t *)&new_head, (volatile int64_t *)&lifo->head));
	return (queue_node_t *)old_head.head;
}
#endif

queue_node_t *lifo_malloc(size_t bytes)
{
	return _aligned_malloc(bytes, MEMORY_ALLOCATION_ALIGNMENT);
}

void lifo_free(queue_node_t *ptr)
{
	_aligned_free(ptr);
}