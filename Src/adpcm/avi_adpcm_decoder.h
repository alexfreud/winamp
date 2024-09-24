#pragma once
#include "../Plugins/Input/in_avi/svc_avidecoder.h"
#include "../Plugins/Input/in_avi/ifc_aviaudiodecoder.h"

// {E0BCBBDF-F1DC-4459-8C0A-4F3FFECFC80E}
static const GUID avi_adpcm_guid = 
{ 0xe0bcbbdf, 0xf1dc, 0x4459, { 0x8c, 0xa, 0x4f, 0x3f, 0xfe, 0xcf, 0xc8, 0xe } };

struct ms_adpcm_format;

class AVIDecoder : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "ADPCM AVI Decoder"; }
	static GUID getServiceGuid() { return avi_adpcm_guid; }
	int CreateAudioDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point, ifc_aviaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class MS_ADPCM_AVIDecoder : public ifc_aviaudiodecoder
{
public:
	MS_ADPCM_AVIDecoder(const ms_adpcm_format *adpcmformat, const nsavi::STRH *stream_header);
	
protected:
	RECVS_DISPATCH;
private:
	/* ifc_aviaudiodecoder implementation */
	int OutputFrameSize(size_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat);
	int DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Close();

private:
	const ms_adpcm_format *adpcmformat;
	const nsavi::STRH *stream_header;
};