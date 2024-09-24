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
		AVIAACDecoder * decoder = new AVIAACDecoder(preferred_bits, floating_point);
		if (decoder && SUCCEEDED(decoder->decoder.Open((const unsigned char *)(waveformat + 1), waveformat->extra_size_bytes))) {
			return decoder;
		}
		delete decoder;
	}

	return 0;
}

AVIAACDecoder::AVIAACDecoder(unsigned int bps, bool floating_point)
:  bps(bps), floating_point(floating_point)
{
}

int AVIAACDecoder::OutputFrameSize(size_t *frame_size)
{
	size_t local_frame_size;
	if (FAILED(decoder.OutputBlockSizeSamples(&local_frame_size))) {
		return AVI_FAILURE;
	}

	*frame_size = local_frame_size;
	return AVI_SUCCESS;
}

int AVIAACDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	uint32_t local_sample_rate, local_channels;
	HRESULT hr = decoder.GetOutputProperties(&local_sample_rate, &local_channels);
	if (FAILED(hr)) {
		return AVI_FAILURE;
	}

	*sampleRate = local_sample_rate;
	*channels = local_channels;
	*bitsPerSample = bps;
	*isFloat = floating_point;
	return AVI_SUCCESS;
}

int AVIAACDecoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	if (SUCCEEDED(decoder.Feed(*inputBuffer, *inputBufferBytes))
		&& SUCCEEDED(decoder.Decode(outputBuffer, outputBufferBytes, bps, false, 1.0))) {
			*inputBufferBytes = 0;
			return AVI_SUCCESS;
	}
	return AVI_FAILURE;
}

void AVIAACDecoder::Flush()
{
	decoder.Flush();
}

void AVIAACDecoder::Close()
{
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