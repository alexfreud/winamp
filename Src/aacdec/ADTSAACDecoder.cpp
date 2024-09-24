#include "ADTSAACDecoder.h"
#include "ADTSHeader.h"
#include "../nsutil/pcm.h"
#include <bfc/error.h>

#define PRE_PADDING_MAGIC_WORD 'lluN'
#define POST_PADDING_MAGIC_WORD 'tfos'

#pragma pack(push, 1)
struct AncillaryData
{
	int magicWord; // set to 'lluN' for pre-delay, 'tfos' for post-delay 
	unsigned short padding;
};
#pragma pack(pop)

ADTSAACDecoder::ADTSAACDecoder()
{
	predelay = 0;
	decoder = 0;
	access_unit = 0;
	composition_unit = 0;
	useFloat = false; /* we'll fix during Initialize */
	gain=1.0f; /* we'll fix during Initialize */
	channels = 2; /* we'll fix during Initialize */
	// get bps
	bitsPerSample = 16; /* we'll fix during Initialize */
	allowRG = false; /* we'll fix during Initialize */
}

int ADTSAACDecoder::Initialize(bool forceMono, bool reverse_stereo, bool allowSurround, int maxBits, bool _allowRG, bool _useFloat, bool _useCRC)
{
	allowRG = _allowRG;
	useFloat = _useFloat;

	if (_useFloat)
	{
		bitsPerSample = 32;
	}
	else if (maxBits >= 24)
	{
		bitsPerSample = 24;
	}
	else
	{
		bitsPerSample = 16;
	}

	if (forceMono)
		channels = 1;
	else if (allowSurround)
		channels = 8;
	else
		channels = 2;


	/* with FhG's API, we can't actually create a decoder until we have the ASC.
	best we can do right now is create the access unit object */
	access_unit = CAccessUnit_Create(0, 0);
	if (access_unit)
		return adts::SUCCESS;
	else
		return adts::FAILURE;	
}

bool ADTSAACDecoder::Open(ifc_mpeg_stream_reader *file)
{
	if (allowRG)
		gain = file->MPEGStream_Gain();

	return true;
}

void ADTSAACDecoder::Close()
{
	mp4AudioDecoder_Destroy(&decoder);
	decoder=0;
	CAccessUnit_Destroy(&access_unit);
	CCompositionUnit_Destroy(&composition_unit);
}

void ADTSAACDecoder::GetOutputParameters(size_t *numBits, int *numChannels, int *sampleRate)
{
	CCompositionUnit_GetSamplingRate(composition_unit, (unsigned int *)sampleRate);
	

	*numChannels = channels;
	*numBits = bitsPerSample;
}

void ADTSAACDecoder::CalculateFrameSize(int *frameSize)
{
	unsigned int samples_per_channel;
	if (decoder && CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel) == MP4AUDIODEC_OK)
		*frameSize = samples_per_channel*channels;
	else
		*frameSize = 0;
}

void ADTSAACDecoder::Flush(ifc_mpeg_stream_reader *file)
{
	mp4AudioDecoder_Reset(decoder, MP4AUDIODECPARAM_DEFAULT, 0);
}

size_t ADTSAACDecoder::GetCurrentBitrate()
{
	int current_bitrate;
	if (CCompositionUnit_GetProperty(composition_unit, CUBUFFER_AVGBITRATE, &current_bitrate) == MP4AUDIODEC_OK)
	{
		return current_bitrate/1000;
	}
	else
		return 0;
}

size_t ADTSAACDecoder::GetDecoderDelay()
{
	return predelay;
}

static int ADTSSync(const uint8_t *buffer, size_t bytes_in_buffer, size_t *header_position)
{
	for (size_t position=0;position<bytes_in_buffer;position++)
	{
		// find POTENTIAL sync
		if (buffer[position] == 0xFF && bytes_in_buffer - position >= 7)
		{
			ADTSHeader header;
			if (nsaac_adts_parse(&header, &buffer[position]) == NErr_Success)
			{
				int frame_length = header.frame_length;
				if (frame_length && bytes_in_buffer - position - frame_length >= 7)
				{
					ADTSHeader header2;
					if (nsaac_adts_parse(&header2, &buffer[position+frame_length]) == NErr_Success)
					{
						// verify that parameters match
						if (nsaac_adts_match(&header, &header2) != NErr_True)
							return NErr_Changed;

						// do a dummy read to advance the stream
						*header_position = position;
						return NErr_Success;
					}
				}
				else
				{
					/* not enough in the buffer to verify the next header */
					*header_position = position;
					return NErr_NeedMoreData;
				}
			}
		}
	}
	return NErr_False;	
}

static int ReturnIsEOF(ifc_mpeg_stream_reader *file)
{
	if (file->MPEGStream_EOF())
		return adts::ENDOFFILE;
	else
		return adts::NEEDMOREDATA;
}

int ADTSAACDecoder::Internal_Decode(ifc_mpeg_stream_reader *file, const void *input, size_t input_length, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut)
{
	CAccessUnit_Reset(access_unit);
	CAccessUnit_Assign(access_unit, (const unsigned char *)input, input_length);
	CCompositionUnit_Reset(composition_unit);
	MP4_RESULT result = mp4AudioDecoder_DecodeFrame(decoder, &access_unit, composition_unit);

	if (result == MP4AUDIODEC_OK)
	{
		/* check ancillary data for gapless data */
		unsigned int ancillary_fields, ancillary_bytes;
		if (CCompositionUnit_GetAncDataCount(composition_unit, &ancillary_fields, &ancillary_bytes) == MP4AUDIODEC_OK)
		{
			for (unsigned int i=0;i<ancillary_fields;i++)
			{
				unsigned char *ancillary_data;
				unsigned int ancillary_size;
				unsigned int ancillary_tag;
				CCompositionUnit_GetAncDataByPos(composition_unit, i, &ancillary_data, &ancillary_size, &ancillary_tag);
				if (ancillary_tag == ANCDATA_IS_AAC_DSE_TAG15 && ancillary_size == 6)
				{
					/* this is only safe on x86 because of alignment and endian */
					const AncillaryData *data = (const AncillaryData *)ancillary_data;
					if (data->magicWord == PRE_PADDING_MAGIC_WORD)
					{
						predelay = data->padding;
					} 
					else if (data->magicWord == POST_PADDING_MAGIC_WORD)
					{
						*endCut = data->padding;
					}
				}
			}
		}

		unsigned int channels;
		unsigned int samples_per_channel;
		if (CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel) != MP4AUDIODEC_OK
			||		CCompositionUnit_GetChannels(composition_unit, &channels) != MP4AUDIODEC_OK)
			return adts::FAILURE;

		if (samples_per_channel == 0)
			return adts::NEEDMOREDATA;
		const float *audio_output = 0;

		size_t num_samples = samples_per_channel * channels;
		size_t output_size = num_samples * (bitsPerSample/8);
		if (output_size > outputSize)
			return adts::FAILURE;

		*outputWritten = output_size;
		CCompositionUnit_GetPcmPtr(composition_unit, &audio_output);
		if (!useFloat)
		{
			nsutil_pcm_FloatToInt_Interleaved_Gain(output, audio_output, bitsPerSample, num_samples, gain/32768.0f);
		}
		else
		{
			for (size_t i = 0;i != num_samples;i++)
				((float *)output)[i] = audio_output[i]  * gain / 32768.0f;
		}

		int br;
		CCompositionUnit_GetProperty(composition_unit, CUBUFFER_CURRENTBITRATE, &br);
		*bitrate = br/1000;

		return adts::SUCCESS;
	}
	else
		return adts::FAILURE;
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

int ADTSAACDecoder::Sync(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate)
{
	/* ok this will be interesting.  we'll peek from the input buffer and try to synchronize on an ADTS header */
	uint8_t peek_buffer[16384] = {0};
	size_t bytes_read = 0;
	if (file->MPEGStream_Peek(peek_buffer, sizeof(peek_buffer), &bytes_read) != 0)
	{
		return adts::FAILURE;
	}

	size_t header_position=0;
	int ret = ADTSSync(peek_buffer, bytes_read, &header_position);
	if (ret == NErr_NeedMoreData)
	{ 
		// this one means we found one sync but not enough to verify the next frame

		// if the header was at the start of the block, then unfortunately this might be the LAST adts frame in the file, so let's just pass it the decoder and hope for the best
		if (header_position != 0) 
		{
			if (file->MPEGStream_EOF())
				return adts::ENDOFFILE;
	
			/* dummy read to advance the stream */
			file->MPEGStream_Read(peek_buffer, header_position, &header_position);
			return adts::NEEDMOREDATA;
		}
	}
	else if (ret == NErr_False)
	{
		if (file->MPEGStream_EOF())
			return adts::ENDOFFILE;

		// not even a potential sync found
		/* dummy read to advance the stream */
		file->MPEGStream_Read(peek_buffer, bytes_read, &bytes_read);
		return adts::NEEDMOREDATA;
	}
	else if (ret != NErr_Success)
	{
		if (file->MPEGStream_EOF())
			return adts::ENDOFFILE;

		return adts::FAILURE;
	}


	ADTSHeader header;
	if (nsaac_adts_parse(&header, &peek_buffer[header_position]) == NErr_Success)
	{
		CSAudioSpecificConfig asc;
		memset(&asc, 0, sizeof(asc));
		ConfigureADTS(&asc, &header);

		if (!decoder)
		{
		CSAudioSpecificConfig *asc_array = &asc;
		decoder = mp4AudioDecoder_Create(&asc_array, 1);
		if (decoder)
		{
			mp4AudioDecoder_SetParam(decoder, TDL_MODE, SWITCH_OFF);
			mp4AudioDecoder_SetParam(decoder, CONCEALMENT_ENERGYINTERPOLATION, SWITCH_OFF);
			composition_unit = CCompositionUnit_Create(max(asc.m_channels, 8), asc.m_samplesPerFrame * 2, asc.m_samplingFrequency, 6144, CUBUFFER_PCMTYPE_FLOAT);	
		}
		if (!decoder || !composition_unit)
			return adts::FAILURE;
		}

		/* all this error checking might be uncessary, since in theory we did a successful peek above.  but you never know ... */
		if (file->MPEGStream_Read(peek_buffer, header_position, &bytes_read))	/* dummy read to advance the stream */
			return adts::FAILURE;

		if (bytes_read != header_position)
			return ReturnIsEOF(file);

		if (file->MPEGStream_Read(peek_buffer, header.frame_length, &bytes_read)) /* read ADTS frame */
			return adts::FAILURE;

		if (bytes_read != header.frame_length) 
			return ReturnIsEOF(file);

		if (bytes_read < 7) /* bad header data? */
			return adts::FAILURE;

		/* ok, we've created the decoder, but we should really decode the frame to see if there's VBR, PS or MPEGS in it */
		size_t header_size = nsaac_adts_get_header_size(&header);

		size_t endCut;
		int ret = Internal_Decode(file, peek_buffer+header_size, bytes_read-header_size, output, outputSize, outputWritten, bitrate, &endCut);
		if (ret == adts::SUCCESS)
			CCompositionUnit_GetChannels(composition_unit, &channels);
		return ret;
	}
	return adts::FAILURE;
}

int ADTSAACDecoder::Decode(ifc_mpeg_stream_reader *file, unsigned __int8 *output, size_t outputSize, size_t *outputWritten, size_t *bitrate, size_t *endCut)
{
	uint8_t peek_buffer[8192] = {0};
	size_t bytes_read = 0;

	file->MPEGStream_Peek(peek_buffer, 7, &bytes_read);
	if (bytes_read != 7)
		return ReturnIsEOF(file);

	ADTSHeader header;
	if (nsaac_adts_parse(&header, peek_buffer) == NErr_Success)
	{
		if (header.frame_length < 7)
			return adts::FAILURE;

		file->MPEGStream_Peek(peek_buffer, header.frame_length, &bytes_read);
		if (bytes_read != header.frame_length)
			return ReturnIsEOF(file);
		file->MPEGStream_Read(peek_buffer, header.frame_length, &bytes_read);

		size_t header_size = nsaac_adts_get_header_size(&header);
		return Internal_Decode(file, peek_buffer+header_size, bytes_read-header_size, output, outputSize, outputWritten, bitrate, endCut);
	}
	else
	{
		/* Resynchronize */
		return Sync(file, output, outputSize, outputWritten, bitrate);
	}

}

int ADTSAACDecoder::GetLayer()
{
	return 4;
}

void ADTSAACDecoder::Release()
{
	delete this;
}