#include "LockFreeFIFO.h"
#include "foundation/align.h"
#include "foundation/atomics.h"

#if 1 //def USE_VISTA_UP_API
static int CAS2(FIFO_POINTER *fifo, queue_node_t *tail, size_t icount, queue_node_t *next, size_t icount2)
{
	NALIGN(8) FIFO_POINTER compare = { tail, icount };
	NALIGN(8) FIFO_POINTER exchange = { next, icount2 };
	return nx_atomic_cmpxchg2(*(int64_t *)&compare, *(int64_t *)&exchange, (volatile int64_t *)fifo);
	//return atomic_compare_exchange_doublepointer((volatile intptr2_t *)fifo, *(intptr2_t *)&exchange, *(intptr2_t *)&compare);
}
#else
inline char CAS2 (volatile void * addr, volatile void * v1, volatile long v2, void * n1, long n2) 
{
	register char c;
	__asm {
		push	ebx
		push	ecx
		push	edx
		push	esi
		mov		esi, addr
		mov		eax, v1
		mov		ebx, n1
		mov		ecx, n2
		mov		edx, v2
		LOCK    cmpxchg8b qword ptr [esi]
		sete	c
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
	}
	return c;
}
#endif

static int CAS(queue_node_t **fifo, queue_node_t *compare, queue_node_t *exchange)
{
	return nx_atomic_cmpxchg_pointer(compare, exchange, (void* volatile *)fifo);
}

#define ENDFIFO(ff)	((queue_node_t*)0)
void fifo_init(fifo_t *fifo)
{
	fifo->dummy.Next = ENDFIFO(fifo); //makes the cell the only cell in the list
	fifo->head.fifo_node_t = fifo->tail.fifo_node_t = &fifo->dummy;
	fifo->head.count = fifo->tail.count = 0;
	fifo->count = 0;
}

void fifo_push(fifo_t *fifo, queue_node_t *cl)
{
	queue_node_t * volatile tail;
	size_t icount;
	cl->Next = ENDFIFO(fifo); // // set the cell next pointer to end marker
	for (;;) // try until enqueue is done
	{
		icount = fifo->tail.count; // read the _tail modification count
		tail = fifo->tail.fifo_node_t; // read the _tail cell
		if (CAS(&tail->Next, ENDFIFO(fifo), cl)) // try to link the cell to the _tail cell
		{
			break; // enqueue is done, exit the loop
		}
		else // _tail was not pointing to the last cell, try to set _tail to the next cell
		{
			CAS2(&fifo->tail, tail, icount, tail->Next, icount+1);
		}
	}
	CAS2 (&fifo->tail, tail, icount, cl, icount+1); // enqueue is done, try to set _tail to the enqueued cell
	nx_atomic_inc(&fifo->count);
}

queue_node_t *fifo_pop(fifo_t *fifo)
{
	queue_node_t * volatile head;
	for (;;)
	{
		size_t ocount = fifo->head.count;  // read the _head modification count
		size_t icount = fifo->tail.count; // read the _tail modification count
		head = fifo->head.fifo_node_t; // read the _head cell
		queue_node_t *next = head->Next; // read the next cell
		if (ocount == fifo->head.count) // ensures that next is a valid pointer to avoid failure when reading next value
		{
			if (head == fifo->tail.fifo_node_t) // is queue empty or _tail falling behind ?
			{
				if (next == ENDFIFO(fifo)) // is queue empty ?
				{
					return 0; // queue is empty: return NULL
				}
				// _tail is pointing to _head in a non empty queue, try to set _tail to the next cell
				CAS2(&fifo->tail, head, icount, next, icount+1);
			}
			else if (next != ENDFIFO(fifo)) // if we are not competing on the dummy next
			{
				if (CAS2(&fifo->head, head, ocount, next, ocount+1)) // try to set _head to the next cell
				{
					break;
				}
			}
		}
	}
	nx_atomic_dec(&fifo->count);
	if (head == &fifo->dummy) 
	{
		fifo_push(fifo,head);
		head = fifo_pop(fifo);
	}
	return head;
}
