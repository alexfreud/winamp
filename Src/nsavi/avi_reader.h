#pragma once
#include <bfc/platform/types.h>
#include <stdio.h>
#include "read.h" // for the error codes


namespace nsavi
{
		// return codes from avi_reader functions

	enum
	{
		READ_OK = 0,
		READ_EOF = 1,
		READ_FAILED = 2,
		READ_INVALID_DATA = 3, // read was successful but data didn't make any sense
		READ_INVALID_CALL = 4, // wrong time to call this function
		READ_NOT_FOUND = 5, // requested item doesn't exist in the file
		READ_OUT_OF_MEMORY = 6, // some malloc failed and so we're aborting
		READ_DISCONNECT = 7,
	};

class avi_reader
{
public:
	virtual int Read(void *buffer, uint32_t read_length, uint32_t *bytes_read)=0;
	
	// TODO: need to put an upper bound on Peek buffer sizes
	virtual int Peek(void *buffer, uint32_t read_length, uint32_t *bytes_read)=0;
	
	// in_avi will call this before descending into certain chunks that will be read entirely (e.g. avih)
	// you aren't required to do anything in response
	virtual void OverlappedHint(uint32_t read_length){}

	virtual int Seek(uint64_t position)=0;

	virtual uint64_t Tell()=0;

	// skip ahead a certain number of bytes. equivalent to fseek(..., SEEK_CUR)
	virtual int Skip(uint32_t skip_bytes)=0;
	virtual uint64_t GetContentLength()=0;
	virtual void GetFilename(wchar_t *fn, size_t len)=0;
};

}