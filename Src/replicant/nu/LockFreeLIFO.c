#include "LockFreeLIFO.h"
#include "foundation/atomics.h"

/* TODO: on windows, replace with InitializeSListHead/InterlockedPushEntrySList/InterlockedPopEntrySList just to be safe */
void lifo_init(lifo_t *lifo)
{
	lifo->head = 0;
}

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
	queue_node_t *new_head = 0, *old_head = 0;
	do
	{
		old_head = (queue_node_t *)lifo->head;
		if (old_head)
			new_head = old_head->Next;
		else
			new_head = 0;
	} while (!nx_atomic_cmpxchg_pointer(old_head, new_head, (void * volatile *)&lifo->head));
	return old_head;
}

	queue_node_t *lifo_malloc(size_t bytes)
	{
#ifdef __GNUC__
#   ifdef __APPLE__
		void *v = 0;
		(void) posix_memalign(&v, sizeof(void *), bytes);
		return v;
#   else
		return memalign(bytes, sizeof(void *));
#   endif
#elif defined(_WIN32)
		return _aligned_malloc(bytes, MEMORY_ALLOCATION_ALIGNMENT);
#else
#error port me!
#endif
	}

	void lifo_free(queue_node_t *ptr)
	{
#ifdef __GNUC__
		free(ptr);
#elif defined(_WIN32)
		_aligned_free(ptr);
#else
#error port me!
#endif
	}
