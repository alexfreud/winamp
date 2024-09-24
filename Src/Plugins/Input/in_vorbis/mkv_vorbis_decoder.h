#pragma once
#include "../in_mkv/ifc_mkvaudiodecoder.h"
#include "../in_mkv/svc_mkvdecoder.h"
#include <vorbis/codec.h>

// {6058D315-2F08-4b2f-903E-4C2E6B5EFFA9}
static const GUID mkv_vorbis_guid = 
{ 0x6058d315, 0x2f08, 0x4b2f, { 0x90, 0x3e, 0x4c, 0x2e, 0x6b, 0x5e, 0xff, 0xa9 } };


class MKVDecoderCreator : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "Vorbis MKV Decoder"; }
	static GUID getServiceGuid() { return mkv_vorbis_guid; } 
	int CreateAudioDecoder(const char *codec_id,
		const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, 
		unsigned int preferred_bits, unsigned int max_channels, bool floating_point, 
		ifc_mkvaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class MKVVorbis : public ifc_mkvaudiodecoder
{
public:
	MKVVorbis();
	int DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat);
	void Flush();
	void Close();
//private:
	unsigned int bps;
	vorbis_info info;
	vorbis_dsp_state dsp;
	vorbis_block block;
	vorbis_comment comment;
	ogg_int64_t packet_number;
protected:
	RECVS_DISPATCH;
};