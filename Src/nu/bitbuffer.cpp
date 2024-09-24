#include "bitbuffer.h"
#include <memory.h>
#include <stdlib.h>

BitBuffer::BitBuffer()
{
  buffer=0;
  length=0;
  bits=0;
}

void BitBuffer::WriteBit(char bit)
{
  if (bits == 0)
    Resize(length+1);
  
  bit = !!bit;
  unsigned char mask = 1 << (7-bits);
  buffer[length-1] &= ~mask;
  buffer[length-1] |= (bit << (7-bits));
  bits=(bits+1)%8;
}

void BitBuffer::Resize(size_t newlen)
{
  	if (newlen > length)
  	{
		unsigned char *new_buffer = (unsigned char *)realloc(buffer, newlen);
		if (new_buffer)
		{
			buffer = new_buffer;
    		memset(buffer+length, 0, newlen-length); // zero out new data
    		length=newlen;
		}
		else
		{
    		new_buffer = (unsigned char *)malloc(newlen);
			if (new_buffer)
			{
				memcpy(new_buffer, buffer, length);
				free(buffer);
				buffer = new_buffer;
	    		memset(buffer+length, 0, newlen-length); // zero out new data
	    		length=newlen;
			}
		}
  	}
}

void BitBuffer::WriteBits(uintptr_t num, size_t bitlen)
{
  for (size_t i=0;i!=bitlen;i++)
  {
    WriteBit((num >> (bitlen-i-1))&1);
  }
}

void BitBuffer::WriteBytes(void *buffer, size_t bytes)
{
  unsigned char *b = (unsigned char *)buffer;
  for (size_t i=0;i!=bytes;i++)
    WriteBits(b[i], 8);
}

void BitBuffer::WriteByte(unsigned char byte)
{
	for (size_t i=0;i!=8;i++)
	{
		WriteBit((byte >> (7-i))&1);
	}
}