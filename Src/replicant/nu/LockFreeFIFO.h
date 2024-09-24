#pragma once
#include "queue_node.h"
/* Algorithm taken from
Lock-Free Techniques for Concurrent Access to Shared Objects
Dominique Fober, Yann Orlarey, Stephane Letz
http://www.grame.fr/Ressources/pub/LockFree.pdf
http://nedko.arnaudov.name/soft/L17_Fober.pdf
http://www.grame.fr/Ressources/pub/TR-050523.pdf
This implementation (c) 2010 Nullsoft, Inc.
*/
#ifdef __cplusplus
extern "C" {
#endif

	
struct FIFO_POINTER
{
	queue_node_t *fifo_node_t;
	size_t count;
};

struct fifo_t
{
	FIFO_POINTER head; // _head pointer and total count of pop operations (ocount)
	FIFO_POINTER tail; // _tail pointer and total count of push operations (icount)
	queue_node_t dummy;
	size_t count;
};
void fifo_init(fifo_t *fifo);
void fifo_push(fifo_t *fifo, queue_node_t *cl);
queue_node_t *fifo_pop(fifo_t *fifo);
#ifdef __cplusplus
}
#endif