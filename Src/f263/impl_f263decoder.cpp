#include "impl_f263decoder.h"
#include "lib.h"

F263Decoder::F263Decoder() : context(0)
{
	context = F263_CreateDecoder();
}

F263Decoder::~F263Decoder()
{
	if (context)
		F263_DestroyDecoder(context);
}

int F263Decoder::DecodeFrame(void *frameData, size_t frameSize, YV12_PLANES *yv12, int *width, int *height, int *keyframe)
{
	return F263_DecodeFrame(context, frameData, frameSize, yv12, width, height, keyframe);
}

#define CBCLASS F263Decoder
START_DISPATCH;
CB(DISP_DECODEFRAME, DecodeFrame)
END_DISPATCH;
#undef CBCLASS