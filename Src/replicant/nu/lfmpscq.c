#include "lfmpscq.h"
#include "foundation/atomics.h"

void mpscq_init(mpscq_t* self)
{
	self->head = &self->stub;
	self->tail = &self->stub;
	self->stub.Next = 0;
}

int mpscq_push(mpscq_t *self, queue_node_t *n)
{
	queue_node_t *prev;
	n->Next = 0;
	prev = nx_atomic_swap_pointer(n, (void * volatile *)&self->head);
	//(*)
	prev->Next = n;
	return prev!=&self->stub;
}

queue_node_t *mpscq_pop(mpscq_t *self)
{
	queue_node_t* tail = self->tail;
	queue_node_t* next = tail->Next;
	queue_node_t* head;
	if (tail == &self->stub)
	{
		if (0 == next)
			return 0;
		self->tail = next;
		tail = next;
		next = next->Next;
	}
	if (next)
	{
		self->tail = next;
		return tail;
	}
	head = self->head;
	if (tail != head)
		return (queue_node_t *)1;
	mpscq_push(self, &self->stub);
	next = tail->Next;
	if (next)
	{
		self->tail = next;
		return tail;
	}
	return 0;
}
