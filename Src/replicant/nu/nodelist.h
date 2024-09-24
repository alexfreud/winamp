#pragma once
#include "queue_node.h"
#ifdef __cplusplus
extern "C" {
#endif

	// non-thread-safe list of queue_node_t objects with _head and _tail
	typedef struct nodelist_s
	{
		queue_node_t *head;
		queue_node_t *tail;
	} nodelist_s, *nodelist_t;

	void nodelist_init(nodelist_t nodelist);
	void nodelist_push_back(nodelist_t nodelist, queue_node_t *item);
	void nodelist_push_front(nodelist_t nodelist, queue_node_t *item);
	queue_node_t *nodelist_pop_front(nodelist_t nodelist);
	// pushes an item onto the list, but treat it as a whole list rather than a single item
	void nodelist_push_back_list(nodelist_t nodelist, queue_node_t *item);

	
	#ifdef __cplusplus
}
#endif
