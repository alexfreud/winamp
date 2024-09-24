#pragma once
#include "../Plugins/Input/in_avi/svc_avidecoder.h"
#include "../Plugins/Input/in_avi/ifc_aviaudiodecoder.h"

// {739A9D4E-2235-4298-AA4F-2E9F1B5DE8EC}
static const GUID avi_pcm_guid = 
{ 0x739a9d4e, 0x2235, 0x4298, { 0xaa, 0x4f, 0x2e, 0x9f, 0x1b, 0x5d, 0xe8, 0xec } };


class AVIDecoder : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "PCM AVI Decoder"; }
	static GUID getServiceGuid() { return avi_pcm_guid; }
	int CreateAudioDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point, ifc_aviaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class AVIPCMDecoder : public ifc_aviaudiodecoder
{
public:
	AVIPCMDecoder(const nsavi::audio_format *waveformat);
	
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