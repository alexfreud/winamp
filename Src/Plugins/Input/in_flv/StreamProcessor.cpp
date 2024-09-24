#include "StreamProcessor.h"
#include "FLVHeader.h"

StreamProcessor::StreamProcessor()
{
	buffer.reserve(1024*1024);
	bytesWritten=0;
	readHeader=false;
}

int StreamProcessor::Write(void *data, size_t datalen, size_t *written)
{
	*written = buffer.write(data, datalen);
	bytesWritten += *written;
	if (*written != datalen)
		return FLVPROCESSOR_WRITE_WAIT; // tell the FLVReader we need a break :)
	return FLVPROCESSOR_WRITE_OK;
}

int StreamProcessor::Process()
{
	// we're actually not going to parse anything until the GetFrame() call.
	// since we can't seek anyway
	// but we do need to validate that this is an FLV stream

	if (!readHeader) 
	{
		if (buffer.size() >= 9) // see if we have enough data
		{
			uint8_t data[9] = {0};
			buffer.read(data, 9);

			if (header.Read(data, 9))
			{
				readHeader=true;
				return FLV_OK;
			}
			else // not an FLV header
			{
				return FLV_ERROR;
			}
		}
	}

	return FLV_NEED_MORE_DATA;
}

uint64_t StreamProcessor::GetProcessedPosition()
{
	// since we parse the bitstream on-demand, we'll just return how many bytes we've buffered so far
	if (readHeader) // make sure we've at least found the main FLV header
		return bytesWritten;
	else
		return 0;
}

uint32_t StreamProcessor::GetMaxTimestamp()
{
	return -1000; // it's a stream! no length!
}

uint64_t StreamProcessor::Seek(uint64_t position)
{
	// we can't really seek in a traditional sense, but since Seek gets called to simply advance the read pointer, 
	// we'll advance within our buffer
	// we always set FrameData::location to 0, so can just call like this
	return buffer.advance((size_t)position);
}

size_t StreamProcessor::Read(void *data, size_t bytes)
{
	// easy :)
	return buffer.read(data, bytes);
}

// the fun happens here
bool StreamProcessor::GetFrame(size_t frameIndex, FrameData &frameData)
{
	// since this is a stream, we're going to ignore frameIndex and just give them the next frame

	if (buffer.size() >= 15)
	{
		uint8_t data[15] = {0};
		buffer.peek(data, 15);
		if (frameData.header.Read(data, 15))
		{
			// first, make sure we have enough data buffered to read the whole thing
			// because the next thing to get called after this function is Read()
			if (frameData.header.dataSize + 15 <= buffer.size()) 
			{
				 // since we're streaming and only processing one frame at a time
				// we're going to set the returned frame's location to 0 (start of the ring buffer)
				frameData.location = 0;
				return true;
			}
		}
	}
	return false; // not ready for a frame yet
}

bool StreamProcessor::GetPosition(int time_in_ms, size_t *frameIndex, bool needVideoKeyFrame)
{
	return false; // can't seek!
}

FLVHeader *StreamProcessor::GetHeader()
{
if (readHeader)
return &header;
else
return 0;
}