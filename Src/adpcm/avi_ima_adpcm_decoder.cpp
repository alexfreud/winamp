#include "avi_ima_adpcm_decoder.h"
#include "../f263/BitReader.h"

#pragma pack(push, 1)
struct ima_adpcm_format 
{
	nsavi::audio_format format;
	uint16_t samples_per_block;
};
#pragma pack(pop)

IMA_ADPCM_AVIDecoder::IMA_ADPCM_AVIDecoder(const ima_adpcm_format *adpcmformat, const nsavi::STRH *stream_header) : adpcmformat(adpcmformat), stream_header(stream_header)
{
}

int IMA_ADPCM_AVIDecoder::OutputFrameSize(size_t *frame_size)
{
	int channels = adpcmformat->format.channels;
	*frame_size = ((adpcmformat->format.block_align - 7*channels)*2 + 2*channels) * 2;
	return AVI_SUCCESS;
}

int IMA_ADPCM_AVIDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
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

static int index_table[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
}; 

static int step_table[89] = { 
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 
};

int IMA_ADPCM_AVIDecoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
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

		const uint8_t *adpcm_data = (const uint8_t *)(*inputBuffer);

		int predictor =  *(int16_t *)adpcm_data;
		*out16++ = predictor;
		adpcm_data+=2;
		out16_length--;
		int step_index = *adpcm_data;
		if (step_index > 88)
			return AVI_FAILURE;
		adpcm_data+=2;

		BitReader reader;
		reader.data = adpcm_data;
		reader.numBits = (uint32_t)(*inputBufferBytes - 4)*8;

		while (reader.numBits >= 8 && out16_length)
		{
			int diff, step, nibble;

			step = step_table[step_index];
			nibble = reader.getbits(4);
			step_index += index_table[nibble];
			step_index = min(step_index, 88);
			step_index = max(step_index, 0);

			diff = step>>3;
			if(nibble&4)
				diff += step;
			if(nibble&2)
				diff += step>>1;
			if(nibble&1)
				diff += step>>2;

			if (nibble&8)
				predictor -= diff;
			else
				predictor += diff;
			predictor = min(predictor, 32767);
			predictor = max(predictor, -32768);
			*out16++ = predictor;
			out16_length--;
		}
		*inputBuffer = (uint8_t *)(*inputBuffer) + adpcm_stream_length;
		*inputBufferBytes -= adpcm_stream_length;
		*outputBufferBytes = adpcmformat->samples_per_block*2;
		return AVI_SUCCESS;
	}
	else if (adpcmformat->format.channels == 2)
	{
		size_t adpcm_stream_length = *inputBufferBytes;
		if (adpcm_stream_length < adpcmformat->format.block_align) // i'm not even going to consider the possibility of adpcm frames split across avi chunks
			return AVI_FAILURE;

		adpcm_stream_length = adpcmformat->format.block_align; // do one block at a time, in_avi will call us again

		if (adpcm_stream_length < 8)
			return AVI_FAILURE;

		int16_t *out16 = (int16_t *)outputBuffer;
		size_t out16_length = *outputBufferBytes/2;

		const uint8_t *adpcm_data = (const uint8_t *)(*inputBuffer);

		int predictor_left =  *(int16_t *)adpcm_data;
		*out16++ = predictor_left;
		adpcm_data+=2;
		out16_length--;
		int step_index_left = *adpcm_data;
		if (step_index_left > 88)
			return AVI_FAILURE;
		adpcm_data+=2;

		int predictor_right =  *(int16_t *)adpcm_data;
		*out16++ = predictor_right;
		adpcm_data+=2;
		out16_length--;
		int step_index_right =  *adpcm_data;
		if (step_index_right > 88)
			return AVI_FAILURE;
		adpcm_data+=2;

		BitReader reader;
		reader.data = adpcm_data;
		reader.numBits = (uint32_t)(*inputBufferBytes - 8)*8;

		while (reader.numBits >= 8 && out16_length > 15)
		{
			int nibbles_left[8] = {0};
			int nibbles_right[8] = {0};
			for (int i=0;i<8;i++)
				nibbles_left[i] = reader.getbits(4);

			for (int i=0;i<8;i++)
				nibbles_right[i] = reader.getbits(4);

			for (int i=0;i<8;i++)
			{
				int diff, step, nibble;
				step = step_table[step_index_left];
				nibble = nibbles_left[i];
				step_index_left += index_table[nibble];
				step_index_left = min(step_index_left, 88);
				step_index_left = max(step_index_left, 0);

				diff = step>>3;
				if(nibble&4)
					diff += step;
				if(nibble&2)
					diff += step>>1;
				if(nibble&1)
					diff += step>>2;

				if (nibble&8)
					predictor_left -= diff;
				else
					predictor_left += diff;
				predictor_left = min(predictor_left, 32767);
				predictor_left = max(predictor_left, -32768);
				*out16++ = predictor_left;
				out16_length--;

				step = step_table[step_index_right];
				nibble =nibbles_right[i];
				step_index_right += index_table[nibble];
				step_index_right = min(step_index_right, 88);
				step_index_right = max(step_index_right, 0);
				diff = step>>3;
				if(nibble&4)
					diff += step;
				if(nibble&2)
					diff += step>>1;
				if(nibble&1)
					diff += step>>2;
				if (nibble&8)
					predictor_right -= diff;
				else
					predictor_right += diff;
				predictor_right = min(predictor_right, 32767);
				predictor_right = max(predictor_right, -32768);
				*out16++ = predictor_right;
				out16_length--;
			}
		}
		*inputBuffer = (uint8_t *)(*inputBuffer) + adpcm_stream_length;
		*inputBufferBytes -= adpcm_stream_length;
		*outputBufferBytes = adpcmformat->samples_per_block*4;
		return AVI_SUCCESS;
	}

	return AVI_FAILURE;
}

void IMA_ADPCM_AVIDecoder::Close()
{
	delete this;
}

#define CBCLASS IMA_ADPCM_AVIDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS