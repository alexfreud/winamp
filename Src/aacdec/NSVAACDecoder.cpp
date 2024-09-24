#include "NSVAACDecoder.h"

#include <assert.h>
#include "api.h"
#include "../nsv/nsvlib.h"
#include "api.h"
#include "../nsv/nsvlib.h"
#include "../nsv/dec_if.h"
#include <string.h>
#include <bfc/platform/export.h>
#include "NSVAACDecoder.h"
#include <bfc/error.h>
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

NSVAACDecoder *NSVAACDecoder::CreateDecoder()
{
	CAccessUnitPtr access_unit = CAccessUnit_Create(0, 0);
	if (!access_unit)
		return 0;

	NSVAACDecoder *decoder=0;
	WASABI_API_MEMMGR->New(&decoder);
	if (!decoder)
	{
		CAccessUnit_Destroy(&access_unit);
		return 0;
	}
	decoder->Initialize(access_unit);
	return decoder;
}

NSVAACDecoder::NSVAACDecoder()
{
	access_unit = 0;
	composition_unit = 0;
	decoder = 0;
	source_position=0;
	out_left=0;
	in_position=0;
}

NSVAACDecoder::~NSVAACDecoder()
{
	mp4AudioDecoder_Destroy(&decoder);
	CAccessUnit_Destroy(&access_unit);
	CCompositionUnit_Destroy(&composition_unit);
}

void NSVAACDecoder::Initialize(CAccessUnitPtr _access_unit)
{
	access_unit = _access_unit;
}

void NSVAACDecoder::flush()
{
	if (decoder)
		mp4AudioDecoder_Reset(decoder, MP4AUDIODECPARAM_DEFAULT, 0);
}


static void ConfigureADTS(CSAudioSpecificConfig* asc, nsaac_adts_header_t header)
{
	asc->m_aot = (AUDIO_OBJECT_TYPE)(header->profile + 1);
	asc->m_channelConfiguration = header->channel_configuration;
	asc->m_channels = nsaac_adts_get_channel_count(header);
	asc->m_nrOfStreams = 1;
	asc->m_samplesPerFrame = 1024;
	asc->m_samplingFrequencyIndex = header->sample_rate_index;
	asc->m_samplingFrequency = nsaac_adts_get_samplerate(header);
	asc->m_avgBitRate = 0;  /* only needed for tvq */
	asc->m_mpsPresentFlag   = -1;
	asc->m_saocPresentFlag  = -1;
	asc->m_ldmpsPresentFlag = -1;
}

// returns -1 on error, 0 on success (done with data in 'in'), 1 on success
// but to pass 'in' again next time around.
int NSVAACDecoder::decode(void *in, int in_len, void *out, int *out_len, unsigned int out_fmt[8])
{
	if (out_left)
	{
		unsigned int channels;
		unsigned int sample_rate;
		if (CCompositionUnit_GetChannels(composition_unit, &channels) != MP4AUDIODEC_OK
			||  CCompositionUnit_GetSamplingRate(composition_unit, &sample_rate) != MP4AUDIODEC_OK)
			return -1;


		out_fmt[0] = NSV_MAKETYPE('P', 'C', 'M', ' ');
		out_fmt[1] = sample_rate;
		out_fmt[2] = channels;
		out_fmt[3] = 16;

		const uint8_t *audio_output=0;
		CCompositionUnit_GetPcmPtr(composition_unit, &audio_output);
		size_t copy_size = min(out_left, *out_len);
		memcpy(out, audio_output + source_position, copy_size);
		*out_len = copy_size;
		out_left -= copy_size;
		source_position += copy_size;
		return 1;
;
	}

	in = (uint8_t *)in + in_position;
	in_len -= in_position;

	if (in_len > 7)
	{
		ADTSHeader header;
		if (nsaac_adts_parse(&header, (const uint8_t *)in) == NErr_Success)
		{
			if (!decoder)
			{
				CSAudioSpecificConfig asc;
				memset(&asc, 0, sizeof(asc));
				ConfigureADTS(&asc, &header);

				CSAudioSpecificConfig *asc_array = &asc;
				decoder = mp4AudioDecoder_Create(&asc_array, 1);
				if (decoder)
				{
					mp4AudioDecoder_SetParam(decoder, TDL_MODE, SWITCH_ON);
					mp4AudioDecoder_SetParam(decoder, CONCEALMENT_ENERGYINTERPOLATION, SWITCH_OFF);
					composition_unit = CCompositionUnit_Create(max(asc.m_channels, 8), asc.m_samplesPerFrame * 2, asc.m_samplingFrequency, 6144, CUBUFFER_PCMTYPE_INT16);	
				}
				if (!decoder || !composition_unit)
				{
					in_position=0;
					return -1;
				}
			}
			if (header.frame_length > in_len)
			{
				in_position=0;
				return -1;
			}
			if (header.frame_length != in_len)
			{
				in_position+=header.frame_length;
			}
			else
			{
				in_position=0;
			}
			
			CAccessUnit_Reset(access_unit);
			CAccessUnit_Assign(access_unit, (const uint8_t *)in + 7, header.frame_length-7);
			CCompositionUnit_Reset(composition_unit);
			MP4_RESULT result = mp4AudioDecoder_DecodeFrame(decoder, &access_unit, composition_unit);

			if (result == MP4AUDIODEC_OK)
			{
				unsigned int channels;
				unsigned int samples_per_channel;
				unsigned int sample_rate;
				if (CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel) != MP4AUDIODEC_OK
					||		CCompositionUnit_GetChannels(composition_unit, &channels) != MP4AUDIODEC_OK
					||  CCompositionUnit_GetSamplingRate(composition_unit, &sample_rate) != MP4AUDIODEC_OK)
					return -1;

				size_t num_samples = samples_per_channel * channels;
				size_t output_size = num_samples * 2 /* 16 bits */;

				const uint16_t *audio_output=0;
				CCompositionUnit_GetPcmPtr(composition_unit, &audio_output);
				size_t copy_size = min(output_size, *out_len);
				memcpy(out, audio_output, copy_size);
				*out_len = copy_size;
				out_left = output_size - copy_size;
				source_position = copy_size;

				out_fmt[0] = NSV_MAKETYPE('P', 'C', 'M', ' ');
				out_fmt[1] = sample_rate;
				out_fmt[2] = channels;
				out_fmt[3] = 16;

				int br;
				CCompositionUnit_GetProperty(composition_unit, CUBUFFER_AVGBITRATE, &br);
				out_fmt[4] =br/1000;
				if (in_position)
					return 1;
				return 0;
			}
			else
			{
				return -1;
			}
			
		}
		else
		{
			in_position=0;
			return -1;
		}
	}
	*out_len = 0;
	in_position=0;
	return 0;

}



IAudioDecoder *NSVDecoder::CreateAudioDecoder(FOURCC format, IAudioOutput **output)
{
	switch (format)
	{
	case NSV_MAKETYPE('A', 'A', 'C', ' ') :							
	case NSV_MAKETYPE('A', 'A', 'C', 'P'):
	case NSV_MAKETYPE('A', 'P', 'L', ' '):
		{
			NSVAACDecoder *dec = NSVAACDecoder::CreateDecoder();
			return dec;
		}

	default:
		return 0;
	}
}


#define CBCLASS NSVDecoder
START_DISPATCH;
CB(SVC_NSVFACTORY_CREATEAUDIODECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS
