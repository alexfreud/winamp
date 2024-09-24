#include "avi_pcm_decoder.h"
#include "avi_alaw_decoder.h"
#include "avi_ulaw_decoder.h"

int AVIDecoder::CreateAudioDecoder(const nsavi::AVIH *avi_header, 
																	 const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, 
																	 unsigned int preferred_bits, unsigned int max_channels, bool floating_point, 
																	 ifc_aviaudiodecoder **decoder)
{
	nsavi::audio_format *waveformat = (nsavi::audio_format *)stream_format;

	if (waveformat->format == nsavi::audio_format_pcm		|| waveformat->format == nsavi::audio_format_extensible)
	{
		*decoder = new AVIPCMDecoder(waveformat);
		return CREATEDECODER_SUCCESS;
	}
	else if (waveformat->format == nsavi::audio_format_alaw)
	{
			*decoder = new AVIALawDecoder(waveformat);
		return CREATEDECODER_SUCCESS;
	}
		else if (waveformat->format == nsavi::audio_format_ulaw)
	{
			*decoder = new AVIULawDecoder(waveformat);
		return CREATEDECODER_SUCCESS;
	}

	return CREATEDECODER_NOT_MINE;

}

#define CBCLASS AVIDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS

AVIPCMDecoder::AVIPCMDecoder(const nsavi::audio_format *waveformat) : waveformat(waveformat)
{
}

int AVIPCMDecoder::OutputFrameSize(size_t *frame_size)
{
	*frame_size = waveformat->block_align; // TODO
	return AVI_SUCCESS;
}

int AVIPCMDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	if (waveformat)
	{
		*sampleRate = waveformat->sample_rate;
		*channels = waveformat->channels;
		*bitsPerSample = waveformat->bits_per_sample;
		*isFloat = false; // TODO
		return AVI_SUCCESS;
	}
	else
	{
		return AVI_FAILURE;
	}
}

int AVIPCMDecoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	size_t copy_size = min(*inputBufferBytes, *outputBufferBytes);
	int remainder = copy_size % waveformat->block_align;
	copy_size -= remainder;
	memcpy(outputBuffer, *inputBuffer, copy_size);
	*inputBuffer = (uint8_t *)(*inputBuffer) + copy_size;
	*inputBufferBytes = *inputBufferBytes - copy_size;
	*outputBufferBytes = copy_size;
	return AVI_SUCCESS;
}

void AVIPCMDecoder::Close()
{
	delete this;
}

#define CBCLASS AVIPCMDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS