#include "FLVAACDecoder.h"
#include <math.h>
#include "../nsutil/pcm.h"

int FLVDecoder::CreateAudioDecoder(int stereo, int bits, int sample_rate, int format_type, ifc_flvaudiodecoder **decoder)
{
	if (format_type == FLV::AUDIO_FORMAT_AAC)
	{
		FLVAAC *aac = new FLVAAC();
		if (!aac)
		{
			return CREATEDECODER_FAILURE;
		}
		*decoder = aac;
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

int FLVDecoder::HandlesAudio(int format_type)
{
	if (format_type == FLV::AUDIO_FORMAT_AAC)
	{
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS FLVDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
CB(HANDLES_AUDIO, HandlesAudio)
END_DISPATCH;
#undef CBCLASS

/* --- */
FLVAAC::FLVAAC()
{
	bps = 16;
	preDelay=0;
	got_decoder_config = false;
}

int FLVAAC::GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *_bits)
{
	uint32_t local_sample_rate, local_channels;
	HRESULT hr = decoder.GetOutputProperties(&local_sample_rate, &local_channels);
	if (FAILED(hr)) {
		return FLV_AUDIO_FAILURE;
	}

	*sample_rate = local_sample_rate;
	*channels = local_channels;

	*_bits = bps;
	return FLV_AUDIO_SUCCESS;
}

int FLVAAC::DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate)
{
	const uint8_t *type = (const uint8_t *)input_buffer;
	if (type[0] == 0)
	{
		decoder.Open(type+1, input_buffer_bytes-1);

		got_decoder_config=true;
		*samples_size_bytes=0;
		return FLV_AUDIO_SUCCESS;
		return FLV_AUDIO_FAILURE;
	}
	else if (type[0] == 1)
	{
		decoder.Feed(input_buffer, input_buffer_bytes);
		decoder.Decode(samples, samples_size_bytes, bps, false, 1.0);
		*bitrate = 0;
		return FLV_AUDIO_SUCCESS;
		
	}
	else
		return FLV_AUDIO_FAILURE;
}

void FLVAAC::Flush()
{
	decoder.Flush();
}

void FLVAAC::Close()
{
	delete this;
}

int FLVAAC::Ready()
{
	return !!got_decoder_config;
}

void FLVAAC::SetPreferences(unsigned int _max_channels, unsigned int preferred_bits)
{
	if (preferred_bits)
		bps = preferred_bits;

	// TODO: max channels
}

#define CBCLASS FLVAAC
START_DISPATCH;
CB(FLV_AUDIO_GETOUTPUTFORMAT, GetOutputFormat)
CB(FLV_AUDIO_DECODE, DecodeSample)
VCB(FLV_AUDIO_FLUSH, Flush)
VCB(FLV_AUDIO_CLOSE, Close)
CB(FLV_AUDIO_READY, Ready)
VCB(FLV_AUDIO_SETPREFERENCES, SetPreferences)
END_DISPATCH;
#undef CBCLASS
