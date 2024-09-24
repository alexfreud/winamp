#ifndef NULLSOFT_BUFFERPOOLH
#define NULLSOFT_BUFFERPOOLH

#include <wmsdk.h>
#include "../nu/AutoLock.h"
#include <deque>
#include <iostream>
#include <cassert>
using namespace Nullsoft::Utility;
class Buffer;
class Pool
{
public:
	virtual void ReturnBuffer(Buffer *buffer) = 0;
};

class Buffer : public INSSBuffer
{
public:
	Buffer(size_t _size, Pool *_pool)
			: size(_size),
			length(0),
			pool(_pool),
			refCount(1)
	{
		buffer = new unsigned char[size];
	}

	~Buffer()
	{
		delete[] buffer;
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return ++refCount;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		assert(refCount > 0);

		if (--refCount == 0)
		{
			length = 0;
			pool->ReturnBuffer(this);
		}
		return refCount;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject)
	{
		if (IID_INSSBuffer == iid)
		{
			*ppvObject = static_cast<INSSBuffer *>(this);
			AddRef();
			return S_OK;
		}
		else
		{
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
	}

	HRESULT STDMETHODCALLTYPE GetBuffer(BYTE **ppdwBuffer)
	{
		*ppdwBuffer = (BYTE *)buffer;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetBufferAndLength(BYTE **ppdwBuffer, DWORD *pdwLength)
	{
		*ppdwBuffer = (BYTE *)buffer;
		*pdwLength = length;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetLength(DWORD *pdwLength)
	{
		*pdwLength = length;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetMaxLength(DWORD *pdwLength)
	{
		*pdwLength = size;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE SetLength(DWORD dwLength)
	{
		length = dwLength;
		return S_OK;
	}

	bool CanFit(size_t sizeCompare)
	{
		return (sizeCompare <= size);
	}
	size_t size, length;
	Pool *pool;
	int refCount;
	unsigned char *buffer;
};

class BufferPool : public Pool
{
	typedef std::deque<Buffer *> PoolList;
public:
	long limit;
	BufferPool() : allocSize(0),
			poolSize(0),
			limit(0)
	{}

	~BufferPool()
	{
		FreeBuffers();
	}
	void FreeBuffers()
	{

		AutoLock lock (bufferGuard);
		while (!pool.empty())
		{
			Buffer *buff = pool.front();
			delete buff;
			pool.pop_front();
			poolSize = 0;
		}
	}
	void ReturnBuffer(Buffer *buffer)
	{
		AutoLock lock (bufferGuard);
		pool.push_back(buffer);
	}
	Buffer *SearchForBuffer(size_t size)
	{
		PoolList::iterator itr;
		AutoLock lock (bufferGuard);
		for (itr = pool.begin();itr != pool.end();itr++)
		{
			if ((*itr)->CanFit(size))
			{
				Buffer *buff = *itr;
				pool.erase(itr);
				buff->AddRef();
				return buff;
			}
		}
		return 0;
	}

	void PreAllocate(size_t count)
	{
		if (!allocSize)
			return ;
		for (size_t i = 0;i != count;i++)
		{
			AutoLock lock (bufferGuard);
			pool.push_back( new Buffer(allocSize , this));
		}
	}
	INSSBuffer *GetBuffer(size_t size)
	{
		Buffer *buff = SearchForBuffer(size);

		/*
		while (!buff && poolSize>=limit && limit)
		{
			Sleep(1);
			buff = SearchForBuffer(size);
		}*/

		if (!buff)
		{
			poolSize++;
			std::cerr << "poolsize = " << poolSize << std::endl;
			buff = new Buffer(allocSize ? allocSize : size, this);
		}
		return buff;
	}

	void test()
	{
		std::cerr << "pool.size() == " << pool.size();
		std::cerr << " and poolsize == " << poolSize << std::endl;
	}

	void SetAllocSize(long size)
	{
		allocSize = size;
	}

	PoolList pool;
	LockGuard bufferGuard;
	size_t allocSize;
	size_t poolSize;

};

#endif
