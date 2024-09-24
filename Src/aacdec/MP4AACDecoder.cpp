#include "MP4AACDecoder.h"
#include "api.h"
#include <math.h>
#include <bfc/platform/minmax.h>
#include "../nsutil/pcm.h"



// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
    { 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

MP4AACDecoder::MP4AACDecoder()
{
	preDelay = 0;
	decoder = 0;
	access_unit = 0;
	composition_unit = 0;
	isFloat = false;
	gain=1.0f;
	// get bps
	bitsPerSample = AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16);
	if (bitsPerSample >= 24)	bitsPerSample = 24;
	else	bitsPerSample = 16;

	// get max channels
	if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"surround", true))
		maxChannels = 6;
	else if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"mono", false))
		maxChannels = 1;
	else
		maxChannels = 2;
}

int MP4AACDecoder::Open()
{
	/* with FhG's API, we can't actually create a decoder until we have the ASC.
	best we can do right now is create the access unit object */
	access_unit = CAccessUnit_Create(0, 0);
	if (access_unit)
		return MP4_SUCCESS;
	else
		return MP4_FAILURE;
}

int MP4AACDecoder::OpenEx(size_t _bits, size_t _maxChannels, bool useFloat)
{
	isFloat = useFloat;
	if (isFloat)
	{
		bitsPerSample = 32;
	}
	else
	{
		if (_bits)
			bitsPerSample = _bits;
		if (bitsPerSample >= 24)	bitsPerSample = 24;
		else	bitsPerSample = 16;
	}
	if (_maxChannels)
		maxChannels = _maxChannels;
	return Open();
}

int MP4AACDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *_bitsPerSample)
{
	bool dummy;
	return GetOutputPropertiesEx(sampleRate, channels, _bitsPerSample, &dummy);
}

int MP4AACDecoder::GetOutputPropertiesEx(unsigned int *sampleRate, unsigned int *channels, unsigned int *_bitsPerSample, bool *useFloat)
{
			/* TODO: verify that it's safe to call these, e.g. one frame has been decoded successfully, otherwise call MKV_NEED_MORE_INPUT */
	CCompositionUnit_GetSamplingRate(composition_unit, sampleRate);
	CCompositionUnit_GetChannels(composition_unit, channels);

	*_bitsPerSample = bitsPerSample;
	*useFloat = isFloat;
	return MP4_SUCCESS;
}

void MP4AACDecoder::Flush()
{
	mp4AudioDecoder_Reset(decoder, MP4AUDIODECPARAM_DEFAULT, 0);
}

int MP4AACDecoder::GetCurrentBitrate(unsigned int *bitrate)
{
	int current_bitrate;
	if (CCompositionUnit_GetProperty(composition_unit, CUBUFFER_CURRENTBITRATE, &current_bitrate) == MP4AUDIODEC_OK)
	{
		*bitrate = current_bitrate/1000;
		return MP4_SUCCESS;
	}
	else
		return MP4_GETCURRENTBITRATE_UNKNOWN;
}

int MP4AACDecoder::DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	CAccessUnit_Reset(access_unit);
	CAccessUnit_Assign(access_unit, (const unsigned char *)inputBuffer, inputBufferBytes);
	CCompositionUnit_Reset(composition_unit);
	MP4_RESULT result = mp4AudioDecoder_DecodeFrame(decoder, &access_unit, composition_unit);

	if (result == MP4AUDIODEC_OK)
	{
		unsigned int channels;
		unsigned int samples_per_channel;
		if (CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel) != MP4AUDIODEC_OK
			||		CCompositionUnit_GetChannels(composition_unit, &channels) != MP4AUDIODEC_OK)
			return MP4_FAILURE;
		const float *audio_output = 0;

		size_t num_samples = samples_per_channel * channels;
		size_t output_size = num_samples * (bitsPerSample/8);
		if (output_size > *outputBufferBytes)
			return MP4_FAILURE;

		*outputBufferBytes = output_size;
		CCompositionUnit_GetPcmPtr(composition_unit, &audio_output);
		if (!isFloat)
		{
			nsutil_pcm_FloatToInt_Interleaved_Gain(outputBuffer, audio_output, bitsPerSample, num_samples, gain/32768.0f);
		}
		else
		{
			for (size_t i = 0;i != num_samples;i++)
				((float *)outputBuffer)[i] = audio_output[i] * gain / 32768.0f;
		}

		return MP4_SUCCESS;
	}
	else
		return MP4_FAILURE;
}

int MP4AACDecoder::OutputFrameSize(size_t *frameSize)
{
	if (!decoder)
		return MP4_FAILURE;

	unsigned int samples_per_channel;
	unsigned int channels;
	if (CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel) != MP4AUDIODEC_OK
		|| 	CCompositionUnit_GetChannels(composition_unit, &channels) != MP4AUDIODEC_OK)
		return MP4_FAILURE;

	*frameSize = samples_per_channel*channels;
	return MP4_SUCCESS;	
}


int MP4AACDecoder::AudioSpecificConfiguration(void *buffer, size_t buffer_size) // reads ASC block from mp4 file
{
	// TODO: error check
	if (buffer && buffer_size)
	{
		CSAudioSpecificConfig asc;
				memset(&asc, 0, sizeof(asc));
		if (mp4AudioDecoder_ascParse((const unsigned char *)buffer, buffer_size, &asc) == MP4AUDIODEC_OK)
		{
			CSAudioSpecificConfig *asc_array = &asc;;
			decoder = mp4AudioDecoder_Create(&asc_array, 1);
			if (decoder)
			{
				mp4AudioDecoder_SetParam(decoder, TDL_MODE, SWITCH_OFF);
				mp4AudioDecoder_SetParam(decoder, CONCEALMENT_ENERGYINTERPOLATION, SWITCH_OFF);
				composition_unit = CCompositionUnit_Create(max(asc.m_channels, 8), asc.m_samplesPerFrame * 2, asc.m_samplingFrequency, 6144, CUBUFFER_PCMTYPE_FLOAT);
				return MP4_SUCCESS;
			}
		}
	}

	return MP4_FAILURE;
}

void MP4AACDecoder::Close()
{
	mp4AudioDecoder_Destroy(&decoder);
	CAccessUnit_Destroy(&access_unit);
	CCompositionUnit_Destroy(&composition_unit);
}

int MP4AACDecoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "mp4a");
}

int MP4AACDecoder::CanHandleType(uint8_t type)
{
	switch (type)
	{
	case MP4_TYPE_MPEG4_AUDIO:
		return 1;
	case MP4_TYPE_MPEG2_AAC_LC_AUDIO:
		return 1;
	default:
		return 0;
	}
}

int MP4AACDecoder::CanHandleMPEG4Type(uint8_t type)
{
	switch (type)
	{
	case MP4_MPEG4_TYPE_AAC_LC_AUDIO:
	case MP4_MPEG4_TYPE_AAC_HE_AUDIO:
	case MP4_MPEG4_TYPE_PARAMETRIC_STEREO:
		return 1;
	default:
		return 0;
	}
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS MP4AACDecoder
START_DISPATCH;
CB(MPEG4_AUDIO_OPEN, Open)
CB(MPEG4_AUDIO_OPEN_EX, OpenEx)
CB(MPEG4_AUDIO_ASC, AudioSpecificConfiguration)
CB(MPEG4_AUDIO_BITRATE, GetCurrentBitrate)
CB(MPEG4_AUDIO_FRAMESIZE, OutputFrameSize)
CB(MPEG4_AUDIO_OUTPUTINFO, GetOutputProperties)
CB(MPEG4_AUDIO_OUTPUTINFO_EX, GetOutputPropertiesEx)
CB(MPEG4_AUDIO_DECODE, DecodeSample)
VCB(MPEG4_AUDIO_FLUSH, Flush)
VCB(MPEG4_AUDIO_CLOSE, Close)
CB(MPEG4_AUDIO_HANDLES_CODEC, CanHandleCodec)
CB(MPEG4_AUDIO_HANDLES_TYPE, CanHandleType)
CB(MPEG4_AUDIO_HANDLES_MPEG4_TYPE, CanHandleMPEG4Type)
CB(MPEG4_AUDIO_SET_GAIN, SetGain)
END_DISPATCH;
