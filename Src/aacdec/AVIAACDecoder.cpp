#include "AVIAACDecoder.h"
#include <math.h>
#include "../nsutil/pcm.h"

int AVIDecoder::CreateAudioDecoder(const nsavi::AVIH *avi_header, 
																	 const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, 
																	 unsigned int preferred_bits, unsigned int max_channels, bool floating_point, 
																	 ifc_aviaudiodecoder **decoder)
{
	nsavi::audio_format *waveformat = (nsavi::audio_format *)stream_format;

	if (waveformat->format == nsavi::audio_format_aac)
	{
		AVIAACDecoder *aac_decoder = AVIAACDecoder::Create(waveformat, preferred_bits, max_channels, floating_point);
		if (aac_decoder)
		{
			*decoder = aac_decoder;
			return CREATEDECODER_SUCCESS;
		}
		return CREATEDECODER_SUCCESS;
	}

	return CREATEDECODER_NOT_MINE;

}

#define CBCLASS AVIDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS

AVIAACDecoder *AVIAACDecoder::Create(const nsavi::audio_format *waveformat, unsigned int preferred_bits, unsigned int max_channels, bool floating_point)
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

	if (waveformat->extra_size_bytes)
	{
		CSAudioSpecificConfig asc;
				memset(&asc, 0, sizeof(asc));
		if (mp4AudioDecoder_ascParse((const unsigned char *)(waveformat + 1), waveformat->extra_size_bytes, &asc) == MP4AUDIODEC_OK)
		{
			CSAudioSpecificConfig *asc_array = &asc;;
			mp4AudioDecoderHandle decoder = mp4AudioDecoder_Create(&asc_array, 1);
			if (decoder)
			{
				mp4AudioDecoder_SetParam(decoder, TDL_MODE, SWITCH_OFF);
				mp4AudioDecoder_SetParam(decoder, CONCEALMENT_ENERGYINTERPOLATION, SWITCH_OFF);
				CCompositionUnitPtr composition_unit = CCompositionUnit_Create(max(asc.m_channels, 8), asc.m_samplesPerFrame * 2, asc.m_samplingFrequency, 6144, CUBUFFER_PCMTYPE_FLOAT);	
				if (composition_unit)
				{
					CAccessUnitPtr access_unit = CAccessUnit_Create(0, 0);
					if (access_unit)
					{
						return new AVIAACDecoder(decoder, access_unit, composition_unit, preferred_bits, floating_point);
					}
					CCompositionUnit_Destroy(&composition_unit);
				}
				mp4AudioDecoder_Destroy(&decoder);
			}
		}
	}

	return 0;
}

AVIAACDecoder::AVIAACDecoder(mp4AudioDecoderHandle decoder, CAccessUnitPtr access_unit, CCompositionUnitPtr composition_unit, unsigned int bps, bool floating_point)
: decoder(decoder), access_unit(access_unit), composition_unit(composition_unit), bps(bps), floating_point(floating_point)
{
}

int AVIAACDecoder::OutputFrameSize(size_t *frame_size)
{
	if (!decoder)
		return AVI_FAILURE;

	unsigned int samples_per_channel;
	unsigned int channels;
	if (CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel) != MP4AUDIODEC_OK
|| 	CCompositionUnit_GetChannels(composition_unit, &channels) != MP4AUDIODEC_OK)
		return AVI_FAILURE;

	*frame_size = samples_per_channel*channels;
	return AVI_SUCCESS;
}

int AVIAACDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	/* TODO: verify that it's safe to call these, e.g. one frame has been decoded successfully */
	CCompositionUnit_GetSamplingRate(composition_unit, sampleRate);
	CCompositionUnit_GetChannels(composition_unit, channels);

	*bitsPerSample = bps;
	*isFloat = floating_point;
	return AVI_SUCCESS;
}

int AVIAACDecoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	CAccessUnit_Reset(access_unit);
	CAccessUnit_Assign(access_unit, (const unsigned char *)*inputBuffer, *inputBufferBytes);
	CCompositionUnit_Reset(composition_unit);
	MP4_RESULT result = mp4AudioDecoder_DecodeFrame(decoder, &access_unit, composition_unit);

	if (result == MP4AUDIODEC_OK)
	{
		unsigned int bits_remaining;
		unsigned int bits_read;
		CAccessUnit_GetBitCount(access_unit, &bits_read);
		CAccessUnit_GetValidBits(access_unit, &bits_remaining);
		*inputBuffer = (uint8_t *)(*inputBuffer) + bits_read/8;
		*inputBufferBytes = bits_remaining/8;

		unsigned int channels;
		unsigned int samples_per_channel;
		CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel);
		CCompositionUnit_GetChannels(composition_unit, &channels);
		const float *audio_output = 0;
		size_t num_samples = samples_per_channel * channels;
		size_t output_size = num_samples * (bps/8);
		if (output_size > *outputBufferBytes)
			return AVI_FAILURE;

		*outputBufferBytes = output_size;
		CCompositionUnit_GetPcmPtr(composition_unit, &audio_output);
		if (!floating_point)
		{
			nsutil_pcm_FloatToInt_Interleaved_Gain(outputBuffer, audio_output, bps, num_samples, 1.0f/32768.0f);
		}
		else
		{
			for (size_t i = 0;i != num_samples;i++)
				((float *)outputBuffer)[i] = audio_output[i] / 32768.0f;
		}

		return AVI_SUCCESS;
	}
	else
		return AVI_FAILURE;

}

void AVIAACDecoder::Flush()
{
	mp4AudioDecoder_Reset(decoder, MP4AUDIODECPARAM_DEFAULT, 0);
}

void AVIAACDecoder::Close()
{
	mp4AudioDecoder_Destroy(&decoder);
	CAccessUnit_Destroy(&access_unit);
	CCompositionUnit_Destroy(&composition_unit);

	delete this;
}

#define CBCLASS AVIAACDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS