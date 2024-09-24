#pragma once
#include "../Plugins/Input/in_flv/svc_flvdecoder.h"
#include "../Plugins/Input/in_flv/FLVAudioHeader.h"
#include "../Plugins/Input/in_flv/ifc_flvaudiodecoder.h"

#include <mpg123.h>

// {5CAED3F6-ED5F-4801-921E-9BF86A483016}
static const GUID flv_mp3_guid = 
{ 0x5caed3f6, 0xed5f, 0x4801, { 0x92, 0x1e, 0x9b, 0xf8, 0x6a, 0x48, 0x30, 0x16 } };


class FLVDecoderCreator : public svc_flvdecoder
{
public:
	static const char *getServiceName() { return "MP3 FLV Decoder"; }
	static GUID getServiceGuid() { return flv_mp3_guid; } 
	int CreateAudioDecoder(int stereo, int bits, int sample_rate, int format, ifc_flvaudiodecoder **decoder);
	int HandlesAudio(int format_type);
protected:
	RECVS_DISPATCH;
};

class FLVMP3 : public ifc_flvaudiodecoder
{
public:
	FLVMP3(mpg123_handle *mp3);
	~FLVMP3();
	int GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *bits);
	int DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate);
	void Flush();
	void Close();
	void SetPreferences(unsigned int max_channels, unsigned int preferred_bits);
private:
	mpg123_handle *mp3;
	unsigned int bits;
	int pregap;
	unsigned int max_channels;
	unsigned int channels;
	float *decode_buffer;
	size_t decode_buffer_length;
protected:
	RECVS_DISPATCH;
};
