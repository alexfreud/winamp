#ifndef NULLSOFT_REMAININGH
#define NULLSOFT_REMAININGH
#include <assert.h>
#include <memory.h>
/* this class is used to store leftover samples */

class Remaining
{
public:
	Remaining()
		: store(0), size(0), used(0)
	{}

	void Allocate(unsigned long _size)
	{
		assert(_size);
		used=0;
		size=_size;
		if (store)
			delete [] store;
		store = new unsigned char [size];
	}

	/* Saves the incoming data and updates the pointer positions */
	template <class storage_t>
	void UpdatingWrite(storage_t *&data,  unsigned long &bytes)
	{
		unsigned long bytesToWrite = min(bytes, SizeRemaining());
		Write(data, bytesToWrite);
		assert(bytesToWrite);
		data = (storage_t *)((char *)data + bytesToWrite);
		bytes -= bytesToWrite;
	}

	void Write(void *data,  unsigned long bytes)
	{
		unsigned char *copy = (unsigned char *)store;
		copy+=used;
		memcpy(copy, data, bytes);
		used+=bytes;
	}

	unsigned long SizeRemaining()
	{
		return size-used;
	}

	bool Empty()
	{
		return !used;
	}
	bool Full()
	{
		return size == used;
	}
	void *GetData()
	{
		return (void *)store;
	}

	void Flush()
	{
		used=0;
	}
	unsigned char *store;
	 long size, used;
};
#endif