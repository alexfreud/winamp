#include "pfc.h"

void grow_buf::makespace(int new_size)
{
	if (!ptr || !size)
	{
		size = 1;
		while(size<new_size) 
		{
			if (size == (1 << 31))
			{
				return;
			}
			size<<=1;
		}
		ptr = malloc(size);
	}
	else
	{
		if (size<new_size)
		{
			do
			{
				if (size == (1 << 31))
				{
					free(ptr);
					ptr=0;
					return;
				}
				size<<=1;
			}
			while (size<new_size);
			ptr = realloc(ptr,size);
		}
	}
}

void * grow_buf::finish()
{
	void * rv=0;
	if (ptr)
	{
		rv = realloc(ptr,used);
		ptr = 0;
		size = 0;
		used = 0;
	}
	return rv;
}

void grow_buf::reset()
{
	if (ptr) {free(ptr);ptr=0;}
	used=0;
	size=0;
}

static void foo_memcpy(void * dst,const void * src,size_t bytes)
{
	if (src) memcpy(dst,src,bytes);
	else memset(dst,0,bytes);
}

bool grow_buf::write(const void * data, size_t bytes)
{
	makespace(used+bytes);
	if (!ptr)
		return false;
	foo_memcpy((char*)ptr+used,data,bytes);
	used+=bytes;
	return true;
}

void grow_buf::write_ptr(const void * data, int bytes,int offset)
{
	if (offset+bytes>used) {used = offset;write(data,bytes);}
	else foo_memcpy((char*)ptr+offset,data,bytes);
}