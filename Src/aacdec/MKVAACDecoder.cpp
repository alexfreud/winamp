#include "MKVAACDecoder.h"
#include <math.h>
#include "../nsutil/pcm.h"
MKVAACDecoder::MKVAACDecoder(mp4AudioDecoderHandle decoder, CAccessUnitPtr access_unit, CCompositionUnitPtr composition_unit, unsigned int bps, bool floating_point)
: decoder(decoder), access_unit(access_unit), composition_unit(composition_unit), bps(bps), floating_point(floating_point)
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
		CSAudioSpecificConfig asc;
				memset(&asc, 0, sizeof(asc));
		if (mp4AudioDecoder_ascParse((const unsigned char *)track_entry_data->codec_private, track_entry_data->codec_private_len, &asc) == MP4AUDIODEC_OK)
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
						return new MKVAACDecoder(decoder, access_unit, composition_unit, preferred_bits, floating_point);
					}
					CCompositionUnit_Destroy(&composition_unit);
				}
				mp4AudioDecoder_Destroy(&decoder);
			}
		}
	}

	return 0;
}

int MKVAACDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
		/* TODO: verify that it's safe to call these, e.g. one frame has been decoded successfully, otherwise call MKV_NEED_MORE_INPUT */
	CCompositionUnit_GetSamplingRate(composition_unit, sampleRate);
	CCompositionUnit_GetChannels(composition_unit, channels);

	*bitsPerSample = bps;
	*isFloat = floating_point;
	return MKV_SUCCESS;
}

void MKVAACDecoder::Flush()
{
	mp4AudioDecoder_Reset(decoder, MP4AUDIODECPARAM_DEFAULT, 0);
}

int MKVAACDecoder::OutputFrameSize(size_t *frame_size)
{
		if (!decoder)
		return MKV_FAILURE;

	unsigned int samples_per_channel;
	unsigned int channels;
	if (CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel) != MP4AUDIODEC_OK
		|| 	CCompositionUnit_GetChannels(composition_unit, &channels) != MP4AUDIODEC_OK)
		return MKV_FAILURE;

	*frame_size = samples_per_channel*channels;
	return MKV_SUCCESS;	
}

void MKVAACDecoder::Close()
{
	mp4AudioDecoder_Destroy(&decoder);
	CAccessUnit_Destroy(&access_unit);
	CCompositionUnit_Destroy(&composition_unit);
	delete this;
}

int MKVAACDecoder::DecodeBlock(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	CAccessUnit_Reset(access_unit);
	CAccessUnit_Assign(access_unit, (const unsigned char *)inputBuffer, inputBufferBytes);
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
		if (output_size > *outputBufferBytes)
			return MKV_FAILURE;

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

		return MKV_SUCCESS;
	}
	else
		return MKV_FAILURE;
}

#define CBCLASS MKVAACDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
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