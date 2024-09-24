#pragma once
#include "../in_flv/svc_flvdecoder.h"
#include "../in_flv/FLVAudioHeader.h"
#include "../in_flv/ifc_flvaudiodecoder.h"

#include "incs/mp4AudioDecIfc.h"

// {7FD8F2D0-7490-45fd-B741-BF3F9EEF0BE4}
static const GUID flv_aac_guid = 
{ 0x7fd8f2d0, 0x7490, 0x45fd, { 0xb7, 0x41, 0xbf, 0x3f, 0x9e, 0xef, 0xb, 0xe4 } };


class FLVDecoder : public svc_flvdecoder
{
public:
	static const char *getServiceName() { return "AAC FLV Decoder"; }
	static GUID getServiceGuid() { return flv_aac_guid; } 
	int CreateAudioDecoder(int stereo, int bits, int sample_rate, int format, ifc_flvaudiodecoder **decoder);
	int HandlesAudio(int format_type);
protected:
	RECVS_DISPATCH;
};

class FLVAAC : public ifc_flvaudiodecoder
{
public:
	FLVAAC(CAccessUnitPtr access_unit);
	int GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *bits);
	int DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate);
	void Flush();
	void Close();
	int Ready();
	void SetPreferences(unsigned int max_channels, unsigned int preferred_bits);
private:
/* data */
	mp4AudioDecoderHandle decoder;
	CCompositionUnitPtr composition_unit; /* output */
	CAccessUnitPtr access_unit; /* input */
	unsigned int bps;
	size_t preDelay;
	bool got_decoder_config;

protected:
	RECVS_DISPATCH;
};
