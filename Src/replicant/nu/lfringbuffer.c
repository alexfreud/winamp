#include "lfringbuffer.h"
#include "foundation/error.h"
#include <stdlib.h>

typedef struct LFRingBufferHeader
{
	volatile size_t used; /* number of bytes written, in elements */
	size_t size; /* in elements */
	size_t write_position;
	size_t read_position;
} LFRingBufferHeader;

typedef struct LFRingBuffer
{
	LFRingBufferHeader header;
	float data[1];
}  LFRingBuffer;

/* create a ring buffer with the desired number of elements */
int lfringbuffer_create(lfringbuffer_t *out_ring_buffer, size_t number_of_elements)
{
	LFRingBuffer *ring_buffer = (LFRingBuffer *)malloc(sizeof(LFRingBufferHeader) + sizeof(float) * number_of_elements);
	if (!ring_buffer)
		return NErr_OutOfMemory;

	ring_buffer->header.used = 0;
	ring_buffer->header.size = number_of_elements;
	ring_buffer->header.write_position = 0;
	ring_buffer->header.read_position = 0;
	*out_ring_buffer = (lfringbuffer_t)ring_buffer;
	return NErr_Success;
}

int lfringbuffer_destroy(lfringbuffer_t ring_buffer)
{
	free(ring_buffer);
	return NErr_Success;
}

/* ----- Read functions ----- */
/* get how many elements can currently be read */
size_t lfringbuffer_read_available(lfringbuffer_t rb)
{
	LFRingBuffer *ring_buffer = (LFRingBuffer *)rb;
	return ring_buffer->header.used;
}

/* retrieve a pointer that can be read from.  
you might have to call this twice, because of ring buffer wraparound
call lfringbuffer_read_update() when you are done */
int lfringbuffer_read_get(lfringbuffer_t rb, size_t elements_requested, const float **out_buffer, size_t *elements_available)
{
	LFRingBuffer *ring_buffer = (LFRingBuffer *)rb;
	size_t end, to_copy;
	int ret = NErr_Success;		

	/* can only read how many bytes we have available */		
	to_copy=ring_buffer->header.used;
	if (to_copy > elements_requested)
	{
		to_copy = elements_requested;
		ret = NErr_Underrun; /* signal that there was a buffer underrun when reading */
	}

	/* can only read until the end of the buffer */
	end = ring_buffer->header.size-ring_buffer->header.read_position;
	if (to_copy > end)
	{
		to_copy = end;
		ret = NErr_TryAgain; /* signal that they need to call again to get the next part of the buffer */
	}

	*out_buffer = ring_buffer->data + ring_buffer->header.read_position; 
	*elements_available = to_copy;

	ring_buffer->header.read_position += to_copy;
	if (ring_buffer->header.read_position == ring_buffer->header.size)
		ring_buffer->header.read_position=0;

	return ret;
}

/* call to acknowledge that you have read the bytes */
void lfringbuffer_read_update(lfringbuffer_t rb, size_t elements_read)
{
	LFRingBuffer *ring_buffer = (LFRingBuffer *)rb;
#if defined(__ARM_ARCH_7A__)
	__asm__ __volatile__ ("dmb" : : : "memory");
#endif
	nx_atomic_sub(elements_read, &ring_buffer->header.used);
}
static void lfringbuffer_read_set_position(lfringbuffer_t rb, size_t position)
{
	LFRingBuffer *ring_buffer = (LFRingBuffer *)rb;
	intptr_t bytes_to_flush = (intptr_t)(position - (size_t)ring_buffer->header.read_position);
	if (bytes_to_flush < 0)
		bytes_to_flush += ring_buffer->header.size;
	lfringbuffer_read_update(rb, bytes_to_flush);
}

/* ----- Write functions ----- */
/* get how many elements can currently be written */
size_t lfringbuffer_write_available(lfringbuffer_t rb)
{
	LFRingBuffer *ring_buffer = (LFRingBuffer *)rb;
	return ring_buffer->header.size - ring_buffer->header.used;
}

/* retrieve a pointer that can be written to.  
you might have to call this twice, because of ring buffer wraparound
call lfringbuffer_write_update() when you are done */
int lfringbuffer_write_get(lfringbuffer_t rb, size_t elements_requested, float **out_buffer, size_t *elements_available)
{
	LFRingBuffer *ring_buffer = (LFRingBuffer *)rb;
	size_t end, to_copy;
	int ret = NErr_Success;

	/* can only write how many bytes we have available */		
	to_copy=ring_buffer->header.size - ring_buffer->header.used;
	if (to_copy > elements_requested)
	{
		to_copy = elements_requested;
		ret = NErr_Underrun; /* signal that there was a buffer underrun when reading */
	}

	/* can only read until the end of the buffer */
	end = ring_buffer->header.size-ring_buffer->header.write_position;
	if (to_copy > end)
	{
		to_copy = end;
		ret = NErr_TryAgain; /* signal that they need to call again to get the next part of the buffer */
	}

	*out_buffer = ring_buffer->data + ring_buffer->header.write_position; 
	*elements_available = to_copy;

	ring_buffer->header.write_position += to_copy;
	if (ring_buffer->header.write_position == ring_buffer->header.size)
		ring_buffer->header.write_position=0;

	return ret;
}

/* call to acknowledge that you have written the bytes */
void lfringbuffer_write_update(lfringbuffer_t rb, size_t elements_read)
{
	LFRingBuffer *ring_buffer = (LFRingBuffer *)rb;
#if defined(__ARM_ARCH_7A__)
	__asm__ __volatile__ ("dmb" : : : "memory");
#endif
	nx_atomic_add(elements_read, &ring_buffer->header.used);
}

size_t lfringbuffer_write_get_position(lfringbuffer_t rb)
{
	LFRingBuffer *ring_buffer = (LFRingBuffer *)rb;
	return ring_buffer->header.write_position;
}
