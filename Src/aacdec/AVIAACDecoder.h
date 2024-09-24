#pragma once
#include "../in_avi/svc_avidecoder.h"
#include "../in_avi/ifc_aviaudiodecoder.h"
#include "incs/mp4AudioDecIfc.h"

// {4033D2BF-F217-4e8e-877B-2269CD04CA52}
static const GUID avi_aac_guid = 
{ 0x4033d2bf, 0xf217, 0x4e8e, { 0x87, 0x7b, 0x22, 0x69, 0xcd, 0x4, 0xca, 0x52 } };

class AVIDecoder : public svc_avidecoder
{
public:
	static const char *getServiceName() { return "AAC AVI Decoder"; }
	static GUID getServiceGuid() { return avi_aac_guid; }
	int CreateAudioDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point, ifc_aviaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class AVIAACDecoder : public ifc_aviaudiodecoder
{
public:
	AVIAACDecoder(mp4AudioDecoderHandle decoder, CAccessUnitPtr access_unit, CCompositionUnitPtr composition_unit, unsigned int bps, bool floating_point);
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
	mp4AudioDecoderHandle decoder;
	CCompositionUnitPtr composition_unit; /* output */
	CAccessUnitPtr access_unit; /* input */
	unsigned int bps;
	bool floating_point;
};