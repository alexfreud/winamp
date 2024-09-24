/*
 *  RingBuffer.cpp
 *  simple_mp3_playback
 *
 *  Created by Ben Allison on 11/10/07.
 *  Copyright 2007 Nullsoft, Inc. All rights reserved.
 *
 */

#include "RingBuffer.h"
#include "../replicant/foundation/error.h"

#include "bfc/platform/types.h"
#include "bfc/platform/minmax.h"

#include <stdlib.h>
#include <string.h>
#include <algorithm>

#ifdef MIN
#undef MIN
#endif // MIN

#define MIN(a,b) ((a<b)?(a):(b))


RingBuffer::~RingBuffer()
{
	if ( ringBuffer )
		free( ringBuffer );

	ringBuffer = 0;
}

void RingBuffer::Reset()
{
	if ( ringBuffer )
		free( ringBuffer );

	ringBuffer = 0;
}

bool RingBuffer::reserve( size_t bytes )
{
	Reset();

	ringBufferSize = bytes;

	ringBuffer = (char *)calloc( ringBufferSize, sizeof( char ) );
	if ( !ringBuffer )
		return false;

	clear();

	return true;
}

int RingBuffer::expand( size_t bytes )
{
	if ( bytes > ringBufferSize )
	{
		char *new_buffer = (char *)realloc( ringBuffer, bytes );
		if ( !new_buffer )
			return NErr_OutOfMemory;

		size_t write_offset = ringReadPosition - ringBuffer;
		size_t read_offset  = ringWritePosition - ringBuffer;

		/* update write pointer for the new buffer */
		ringWritePosition = new_buffer + write_offset;

		if ( write_offset > read_offset || !ringBufferUsed ) /* ringBufferUsed will resolve the ambiguity when ringWritePosition == ringReadPosition */
		{
			/* the ring buffer looks like [  RXXXW    ], so we don't need to move anything.
			Just update the read pointer */

			ringReadPosition = new_buffer + write_offset;
		}
		else
		{
			/* [XXW    RXX] needs to become [XXW            RXX] */
			size_t end_bytes       = ringBufferSize - read_offset; // number of bytes that we need to relocate (the RXX portion)
			char *new_read_pointer = &new_buffer[ bytes - end_bytes ];

			memmove( new_read_pointer, ringReadPosition, end_bytes );

			ringReadPosition = new_read_pointer; /* update read pointer */
		}

		ringBufferSize = bytes;
		ringBuffer     = new_buffer;

		return NErr_Success;
	}
	else
		return NErr_NoAction;
}

bool RingBuffer::empty() const
{
	return ( ringBufferUsed == 0 );
}

size_t RingBuffer::read( void *dest, size_t len )
{
	int8_t *out   = (int8_t *)dest; // lets us do pointer math easier
	size_t toCopy = MIN( ringBufferUsed, len );
	size_t copied = 0;

	len -= toCopy;

	// read to the end of the ring buffer
	size_t end   = ringBufferSize - ( ringReadPosition - ringBuffer );
	size_t read1 = MIN( end, toCopy );

	memcpy( out, ringReadPosition, read1 );

	copied           += read1;
	ringReadPosition += read1;

	if ( ringReadPosition == ringBuffer + ringBufferSize )
		ringReadPosition = ringBuffer;

	// update positions
	ringBufferUsed -= read1;
	toCopy         -= read1;
	out             = (int8_t *)out + read1;

	// see if we still have more to read after wrapping around
	if ( toCopy )
	{
		memcpy( out, ringReadPosition, toCopy );

		copied           += toCopy;
		ringReadPosition += toCopy;
		ringBufferUsed   -= toCopy;

		if ( ringReadPosition == ringBuffer + ringBufferSize )
			ringReadPosition = ringBuffer;
	}

	return copied;
}

size_t RingBuffer::at(size_t offset, void *dest, size_t len) const
{
	size_t toCopy = ringBufferUsed;

	// make a local copy of this so we don't blow the original
	char *ringReadPosition = this->ringReadPosition;

	/* --- do a "dummy read" to deal with the offset request --- */
	size_t dummy_end = ringBufferSize-(ringReadPosition-ringBuffer);

	offset = MIN(toCopy, offset);
	size_t read0 = MIN(dummy_end, offset);
	ringReadPosition+=read0;

	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition = ringBuffer;

	// update positions
	toCopy -= read0;
	offset -= read0;

	// do second-half read (wraparound)
	if ( offset )
	{
		ringReadPosition += offset;
		toCopy           -= offset;
	}

  // dummy read done

	/* --- set up destination buffer and copy size --- */
	int8_t *out = (int8_t *)dest; // lets us do pointer math easier

	if ( toCopy > len )
		toCopy = len;

	size_t copied=0;

	/* --- read to the end of the ring buffer --- */
	size_t end   = ringBufferSize - ( ringReadPosition - ringBuffer );
	size_t read1 = MIN( end, toCopy );

	memcpy( out, ringReadPosition, read1 );

	copied           += read1;
	ringReadPosition += read1;

	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition = ringBuffer;

	// update positions
	toCopy -= read1;
	out     = (int8_t *)out + read1;

	/* --- see if we still have more to read after wrapping around --- */
	if (toCopy)
	{
		memcpy(out, ringReadPosition, toCopy);

		copied           += toCopy;
		ringReadPosition += toCopy;
	}

	return copied;
}

size_t RingBuffer::peek( void *dest, size_t len ) const
{
	int8_t *out = (int8_t *)dest; // lets us do pointer math easier

	size_t toCopy = MIN( ringBufferUsed, len );
	size_t copied = 0;

	// make a local copy of this so we don't blow the original
	char *ringReadPosition = this->ringReadPosition;

	// read to the end of the ring buffer
	size_t end   = ringBufferSize - ( ringReadPosition - ringBuffer );
	size_t read1 = MIN( end, toCopy );

	memcpy( out, ringReadPosition, read1 );

	copied           += read1;
	ringReadPosition += read1;

	if ( ringReadPosition == ringBuffer + ringBufferSize )
		ringReadPosition = ringBuffer;

	// update positions
	toCopy -= read1;
	out     = (int8_t *)out + read1;

	// see if we still have more to read after wrapping around
	if ( toCopy )
	{
		memcpy( out, ringReadPosition, toCopy );

		copied += toCopy;
		ringReadPosition += toCopy;
	}

	return copied;
}

size_t RingBuffer::advance( size_t len )
{
	size_t toCopy = MIN( ringBufferUsed, len );
	size_t copied = 0;

	len -= toCopy;

	// read to the end of the ring buffer
	size_t end   = ringBufferSize - ( ringReadPosition - ringBuffer );
	size_t read1 = MIN( end, toCopy );
	
	copied           += read1;
	ringReadPosition += read1;

	if ( ringReadPosition == ringBuffer + ringBufferSize )
		ringReadPosition = ringBuffer;

	// update positions
	toCopy         -= read1;
	ringBufferUsed -= read1;

	// see if we still have more to read after wrapping around
	if ( toCopy )
	{
		copied           += toCopy;
		ringReadPosition += toCopy;
		ringBufferUsed   -= toCopy;

		if ( ringReadPosition == ringBuffer + ringBufferSize )
			ringReadPosition = ringBuffer;
	}

	return copied;
}

size_t RingBuffer::avail() const
{
	return ringBufferSize - ringBufferUsed;
}

size_t RingBuffer::write( const void *buffer, size_t bytes )
{
	size_t used  = ringBufferUsed;
	size_t avail = ringBufferSize - used;

	bytes = MIN( avail, bytes );

	// write to the end of the ring buffer
	size_t end    = ringBufferSize - ( ringWritePosition - ringBuffer );
	size_t copied = 0;
	size_t write1 = MIN( end, bytes );

	memcpy( ringWritePosition, buffer, write1 );

	copied            += write1;
	ringWritePosition += write1;

	if ( ringWritePosition == ringBuffer + ringBufferSize )
		ringWritePosition = ringBuffer;

	// update positions
	ringBufferUsed += write1;
	bytes          -= write1;
	buffer          = (const int8_t *)buffer + write1;

	// see if we still have more to write after wrapping around
	if ( bytes )
	{
		memcpy( ringWritePosition, buffer, bytes );

		copied            += bytes;
		ringWritePosition += bytes;
		ringBufferUsed    += bytes;

		if ( ringWritePosition == ringBuffer + ringBufferSize )
			ringWritePosition = ringBuffer;
	}

	return copied;
}

size_t RingBuffer::drain( Drainer *drainer, size_t max_bytes )
{
	// read to the end of the ring buffer
	size_t used  = ringBufferUsed;
	size_t bytes = used;

	bytes = MIN(bytes, max_bytes);

	size_t copied = 0;
	size_t end    = ringBufferSize-(ringReadPosition-ringBuffer);
	size_t drain1 = MIN(end, bytes);

	if (!drain1)
		return 0;

	size_t read1 = drainer->Write(ringReadPosition, drain1);
	if (read1 == 0)
		return 0;

	copied+=read1;
	ringReadPosition+=read1;
	if (ringReadPosition == ringBuffer + ringBufferSize)
		ringReadPosition=ringBuffer;

		// update positions
	ringBufferUsed -= read1;
	bytes-=read1;

	// see if we still have more to read after wrapping around
	if (drain1 == read1 && bytes)
	{
		size_t read2 =  drainer->Write(ringReadPosition, bytes);

		copied           += read2;
		ringReadPosition += read2;
		ringBufferUsed   -= read2;

		if (ringReadPosition == ringBuffer + ringBufferSize)
			ringReadPosition=ringBuffer;
	}

	return copied;
}

size_t RingBuffer::fill(Filler *filler, size_t max_bytes)
{
	// write to the end of the ring buffer
	size_t used  = ringBufferUsed;
	size_t bytes = ringBufferSize - used;

	bytes = MIN(bytes, max_bytes);

	size_t copied = 0;
	size_t end    = ringBufferSize-(ringWritePosition-ringBuffer);
	size_t fill1  = MIN(end, bytes);

	if (!fill1)
		return 0;

	size_t write1 = filler->Read(ringWritePosition, fill1);
	if (write1 == 0)
		return 0;

	copied+=write1;
	ringWritePosition+=write1;

	if (ringWritePosition == ringBuffer + ringBufferSize)
		ringWritePosition=ringBuffer;

	// update positions
	ringBufferUsed += write1;
	bytes-=write1;

	// see if we still have more to write after wrapping around
	if (fill1 == write1 && bytes)
	{
		size_t write2 =  filler->Read(ringWritePosition, bytes);

		copied            += write2;
		ringWritePosition += write2;
		ringBufferUsed    += write2;

		if (ringWritePosition == ringBuffer + ringBufferSize)
			ringWritePosition=ringBuffer;
	}

	return copied;
}

size_t RingBuffer::size() const
{
	return ringBufferUsed;
}

void RingBuffer::clear()
{
	ringBufferUsed    = 0;
	ringWritePosition = ringBuffer;
	ringReadPosition  = ringBuffer;
}

void *RingBuffer::LockBuffer()
{
	return ringBuffer;
}

void RingBuffer::UnlockBuffer( size_t written )
{
	ringWritePosition = ringBuffer+written;
	ringBufferUsed    = written;
}

size_t RingBuffer::write_position() const
{
	return (size_t)ringWritePosition;
}

size_t RingBuffer::read_position() const
{
	return (size_t)ringReadPosition;
}

void RingBuffer::get_read_buffer(size_t bytes, const void **buffer, size_t *bytes_available) const
{
	size_t toCopy = MIN( ringBufferUsed, bytes );

	// read to the end of the ring buffer
	size_t end = ringBufferSize-(ringReadPosition-ringBuffer);

	*bytes_available = MIN(end, toCopy);
	*buffer          = ringReadPosition;
}
