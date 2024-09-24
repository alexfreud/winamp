#pragma once
#include "../Plugins/Input/in_mkv/ifc_mkvaudiodecoder.h"
#include "MFTDecoder.h"
#include "../nsmkv/Tracks.h"
#include "../Plugins/Input/in_mkv/svc_mkvdecoder.h"

// {437C68FA-2972-4ede-A157-CBD371E5E263}
static const GUID AACMKVGUID = 
{ 0x437c68fa, 0x2972, 0x4ede, { 0xa1, 0x57, 0xcb, 0xd3, 0x71, 0xe5, 0xe2, 0x63 } };


class MKVDecoder : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "MFT AAC MKV Decoder"; }
	static GUID getServiceGuid() { return AACMKVGUID; }
	int CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int preferred_channels, bool floating_point, ifc_mkvaudiodecoder **decoder);
protected:
	RECVS_DISPATCH;
};

class MKVAACDecoder : public ifc_mkvaudiodecoder
{
public:
	static MKVAACDecoder *Create(const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point);

protected:
	RECVS_DISPATCH;
private:
	/* ifc_mkvaudiodecoder implementation */
	int OutputFrameSize(size_t *frame_size);
	int GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat);
	int DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes);
	void Flush();
	void Close();
	void EndOfStream(); // no more input, output anything you have buffered
private:
	MKVAACDecoder(unsigned int bps, bool floating_point);

	/* internal implementation */

	/* data */
	MFTDecoder decoder;
	unsigned int bps;
	bool floating_point;
};