#include "GaplessRingBuffer.h"
#include <bfc/platform/types.h>
#include <bfc/error.h>
#include <algorithm>

GaplessRingBuffer::GaplessRingBuffer()
{
	pregapBytes = 0;
	frameBytes = 0;
	postgapBytes = 0;
	currentPregapBytes = 0;
}

GaplessRingBuffer::~GaplessRingBuffer()
{
}

int GaplessRingBuffer::Initialize(size_t samples, size_t bps, size_t channels, size_t pregap, size_t postgap)
{
	this->frameBytes = channels * bps / 8;
	this->currentPregapBytes = this->pregapBytes = pregap * frameBytes;
	this->postgapBytes = postgap * frameBytes;

	ring_buffer.reserve(samples * frameBytes + pregapBytes);
	return NErr_Success;
}

bool GaplessRingBuffer::Empty() const
{
	return (ring_buffer.size() <= pregapBytes);
}

size_t GaplessRingBuffer::Read(void *destination, size_t destination_bytes)
{
	// make sure we've filled enough of the buffer to satisfy the postgap
	if (Empty()) {
		return 0;
	}

	// don't read into postgap area
	size_t remaining = ring_buffer.size() - postgapBytes;
	destination_bytes = min(remaining, destination_bytes);

	return ring_buffer.read(destination, destination_bytes);
}

size_t GaplessRingBuffer::Write(const void *input, size_t input_bytes)
{
	// cut pregap if necessary
	if (currentPregapBytes) {
		size_t cut = min(input_bytes, currentPregapBytes);
		currentPregapBytes -= cut;
		input_bytes -= cut;
		input = (const uint8_t *)input + cut;
	}

	return ring_buffer.write(input, input_bytes);
}

void GaplessRingBuffer::Reset()
{
	currentPregapBytes = pregapBytes;
	ring_buffer.clear();
}