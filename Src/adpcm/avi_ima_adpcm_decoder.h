#pragma once
#include "../Plugins/Input/in_avi/ifc_aviaudiodecoder.h"
#include "../nsavi/avi_header.h"

struct ima_adpcm_format;

class IMA_ADPCM_AVIDecoder : public ifc_aviaudiodecoder
{
public:
	IMA_ADPCM_AVIDecoder(const ima_adpcm_format *adpcmformat, const nsavi::STRH *stream_header);
	
protected:
	RECVS_DISPATCH;
private:
	/* ifc_aviaudiodecoder implementation */
	int OutputFrameSize(size_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat);
	int DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Close();

private:
	const ima_adpcm_format *adpcmformat;
	const nsavi::STRH *stream_header;
};