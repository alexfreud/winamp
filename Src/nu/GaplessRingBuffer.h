#pragma once
#include "RingBuffer.h"

class GaplessRingBuffer
{
public:
	GaplessRingBuffer();
	~GaplessRingBuffer();
	int Initialize(size_t samples, size_t bps, size_t channels, size_t pregap, size_t postgap);
	size_t Read(void *destination, size_t destination_bytes);
	bool Empty() const;
	size_t Write(const void *input, size_t input_bytes);
	void Reset();
private:
	RingBuffer ring_buffer;
	size_t frameBytes; // byte size of one frame (channels*bps/8)
	size_t currentPregapBytes;
	size_t pregapBytes;
	size_t postgapBytes;
};