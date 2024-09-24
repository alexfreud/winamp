#pragma once
#include "../Plugins/Input/in_avi/svc_avidecoder.h"
#include "../Plugins/Input/in_avi/ifc_aviaudiodecoder.h"
#include "MFTDecoder.h"


// {D6DB50A7-5E2F-4374-97B0-67CA707EB3CD}
static const GUID avi_aac_guid = 
{ 0xd6db50a7, 0x5e2f, 0x4374, { 0x97, 0xb0, 0x67, 0xca, 0x70, 0x7e, 0xb3, 0xcd } };

class AVIDecoder : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "MFT AAC AVI Decoder"; }
	static GUID getServiceGuid() { return avi_aac_guid; }
	int CreateAudioDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point, ifc_aviaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class AVIAACDecoder : public ifc_aviaudiodecoder
{
public:
	AVIAACDecoder(unsigned int bps, bool floating_point);
	static AVIAACDecoder *Create(const nsavi::audio_format *stream_format, unsigned int preferred_bits, unsigned int max_channels, bool floating_point);
protected:
	RECVS_DISPATCH;
private:
	/* ifc_aviaudiodecoder implementation */
	int OutputFrameSize(size_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat);
	int DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Flush();
	void Close();

	/* data */
	MFTDecoder decoder;
	unsigned int bps;
	bool floating_point;
};