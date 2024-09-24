#pragma once
#include "../Plugins/Input/in_flv/svc_flvdecoder.h"
#include "../Plugins/Input/in_flv/FLVAudioHeader.h"
#include "../Plugins/Input/in_flv/ifc_flvaudiodecoder.h"

#include "MFTDecoder.h"

// {B670EC35-1313-460a-A2EA-C7120AD18BAD}
static const GUID flv_aac_guid = 
{ 0xb670ec35, 0x1313, 0x460a, { 0xa2, 0xea, 0xc7, 0x12, 0xa, 0xd1, 0x8b, 0xad } };


class FLVDecoder : public svc_flvdecoder
{
public:
	static const char *getServiceName() { return "MFT AAC FLV Decoder"; }
	static GUID getServiceGuid() { return flv_aac_guid; } 
	int CreateAudioDecoder(int stereo, int bits, int sample_rate, int format, ifc_flvaudiodecoder **decoder);
	int HandlesAudio(int format_type);
protected:
	RECVS_DISPATCH;
};

class FLVAAC : public ifc_flvaudiodecoder
{
public:
	FLVAAC();
	int GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *bits);
	int DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate);
	void Flush();
	void Close();
	int Ready();
	void SetPreferences(unsigned int max_channels, unsigned int preferred_bits);
private:
/* data */
	MFTDecoder decoder;
	unsigned int bps;
	size_t preDelay;
	bool got_decoder_config;

protected:
	RECVS_DISPATCH;
};
