#pragma once
#include "../in_mkv/ifc_mkvaudiodecoder.h"
#include "incs/mp4AudioDecIfc.h"
#include "../nsmkv/Tracks.h"
#include "../in_mkv/svc_mkvdecoder.h"

// {7405C1CE-6CD6-4975-B22B-8DBABAA64C44}
static const GUID AACMKVGUID = 
{ 0x7405c1ce, 0x6cd6, 0x4975, { 0xb2, 0x2b, 0x8d, 0xba, 0xba, 0xa6, 0x4c, 0x44 } };


class MKVDecoder : public svc_mkvdecoder
{
public:
	static const char *getServiceName() { return "AAC MKV Decoder"; }
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

private:
	MKVAACDecoder(mp4AudioDecoderHandle decoder, CAccessUnitPtr access_unit, CCompositionUnitPtr composition_unit, unsigned int bps, bool floating_point);
	
	/* internal implementation */

	/* data */
	mp4AudioDecoderHandle decoder;
	CCompositionUnitPtr composition_unit; /* output */
	CAccessUnitPtr access_unit; /* input */
	unsigned int bps;
	bool floating_point;
	};