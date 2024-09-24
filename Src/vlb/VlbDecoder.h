#pragma once
#include "obj_vlbDecoder.h"
#include "vlbout.h"
class VLBDecoder : public obj_vlbDecoder
{
public:
	VLBDecoder();
	~VLBDecoder();
	int Open(DataIOControl *paacInput);
	void Close();
  long Synchronize(AACStreamParameters *paacStreamParameters);
  long DecodeFrame(AACStreamParameters *paacStreamParameters);
	void Flush();
	size_t Read(void *buffer, size_t bufferlen);
protected:
	RECVS_DISPATCH;
private:
	CAacDecoderApi *decoder;
	VLBOut dataout;
	AUDIO_FORMATINFO info;
};