#ifndef NULLSOFT_ALLOCLAYERH
#define NULLSOFT_ALLOCLAYERH

#include "WMHandler.h"
#include "BufferPool.h"
#include <cassert>
class AllocLayer : public WMHandler
{
public:
	AllocLayer(IWMReader *reader)
			: readerAdvanced(0),
			listenOutput( -1),
			maxSize(0)

	{
		reader->QueryInterface(&readerAdvanced);

	}
	~AllocLayer()
	{
		if (readerAdvanced)
		{
			readerAdvanced->Release();
			readerAdvanced = 0;
		}
	}

	void Listen(long output)
	{
		listenOutput = output;
		if (output != -1)
		{
			readerAdvanced->SetAllocateForOutput(listenOutput, TRUE);
			readerAdvanced->GetMaxOutputSampleSize(listenOutput, &maxSize);
			assert(maxSize>0);
			pool.SetAllocSize(maxSize);
		}
	}

		void Listen(long output, long numBuffers)
	{
		listenOutput = output;
		if (output != -1)
		{
			readerAdvanced->SetAllocateForOutput(listenOutput, TRUE);
			readerAdvanced->GetMaxOutputSampleSize(listenOutput, &maxSize);
			assert(maxSize>0);
			pool.SetAllocSize(maxSize);
			pool.PreAllocate(numBuffers);
			pool.limit=numBuffers;

		}
	}

	void FreeBuffers()
	{
		pool.FreeBuffers();
	}
	

	BufferPool pool;
private:

	void AllocateOutput(long outputNum, long bufferSize, INSSBuffer *&buffer)
	{
		if (outputNum == listenOutput)
		{
			assert(maxSize >= bufferSize);
			buffer = pool.GetBuffer(bufferSize);
		}
		else
			WMHandler::AllocateOutput(outputNum, bufferSize, buffer); // let other handlers have a shot at it first.
	}

	// WMHandler
	long listenOutput;
	DWORD maxSize;
	IWMReaderAdvanced *readerAdvanced;
};
#endif
