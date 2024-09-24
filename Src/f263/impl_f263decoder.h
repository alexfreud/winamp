#pragma once
#include "obj_f263decoder.h"

class F263Decoder : public obj_f263decoder
{
public:
	F263Decoder();
	~F263Decoder();
	int DecodeFrame(void *frameData, size_t frameSize, YV12_PLANES *yv12, int *width, int *height, int *keyframe);
protected:
	RECVS_DISPATCH;

private:
	void *context;
};