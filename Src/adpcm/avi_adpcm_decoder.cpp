#include "avi_adpcm_decoder.h"
#include "avi_ima_adpcm_decoder.h"

#pragma pack(push, 1)
typedef int16_t ms_adpcm_coefficients[2];
struct ms_adpcm_format 
{
	nsavi::audio_format format;
	uint16_t samples_per_block;
  uint16_t  number_of_coefficients;
	ms_adpcm_coefficients coefficients[1];
};
#pragma pack(pop)

int AVIDecoder::CreateAudioDecoder(const nsavi::AVIH *avi_header, 
																	 const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, 
																	 unsigned int preferred_bits, unsigned int max_channels, bool floating_point, 
																	 ifc_aviaudiodecoder **decoder)
{
	const nsavi::audio_format *format = (const nsavi::audio_format *)stream_format;

	if (format->format == nsavi::audio_format_ms_adpcm)
	{
		// TODO: verify waveformat sizes
		*decoder = new MS_ADPCM_AVIDecoder( (const ms_adpcm_format *)format, stream_header);
		return CREATEDECODER_SUCCESS;
	}
	else if (format->format == nsavi::audio_format_ima_adpcm)
	{
		// TODO: verify waveformat sizes
		*decoder = new IMA_ADPCM_AVIDecoder((const ima_adpcm_format *)format, stream_header);
		return CREATEDECODER_SUCCESS;
	}

	return CREATEDECODER_NOT_MINE;

}

#define CBCLASS AVIDecoder
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
END_DISPATCH;
#undef CBCLASS

int ms_adpcm_adaptationtable[] = { 230, 230, 230, 230, 307, 409, 512, 614, 768, 614, 512, 409, 307, 230, 230, 230 };
//int ms_adpcm_adaptcoeff1[] = { 256, 512, 0, 192, 240, 460, 392 } ;
//int ms_adpcm_adaptcoeff2[] = { 0, -256, 0, 64, 0, -208, -232 } ;


MS_ADPCM_AVIDecoder::MS_ADPCM_AVIDecoder(const ms_adpcm_format *adpcmformat, const nsavi::STRH *stream_header) : adpcmformat(adpcmformat), stream_header(stream_header)
{
}

int MS_ADPCM_AVIDecoder::OutputFrameSize(size_t *frame_size)
{
	int channels = adpcmformat->format.channels;
	*frame_size = ((adpcmformat->format.block_align - 7*channels)*2 + 2*channels) * 2;
	return AVI_SUCCESS;
}

int MS_ADPCM_AVIDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	if (adpcmformat)
	{
		*sampleRate = adpcmformat->format.sample_rate;
		*channels = adpcmformat->format.channels;
		*bitsPerSample = 16;
		*isFloat = false; 
		return AVI_SUCCESS;
	}
	else
	{
		return AVI_FAILURE;
	}
}

int MS_ADPCM_AVIDecoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
  const ms_adpcm_coefficients *ms_adpcm_adaptcoeff = adpcmformat->coefficients;
	// TODO: use default coef values if they aren't present
	if (adpcmformat->format.channels == 1)
	{
		size_t adpcm_stream_length = *inputBufferBytes;
		if (adpcm_stream_length < adpcmformat->format.block_align) // i'm not even going to consider the possibility of adpcm frames split across avi chunks
			return AVI_FAILURE;

		adpcm_stream_length = adpcmformat->format.block_align; // do one block at a time, in_avi will call us again
		
		if (adpcm_stream_length < 7)
			return AVI_FAILURE;

		int16_t *out16 = (int16_t *)outputBuffer;
		size_t out16_length = *outputBufferBytes/2;

		const uint8_t *adpcm8 = (const uint8_t *)(*inputBuffer);
		
		uint8_t block_predictor = *adpcm8++;
		if (block_predictor > adpcmformat->number_of_coefficients)
			return AVI_FAILURE;

		int32_t coef1 = ms_adpcm_adaptcoeff[block_predictor][0];
		int32_t coef2 = ms_adpcm_adaptcoeff[block_predictor][1];

		const uint16_t *adpcm16 = (const uint16_t *)adpcm8;

		int16_t delta = *adpcm16++;

		int16_t sample1 = out16[1] = *adpcm16++;
		int16_t sample2 = out16[0] = *adpcm16++;
		int i=2;
		adpcm_stream_length-=7;
		adpcm8 = (const uint8_t *)adpcm16;
		while (adpcm_stream_length-- && out16_length)
		{
			int32_t predictor = ((int32_t)sample1 * coef1 + (int32_t)sample2 * coef2)>>8;
			uint32_t nibble = *adpcm8 >> 4;
			int32_t signed_nibble = ((int32_t)nibble << 28) >> 28;
			predictor += signed_nibble*delta;
			predictor = max(predictor, -32768);
			predictor = min(predictor, 32767);
			sample2=sample1;
			sample1=out16[i++]=predictor;
			out16_length--;
			delta = (ms_adpcm_adaptationtable[nibble]*delta)>>8;
			delta = max(delta, 16);

			predictor = ((int32_t)sample1 * coef1 + (int32_t)sample2 * coef2)>>8;
			nibble = *adpcm8++ & 0x0F;
			signed_nibble = ((int32_t)nibble << 28) >> 28;
			predictor += signed_nibble*delta;
			predictor = max(predictor, -32768);
			predictor = min(predictor, 32767);
			sample2=sample1;
			sample1=out16[i++]=predictor;
			out16_length--;
			delta = (ms_adpcm_adaptationtable[nibble]*delta)>>8;
			delta = max(delta, 16);
		}
		*inputBufferBytes -= adpcmformat->format.block_align;
		*inputBuffer = (void *)adpcm8;
		*outputBufferBytes = i*2;
		return AVI_SUCCESS;
	}
	else if (adpcmformat->format.channels == 2)
	{
			size_t adpcm_stream_length = *inputBufferBytes;
		if (adpcm_stream_length < adpcmformat->format.block_align) // i'm not even going to consider the possibility of adpcm frames split across avi chunks
			return AVI_FAILURE;

		adpcm_stream_length = adpcmformat->format.block_align; // do one block at a time, in_avi will call us again
		
		if (adpcm_stream_length < 14)
			return AVI_FAILURE;

		int16_t *out16 = (int16_t *)outputBuffer;
		size_t out16_length = *outputBufferBytes/2;

		const uint8_t *adpcm8 = (const uint8_t *)(*inputBuffer);
		
		uint8_t block_predictor_left = *adpcm8++;
		if (block_predictor_left > adpcmformat->number_of_coefficients)
			return AVI_FAILURE;

		uint8_t block_predictor_right = *adpcm8++;
		if (block_predictor_right > adpcmformat->number_of_coefficients)
			return AVI_FAILURE;

		int32_t coef1_left = ms_adpcm_adaptcoeff[block_predictor_left][0];
		int32_t coef2_left = ms_adpcm_adaptcoeff[block_predictor_left][1];
		int32_t coef1_right = ms_adpcm_adaptcoeff[block_predictor_right][0];
		int32_t coef2_right = ms_adpcm_adaptcoeff[block_predictor_right][1];

		const uint16_t *adpcm16 = (const uint16_t *)adpcm8;

		int16_t delta_left = *adpcm16++;
		int16_t delta_right = *adpcm16++;

		int16_t sample1_left = out16[2] = *adpcm16++;
		int16_t sample1_right = out16[3] = *adpcm16++;
		int16_t sample2_left = out16[0] = *adpcm16++;
		int16_t sample2_right = out16[1] = *adpcm16++;
		int i=4;
		adpcm_stream_length-=14;
		adpcm8 = (const uint8_t *)adpcm16;
		while (adpcm_stream_length-- && out16_length)
		{
			int32_t predictor = ((int32_t)sample1_left * coef1_left + (int32_t)sample2_left * coef2_left)>>8;
			uint32_t nibble = *adpcm8 >> 4;
			int32_t signed_nibble = ((int32_t)nibble << 28) >> 28;
			predictor += signed_nibble*delta_left;
			predictor = max(predictor, -32768);
			predictor = min(predictor, 32767);
			sample2_left=sample1_left;
			sample1_left=out16[i++]=predictor;
			out16_length--;
			delta_left = (ms_adpcm_adaptationtable[nibble]*delta_left)>>8;
			delta_left = max(delta_left, 16);

			predictor = ((int32_t)sample1_right * coef1_right + (int32_t)sample2_right * coef2_right)>>8;
			nibble = *adpcm8++ & 0x0F;
			signed_nibble = ((int32_t)nibble << 28) >> 28;
			predictor += signed_nibble*delta_right;
			predictor = max(predictor, -32768);
			predictor = min(predictor, 32767);
			sample2_right=sample1_right;
			sample1_right=out16[i++]=predictor;
			out16_length--;
			delta_right = (ms_adpcm_adaptationtable[nibble]*delta_right)>>8;
			delta_right = max(delta_right, 16);
		}
		*inputBufferBytes -= adpcmformat->format.block_align;
		*inputBuffer = (void *)adpcm8;
		*outputBufferBytes = i*2;
		return AVI_SUCCESS;
	}
	
	return AVI_FAILURE;
	
}

void MS_ADPCM_AVIDecoder::Close()
{
	delete this;
}

#define CBCLASS MS_ADPCM_AVIDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS