#ifndef NULLSOFT_IN_FLAC_QUICKBUF_H
#define NULLSOFT_IN_FLAC_QUICKBUF_H

#include <malloc.h>
class QuickBuf
{
public:
	QuickBuf() : buffer(0), len(0)
	{
	}

	void Reserve(size_t res)
	{
		if (res > len)
		{
			len=res;
			free(buffer);
			buffer = malloc(len);
		}
	}
	
	void Free()
	{
		free(buffer);
		buffer=0;
		len=0;
	}

	void Move(size_t offset)
	{
		memmove(buffer, (char *)buffer + offset, len-offset);
	}


	operator void *()
	{
		return buffer;
	}

	operator char *()
	{
		return (char *)buffer;
	}

private:
	void *buffer;
	size_t len;
};

#endif