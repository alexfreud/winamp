#pragma once


#include "nu/queue_node.h"
#include <windows.h>
#include <malloc.h>

/* lock free stack object 
multiple threads can push and pop without locking
note that order is not guaranteed.  that is, if Thread 1 calls Insert before Thread 2, Thread 2's item might still make it in before.
*/

#ifdef __cplusplus
#define NU_LIFO_INLINE inline
extern "C" {
#else
#define NX_ATOMIC_INLINE
#endif

	typedef SLIST_HEADER lifo_t;
		
	/* use this to allocate an object that will go into this */
	NU_LIFO_INLINE static queue_node_t *lifo_malloc(size_t bytes) { return (queue_node_t *)_aligned_malloc(bytes, MEMORY_ALLOCATION_ALIGNMENT); }
	NU_LIFO_INLINE static void lifo_free(queue_node_t *ptr) { _aligned_free(ptr); }

	NU_LIFO_INLINE static lifo_t *lifo_create() { return (lifo_t *)_aligned_malloc(sizeof(SLIST_HEADER), MEMORY_ALLOCATION_ALIGNMENT); }
	NU_LIFO_INLINE static void lifo_destroy(lifo_t *lifo) { _aligned_free(lifo); }
	NU_LIFO_INLINE static void lifo_init(lifo_t *lifo) { InitializeSListHead(lifo); }
	NU_LIFO_INLINE static void lifo_push(lifo_t *lifo, queue_node_t *cl) { InterlockedPushEntrySList(lifo, cl); }
	NU_LIFO_INLINE static queue_node_t *lifo_pop(lifo_t *lifo) { return InterlockedPopEntrySList(lifo); }

#ifdef __cplusplus
}
#endif
