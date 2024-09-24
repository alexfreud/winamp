#include "Decoder.h"
#include "lib.h"

void *F263_CreateDecoder()
{
	return new Decoder;
}

int F263_DecodeFrame(void *context, void *frameData, size_t frameSize, YV12_PLANES *yv12, int *width, int *height, int *keyframe)
{
	if (frameSize > 0x1FFFFFFF)
		return F263_ERROR_TOO_MUCH_DATA;

	if (!frameData)
		return F263_ERROR_NO_DATA;

	Decoder *decoder = (Decoder *)context;
	decoder->buffer.data = (uint8_t *)frameData;
	decoder->buffer.numBits = (uint32_t)(frameSize*8);
	return decoder->DecodeFrame(yv12, width, height, keyframe);
}

void F263_DestroyDecoder(void *context)
{
	Decoder *decoder = (Decoder *)context;
	delete decoder;
}
