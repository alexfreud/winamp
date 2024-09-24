#pragma once
#include "../Plugins/Input/in_flv/svc_flvdecoder.h"
#include "../Plugins/Input/in_flv/FLVAudioHeader.h"
#include "../Plugins/Input/in_flv/ifc_flvaudiodecoder.h"

// {A1249BA5-97F3-4820-9E3B-EB883AE7D19A}
static const GUID flv_adpcm_guid = 
{ 0xa1249ba5, 0x97f3, 0x4820, { 0x9e, 0x3b, 0xeb, 0x88, 0x3a, 0xe7, 0xd1, 0x9a } };


class FLVDecoderCreator : public svc_flvdecoder
{
public:
	static const char *getServiceName() { return "ADPCM FLV Decoder"; }
	static GUID getServiceGuid() { return flv_adpcm_guid; } 
	int CreateAudioDecoder(int stereo, int bits, int sample_rate, int format, ifc_flvaudiodecoder **decoder);
	int HandlesAudio(int format_type);
protected:
	RECVS_DISPATCH;
};

class FLVADPCM : public ifc_flvaudiodecoder
{
public:
	FLVADPCM(unsigned int channels);
	int GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *bits);
	int DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate);
	void Close();
private:
/* data */
	unsigned int channels;

protected:
	RECVS_DISPATCH;
};
