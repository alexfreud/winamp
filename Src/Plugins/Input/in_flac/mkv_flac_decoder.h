#pragma once
#include "../in_mkv/svc_mkvdecoder.h"
#include "../in_mkv/ifc_mkvaudiodecoder.h"
#include <FLAC/all.h>

// {F6AF0AD9-608F-4206-892F-765412574A7D}
static const GUID FLACMKVGUID = 
{ 0xf6af0ad9, 0x608f, 0x4206, { 0x89, 0x2f, 0x76, 0x54, 0x12, 0x57, 0x4a, 0x7d } };



class packet_client_data_s
{
public:
	const uint8_t *buffer;
	size_t buffer_length;

	void *outputBuffer;
	size_t *outputBufferBytes;

	uint32_t frame_size;
	uint32_t bps;
	uint32_t bytes_per_sample;
	uint32_t channels;
	uint32_t sample_rate;
	uint64_t samples;
};

typedef packet_client_data_s *packet_client_data_t;

class MKVDecoder : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "FLAC MKV Decoder"; }
	static GUID getServiceGuid() { return FLACMKVGUID; }
	int CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int preferred_channels, bool floating_point, ifc_mkvaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class MKVFLACDecoder : public ifc_mkvaudiodecoder
{
public:
	static MKVFLACDecoder *Create(const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels);
	
protected:
	RECVS_DISPATCH;
private:
	/* ifc_mkvaudiodecoder implementation */
	int OutputFrameSize(size_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat);
	int DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Flush();
	void Close();

private:
	MKVFLACDecoder(FLAC__StreamDecoder *decoder, packet_client_data_t packet, unsigned int bps);
	~MKVFLACDecoder();
	/* internal implementation */

	/* data */
	FLAC__StreamDecoder *decoder;
	unsigned int bps;
packet_client_data_t packet;
};