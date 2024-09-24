#include "FLVAACDecoder.h"
#include <math.h>
#include "../nsutil/pcm.h"

int FLVDecoder::CreateAudioDecoder(int stereo, int bits, int sample_rate, int format_type, ifc_flvaudiodecoder **decoder)
{
	if (format_type == FLV::AUDIO_FORMAT_AAC)
	{
		CAccessUnitPtr access_unit = CAccessUnit_Create(0, 0);
		if (!access_unit)
			return CREATEDECODER_FAILURE;

		FLVAAC *aac = new FLVAAC(access_unit);
		if (!aac)
		{
			CAccessUnit_Destroy(&access_unit);
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
FLVAAC::FLVAAC(CAccessUnitPtr access_unit) : access_unit(access_unit)
{
	bps = 16;
	preDelay=0;
	got_decoder_config = false;
	decoder = 0;
	composition_unit = 0;
}

int FLVAAC::GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *_bits)
{
	/* TODO: verify that it's safe to call these, e.g. one frame has been decoded successfully */
	CCompositionUnit_GetSamplingRate(composition_unit, sample_rate);
	CCompositionUnit_GetChannels(composition_unit, channels);

	*_bits = bps;
	return FLV_AUDIO_SUCCESS;
}

int FLVAAC::DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *samples, size_t *samples_size_bytes, double *bitrate)
{
	const uint8_t *type = (const uint8_t *)input_buffer;
	if (type[0] == 0)
	{
		CSAudioSpecificConfig asc;
		memset(&asc, 0, sizeof(asc));
		if (mp4AudioDecoder_ascParse(type+1, input_buffer_bytes-1, &asc) == MP4AUDIODEC_OK)
		{
			CSAudioSpecificConfig *asc_array = &asc;;
			decoder = mp4AudioDecoder_Create(&asc_array, 1);
			if (decoder)
			{
				mp4AudioDecoder_SetParam(decoder, TDL_MODE, SWITCH_OFF);
				mp4AudioDecoder_SetParam(decoder, CONCEALMENT_ENERGYINTERPOLATION, SWITCH_OFF);
				composition_unit = CCompositionUnit_Create(max(asc.m_channels, 8), asc.m_samplesPerFrame * 2, asc.m_samplingFrequency, 6144, CUBUFFER_PCMTYPE_FLOAT);
				if (composition_unit)
				{
					got_decoder_config=true;
					*samples_size_bytes=0;
					return FLV_AUDIO_SUCCESS;
				}
				mp4AudioDecoder_Destroy(&decoder);
			}
		}
					return FLV_AUDIO_FAILURE;
	}
	else if (type[0] == 1)
	{
		CAccessUnit_Reset(access_unit);
		CAccessUnit_Assign(access_unit, type+1, input_buffer_bytes-1);
		CCompositionUnit_Reset(composition_unit);
		MP4_RESULT result = mp4AudioDecoder_DecodeFrame(decoder, &access_unit, composition_unit);

		if (result == MP4AUDIODEC_OK)
		{
			unsigned int channels;
			unsigned int samples_per_channel;
			CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel);
			CCompositionUnit_GetChannels(composition_unit, &channels);
			const float *audio_output = 0;
			size_t num_samples = samples_per_channel * channels;
			size_t output_size = num_samples * (bps/8);
			if (output_size > *samples_size_bytes)
				return FLV_AUDIO_FAILURE;

			*samples_size_bytes = output_size;
			CCompositionUnit_GetPcmPtr(composition_unit, &audio_output);
			nsutil_pcm_FloatToInt_Interleaved_Gain(samples, audio_output, bps, num_samples, 1.0f/32768.0f);

			int br;
			CCompositionUnit_GetProperty(composition_unit, CUBUFFER_CURRENTBITRATE, &br);
			*bitrate = (double)br/1000.0;
			return FLV_AUDIO_SUCCESS;
		}
		else
			return FLV_AUDIO_FAILURE;

	}
	else
		return FLV_AUDIO_FAILURE;
}

void FLVAAC::Flush()
{
	mp4AudioDecoder_Reset(decoder, MP4AUDIODECPARAM_DEFAULT, 0);
}

void FLVAAC::Close()
{
	mp4AudioDecoder_Destroy(&decoder);
	CAccessUnit_Destroy(&access_unit);
	CCompositionUnit_Destroy(&composition_unit);
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
