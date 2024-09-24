/*
 *  LockFreeRingBuffer.cpp
 *  Lock-free ring buffer data structure.
 *  One thread can be consumer and one can be producer
 *
 *  Created by Ben Allison on 11/10/07.
 *  Copyright 2007 Nullsoft, Inc. All rights reserved.
 *
 */

#include "LockFreeRingBuffer.h"
#include "foundation/types.h"
#include "foundation/atomics.h"
#include "foundation/error.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MIN(a,b) ((a<b)?(a):(b))


LockFreeRingBuffer::LockFreeRingBuffer()
{
	ringBuffer=0;
	ringBufferSize=0;
	ringBufferUsed=0;
	ringWritePosition=0;
	ringReadPosition=0;
}

LockFreeRingBuffer::~LockFreeRingBuffer()
{
	free(ringBuffer);
	ringBuffer=0;
}

void LockFreeRingBuffer::Reset()
{
	free(ringBuffer);
	ringBuffer=0;
}

bool LockFreeRingBuffer::reserve(size_t bytes)
{
	void *new_ring_buffer = realloc(ringBuffer, bytes);
	if (!new_ring_buffer)
		return false;

	ringBufferSize=bytes;
	ringBuffer = (char *)new_ring_buffer;
	clear();
	return true;
}

int LockFreeRingBuffer::expand(size_t bytes)
{
	
	if (bytes > ringBufferSize)
	{
		char *new_buffer = (char *)realloc(ringBuffer, bytes);
		if (!new_buffer)
			return NErr_OutOfMemory;

		size_t write_offset = ringReadPosition-ringBuffer;
		size_t read_offset = ringWritePosition-ringBuffer;
		
		/* update write pointer for the new buffer */
		ringWritePosition = new_buffer + write_offset;

		if (write_offset > read_offset || !ringBufferUsed) /* ringBufferUsed will resolve the ambiguity when ringWritePosition == ringReadPosition */
		{
			/* the ring buffer looks like [  RXXXW    ], so we don't need to move anything.
			Just update the read pointer */

			ringReadPosition = new_buffer + write_offset;
		}
		else
		{
			/* [XXW    RXX] needs to become [XXW            RXX] */
			size_t end_bytes = ringBufferSize-write_offset; // number of bytes that we need to relocate (the RXX portion)
			char *new_read_pointer = &new_buffer[bytes - end_bytes]; 
			memmove(new_read_pointer, ringReadPosition, end_bytes);
			ringReadPosition = new_read_pointer; /* update read pointer */
		}
		ringBufferSize=bytes;
		ringBuffer = new_buffer;
#if defined(__ARM_ARCH_7A__)
	__asm__ __volatile__ ("dmb" : : : "memory");
#endif
		return NErr_Success;
	}
	else
		return NErr_NoAction;
}


bool LockFreeRingBuffer::empty() const
{
	return (ringBufferUsed==0);
}

size_t LockFreeRingBuffer::read(void *dest, size_t len)
{
	int8_t *out = (int8_t *)dest; // lets us do pointer math easier
	size_t toCopy=ringBufferUsed;
	if (toCopy > len) toCopy = len;

	size_t copied=0;
	len-=toCopy;

	// read to the end of the ring buffer
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);

	size_t read1 = MIN(end, toCopy);
	memcpy(out, ringReadPosition, read1);
#if defined(__ARM_ARCH_7A__)
	__asm__ __volatile__ ("dmb" : : : "memory");
#endif
	copied+=read1;
	ringReadPosition+=read1;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

	// update positions
	nx_atomic_sub(read1, &ringBufferUsed);
	toCopy-=read1;
	out = (int8_t *)out+read1;

	// see if we still have more to read after wrapping around
	if (toCopy)
	{
		memcpy(out, ringReadPosition, toCopy);
#if defined(__ARM_ARCH_7A__)
		__asm__ __volatile__ ("dmb" : : : "memory");
#endif
		copied+=toCopy;
		ringReadPosition+=toCopy;
		nx_atomic_sub(toCopy, &ringBufferUsed);
		if (ringReadPosition == ringBuffer + ringBufferSize)
			ringReadPosition=ringBuffer;
	}

	return copied;
}

size_t LockFreeRingBuffer::advance_to(size_t position)
{
	intptr_t bytes_to_flush = (intptr_t)(position - (size_t)ringReadPosition);
	if (bytes_to_flush < 0)
		bytes_to_flush += ringBufferSize;
	return advance(bytes_to_flush);
}

size_t LockFreeRingBuffer::at(size_t offset, void *dest, size_t len) const
{
	size_t toCopy=ringBufferUsed;

	// make a local copy of this so we don't blow the original
	char *ringReadPosition = this->ringReadPosition;

	/* --- do a "dummy read" to deal with the offset request --- */
	size_t dummy_end = ringBufferSize-(ringReadPosition-ringBuffer);

	offset = MIN(toCopy, offset);
	size_t read0 = MIN(dummy_end, offset);
	ringReadPosition+=read0;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

	// update positions
	toCopy-=read0;
	offset-=read0;

	// do second-half read (wraparound)
	if (offset)
	{
		ringReadPosition+=offset;
		toCopy-=offset;
	}

  // dummy read done

	/* --- set up destination buffer and copy size --- */
	int8_t *out = (int8_t *)dest; // lets us do pointer math easier

	if (toCopy > len) toCopy=len;
	size_t copied=0;

	/* --- read to the end of the ring buffer --- */
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);

	size_t read1 = MIN(end, toCopy);
	memcpy(out, ringReadPosition, read1);
	copied+=read1;
	ringReadPosition+=read1;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

	// update positions
	toCopy-=read1;
	out = (int8_t *)out+read1;

	/* --- see if we still have more to read after wrapping around --- */
	if (toCopy)
	{
		memcpy(out, ringReadPosition, toCopy);
		copied+=toCopy;
		ringReadPosition+=toCopy;
	}

	return copied;
}

size_t LockFreeRingBuffer::peek(void *dest, size_t len) const
{
	int8_t *out = (int8_t *)dest; // lets us do pointer math easier

	size_t toCopy=ringBufferUsed;

	if (toCopy > len) toCopy=len;
	size_t copied=0;

	// make a local copy of this so we don't blow the original
	char *ringReadPosition = this->ringReadPosition;

	// read to the end of the ring buffer
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);

	size_t read1 = MIN(end, toCopy);
	memcpy(out, ringReadPosition, read1);
	copied+=read1;
	ringReadPosition+=read1;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

	// update positions
	toCopy-=read1;
	out = (int8_t *)out+read1;

	// see if we still have more to read after wrapping around
	if (toCopy)
	{
		memcpy(out, ringReadPosition, toCopy);
		copied+=toCopy;
		ringReadPosition+=toCopy;
	}

	return copied;
}

size_t LockFreeRingBuffer::advance(size_t len)
{
#if defined(__ARM_ARCH_7A__)
		__asm__ __volatile__ ("dmb" : : : "memory");
#endif
	size_t toCopy=ringBufferUsed;

	if (toCopy>len) toCopy=len;
	size_t copied=0;
	len-=toCopy;

	// read to the end of the ring buffer
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);

	size_t read1 = MIN(end, toCopy);
	copied+=read1;
	ringReadPosition+=read1;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

	// update positions
	toCopy-=read1;
	nx_atomic_sub(read1, &ringBufferUsed);

	// see if we still have more to read after wrapping around
	if (toCopy)
	{
		copied+=toCopy;
		ringReadPosition+=toCopy;
		nx_atomic_sub(toCopy, &ringBufferUsed);

		if (ringReadPosition == ringBuffer + ringBufferSize)
			ringReadPosition=ringBuffer;
	}

	return copied;
}

size_t LockFreeRingBuffer::avail() const
{
	return ringBufferSize - ringBufferUsed;
}

size_t LockFreeRingBuffer::write(const void *buffer, size_t bytes)
{
	size_t used=ringBufferUsed;

	size_t avail = ringBufferSize - used;
	bytes = MIN(avail, bytes);

	// write to the end of the ring buffer
	size_t end = ringBufferSize-(ringWritePosition-ringBuffer);
	size_t copied=0;
	size_t write1 = MIN(end, bytes);
	memcpy(ringWritePosition, buffer, write1);
#if defined(__ARM_ARCH_7A__)
	__asm__ __volatile__ ("dmb" : : : "memory");
#endif
	copied+=write1;
	ringWritePosition+=write1;
	if (ringWritePosition == ringBuffer + ringBufferSize)
		ringWritePosition=ringBuffer;


	// update positions
	nx_atomic_add(write1, &ringBufferUsed);
	bytes-=write1;
	buffer = (const int8_t *)buffer+write1;

	// see if we still have more to write after wrapping around
	if (bytes)
	{
		memcpy(ringWritePosition, buffer, bytes);
#if defined(__ARM_ARCH_7A__)
		__asm__ __volatile__ ("dmb" : : : "memory");
#endif
		copied+=bytes;
		ringWritePosition+=bytes;
		nx_atomic_add(bytes, &ringBufferUsed);
		if (ringWritePosition == ringBuffer + ringBufferSize)
			ringWritePosition=ringBuffer;
	}

	return copied;
}

size_t LockFreeRingBuffer::update(size_t bytes)
{
	size_t used=ringBufferUsed;

	size_t avail = ringBufferSize - used;
	bytes = MIN(avail, bytes);

	// write to the end of the ring buffer
	size_t end = ringBufferSize-(ringWritePosition-ringBuffer);
	size_t copied=0;
	size_t write1 = MIN(end, bytes);
#if defined(__ARM_ARCH_7A__)
	__asm__ __volatile__ ("dmb" : : : "memory");
#endif
	copied+=write1;
	ringWritePosition+=write1;
	if (ringWritePosition == ringBuffer + ringBufferSize)
		ringWritePosition=ringBuffer;

	// update positions
	nx_atomic_add(write1, &ringBufferUsed);
	bytes-=write1;

	// see if we still have more to write after wrapping around
	if (bytes)
	{
		/* no need for memory barrier here, we havn't written anything in the interim */
		copied+=bytes;
		ringWritePosition+=bytes;
		nx_atomic_add(bytes, &ringBufferUsed);
		if (ringWritePosition == ringBuffer + ringBufferSize)
			ringWritePosition=ringBuffer;
	}

	return copied;
}


void LockFreeRingBuffer::get_write_buffer(size_t bytes, void **buffer, size_t *bytes_available)
{
	size_t used=ringBufferUsed;

	size_t avail = ringBufferSize - used;
	bytes = MIN(avail, bytes);

	// can only write to the end of the ring buffer
	size_t end = ringBufferSize-(ringWritePosition-ringBuffer);
	*bytes_available = MIN(end, bytes);
	*buffer = ringWritePosition;
}

void LockFreeRingBuffer::get_read_buffer(size_t bytes, const void **buffer, size_t *bytes_available)
{
	size_t toCopy=ringBufferUsed;

	if (toCopy > bytes) toCopy=bytes;

	// read to the end of the ring buffer
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);

	*bytes_available = MIN(end, toCopy);
	*buffer = ringReadPosition;
}

size_t LockFreeRingBuffer::size() const
{
	return ringBufferUsed;
}

void LockFreeRingBuffer::clear()
{
	nx_atomic_write(0, &ringBufferUsed);
	ringWritePosition=ringBuffer;
	ringReadPosition=ringBuffer;
}

size_t LockFreeRingBuffer::write_position() const
{
	return (size_t)ringWritePosition;
}

size_t LockFreeRingBuffer::read_position() const
{
	return (size_t)ringReadPosition;
}
