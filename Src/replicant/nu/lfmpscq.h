#pragma once
#include "queue_node.h"

/* lock free unbounded multiple producer, single consumer queue */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct mpscq_struct_t
{
	queue_node_t * volatile head;
	queue_node_t *tail;
	queue_node_t stub;
} mpscq_t;

#define MPSCQ_STATIC_INIT(self) {&self.stub, &self.stub, {0}}

void mpscq_init(mpscq_t* self);
int mpscq_push(mpscq_t *self, queue_node_t *n);
queue_node_t *mpscq_pop(mpscq_t *self); /* returns (queue_node_t *)1 if the queue is busy */
#ifdef __cplusplus
}
#endif
