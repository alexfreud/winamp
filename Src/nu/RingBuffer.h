/*
 *  RingBuffer.h
 *  simple_mp3_playback
 *
 *  Created by Ben Allison on 11/10/07.
 *  Copyright 2007 Nullsoft, Inc. All rights reserved.
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

class Filler
{
public:
	virtual size_t Read(void *dest, size_t len)=0;
};

class Drainer
{
public:
	virtual size_t Write(const void *dest, size_t len)=0;
};

class RingBuffer
{
public:
	RingBuffer()                                                      {}
	~RingBuffer();

	void Reset();
	bool reserve( size_t bytes ); // destructive.
	int expand( size_t bytes ); // like reserve, but only expands upward. non-destructive. returns an NError
	bool empty() const;
	size_t avail() const; // how much available for writing
	size_t size() const; // how much available for reading
	void clear();
	size_t read( void *dest, size_t len ); // returns amount actually read
	size_t advance( size_t len ); // same as read() but doesn't write the data any where.
	size_t peek( void *dest, size_t len ) const; // same as read() but doesn't advance the read pointer
	size_t write( const void *src, size_t len );
	size_t fill( Filler *filler, size_t max_bytes );
	size_t drain( Drainer *drainer, size_t max_bytes );
	size_t at( size_t offset, void *dest, size_t len ) const; // peeks() from offset. returns bytes read

	size_t write_position() const; // returns an integer representing a write position
	size_t read_position() const; // returns an integer representing the read position

	void get_read_buffer( size_t bytes, const void **buffer, size_t *bytes_available ) const; /* returns a pointer that you can read data from, call advance() when you are done */
	/* DO NOT USING THIS UNLESS YOU KNOW WHAT YOU'RE DOING
	  you should only use it when the ring buffer is empty
		1) call clear() beforehand - very important!
		2) call LockBuffer(), it'll give you a buffer
		3) call UnlockBufer() with how much you've written
		4) you catch the man
	*/
	void *LockBuffer();
	void UnlockBuffer( size_t written );

private:
	volatile size_t ringBufferUsed = 0;

	size_t ringBufferSize   = 0;
	char *ringBuffer        = 0;
	char *ringWritePosition = 0;
	char *ringReadPosition  = 0;
};
