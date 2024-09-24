#include "MKVAACDecoder.h"
#include <math.h>
#include "../nsutil/pcm.h"

MKVAACDecoder::MKVAACDecoder(unsigned int bps, bool floating_point)
:  bps(bps), floating_point(floating_point)
{
	
}

MKVAACDecoder *MKVAACDecoder::Create(const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels, bool floating_point)
{
	if (!floating_point)
	{
		if (preferred_bits >= 24) 
			preferred_bits=24;
		else
			preferred_bits=16;
	}
	/*if (!max_channels)
		max_channels = 8;*/

	if (track_entry_data->codec_private && track_entry_data->codec_private_len)
	{
		MKVAACDecoder *decoder = new MKVAACDecoder(preferred_bits, floating_point);
		if (decoder && SUCCEEDED(decoder->decoder.Open(track_entry_data->codec_private, track_entry_data->codec_private_len))) {
			return decoder;
		}
		delete decoder;
	}

	return 0;
}

int MKVAACDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	uint32_t local_sample_rate, local_channels;
	HRESULT hr = decoder.GetOutputProperties(&local_sample_rate, &local_channels);
	if (FAILED(hr)) {
		return MKV_FAILURE;
	}

	*sampleRate = local_sample_rate;
	*channels = local_channels;

	*bitsPerSample = bps;
	*isFloat = floating_point;
	return MKV_SUCCESS;
}

void MKVAACDecoder::Flush()
{
	decoder.Flush();
}

int MKVAACDecoder::OutputFrameSize(size_t *frame_size)
{
	size_t local_frame_size;
	if (FAILED(decoder.OutputBlockSizeSamples(&local_frame_size))) {
		return MKV_FAILURE;
	}
	*frame_size = local_frame_size;

	return MKV_SUCCESS;	
}

void MKVAACDecoder::Close()
{
	delete this;
}

int MKVAACDecoder::DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	decoder.Feed(inputBuffer, inputBufferBytes);
	decoder.Decode(outputBuffer, outputBufferBytes, bps, false, 1.0);
	return MKV_SUCCESS;
}

void MKVAACDecoder::EndOfStream()
{
	decoder.Feed(0, 0);
}

#define CBCLASS MKVAACDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
VCB(END_OF_STREAM, EndOfStream)
END_DISPATCH;
#undef CBCLASS


int MKVDecoder::CreateAudioDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::AudioData *audio_data, unsigned int preferred_bits, unsigned int max_channels,bool floating_point, ifc_mkvaudiodecoder **decoder)
{
	if (!strcmp(codec_id, "A_AAC"))
	{
		MKVAACDecoder *aac_decoder = MKVAACDecoder::Create(track_entry_data, audio_data, preferred_bits, max_channels, floating_point);
		if (aac_decoder)
		{
			*decoder = aac_decoder;
			return CREATEDECODER_SUCCESS;
		}
		return CREATEDECODER_FAILURE;
	}

	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS MKVDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS