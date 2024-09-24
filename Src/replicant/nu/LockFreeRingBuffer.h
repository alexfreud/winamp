/*
 *  Created by Ben Allison on 11/10/07.
 *  Copyright 2007-2011 Nullsoft, Inc. All rights reserved.
 *
 *  Ring Buffer class
 *  Thread safety:
 *   This class can be used from exactly two simultaneous threads without locking
 *   as long as one thread only writes and the other thread only reads
 *   the writer thread may call empty(), avail(), size(), write(), fill(
 *   the reader thread my call empty(), avail(), size(), read(), peek(), advance()
 *
 *   two (or more) readers or two (or more) writers requires external locking
 *
 *   Reset(), reserve(), clear(), LockBuffer(), UnlockBuffer() are not thread-safe
 */

#pragma once
#include <stddef.h>
#include "foundation/types.h"

class LockFreeRingBuffer
{
public:
	LockFreeRingBuffer();
	~LockFreeRingBuffer();
	void Reset();
	int expand(size_t bytes); // like reserve, but only expands upward. non-destructive. returns an NError
	bool reserve(size_t bytes);
	bool empty() const;
	size_t avail() const; // how much available for writing
	size_t size() const; // how much available for reading
	void clear();
	size_t read(void *dest, size_t len); // returns amount actually read
	size_t advance(size_t len); // same as read() but doesn't write the data any where.
	size_t peek(void *dest, size_t len) const; // same as read() but doesn't advance the read pointer
	size_t write(const void *src, size_t len);
	size_t update(size_t len); // same as write() but doesn't write the data, usually used after a get_buffer call
	size_t at(size_t offset, void *dest, size_t len) const; // peeks() from offset. returns bytes read
	size_t write_position() const; // returns an integer representing a write position
	size_t read_position() const; // returns an integer representing the read position
	size_t advance_to(size_t position); // moves the read pointer to a position previously returned from write_position().  returns bytes advanced
	void get_write_buffer(size_t bytes, void **buffer, size_t *bytes_available); /* returns a pointer that you can write data to, you MUST call update() when you are done */
	void get_read_buffer(size_t bytes, const void **buffer, size_t *bytes_available); /* returns a pointer that you can read data from, call advance() when you are done */
	

private:
	volatile size_t ringBufferUsed;
	size_t ringBufferSize;
	char *ringBuffer;
	char *ringWritePosition;
	char *ringReadPosition;
};