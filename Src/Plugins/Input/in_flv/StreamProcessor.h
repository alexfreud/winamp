#pragma once

#include "FLVProcessor.h"
#include "../nu/RingBuffer.h"

class StreamProcessor : public FLVProcessor
{
public:
	StreamProcessor();

private:
	int Write(void *data, size_t datalen, size_t *written);
	int Process();
	uint64_t Seek(uint64_t position);
	size_t Read(void *data, size_t bytes);
	uint64_t GetProcessedPosition();
	bool GetFrame(size_t frameIndex, FrameData &frameData);
	uint32_t GetMaxTimestamp();
	bool GetPosition(int time_in_ms, size_t *frameIndex, bool needVideoKeyFrame);
	bool IsStreaming() { return true; }
	FLVHeader *GetHeader();
private:
	RingBuffer buffer;
	uint64_t bytesWritten;
	bool readHeader;
	FLVHeader header;
};

