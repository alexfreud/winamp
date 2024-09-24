#pragma once
#include <bfc/platform/types.h>
#include "FLVStreamHeader.h"
#include "FLVHeader.h"

struct FrameData
{
	FLVStreamHeader header;
	uint64_t location;
	bool keyFrame;
};

enum
{
	FLV_OK=0,
	FLV_NEED_MORE_DATA=1,
	FLV_ERROR=-1,

	FLVPROCESSOR_WRITE_OK=0,
	FLVPROCESSOR_WRITE_ERROR=1,
	FLVPROCESSOR_WRITE_WAIT=2,
};

class FLVProcessor 
{
public:
	virtual ~FLVProcessor() {}
	virtual int Write(void *data, size_t datalen, size_t *written) = 0;
	virtual int Process()=0;
	virtual uint64_t Seek(uint64_t position)=0;
	virtual size_t Read(void *data, size_t bytes)=0;
	virtual uint64_t GetProcessedPosition()=0;
	virtual bool GetFrame(size_t frameIndex, FrameData &frameData)=0;
	virtual uint32_t GetMaxTimestamp()=0;
	virtual bool GetPosition(int time_in_ms, size_t *frameIndex, bool needsVideoKeyFrame)=0;
	virtual bool IsStreaming()=0;
	virtual FLVHeader *GetHeader() = 0;
};


