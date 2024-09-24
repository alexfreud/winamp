#pragma once
#include "foundation/error.h"
#include <stdlib.h>

/* an automatically growing buffer */

class AutoBuffer
{
public:
	AutoBuffer()
	{
		buffer=0;
		buffer_length=0;
	}

	~AutoBuffer()
	{
		free(buffer);
	}

	operator void *()
	{
		return buffer;
	}

	template <class ptr_t>
	ptr_t Get()
	{
		return (ptr_t)buffer;
	}

	int Reserve(size_t new_size)
	{
		if (new_size <= buffer_length)
			return NErr_Success;

		void *new_buffer = realloc(buffer, new_size);
		if (!new_buffer)
			return NErr_OutOfMemory;

		buffer = new_buffer;
		buffer_length = new_size;
		return NErr_Success;
	}

private:
	void *buffer;
	size_t buffer_length;
};
