#pragma once
#include "../nsavi/avi_header.h"
#include "../Plugins/Input/in_avi/ifc_aviaudiodecoder.h"

class AVIULawDecoder : public ifc_aviaudiodecoder
{
public:
	AVIULawDecoder(const nsavi::audio_format *waveformat);
	
protected:
	RECVS_DISPATCH;
private:
	/* ifc_aviaudiodecoder implementation */
	int OutputFrameSize(size_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat);
	int DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Close();

private:
	const nsavi::audio_format *waveformat;
};