#include "flv_adpcm_decoder.h"
#include "../f263/BitReader.h"

int FLVDecoderCreator::CreateAudioDecoder(int stereo, int bits, int sample_rate, int format_type, ifc_flvaudiodecoder **decoder)
{
	if (format_type == FLV::AUDIO_FORMAT_ADPCM)
	{
		*decoder = new FLVADPCM(stereo?2:1);
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

int FLVDecoderCreator::HandlesAudio(int format_type)
{
	if (format_type == FLV::AUDIO_FORMAT_ADPCM)
	{
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS FLVDecoderCreator
START_DISPATCH;
CB(CREATE_AUDIO_DECODER, CreateAudioDecoder)
CB(HANDLES_AUDIO, HandlesAudio)
END_DISPATCH;
#undef CBCLASS

/* --- */
FLVADPCM::FLVADPCM(unsigned int channels) : channels(channels)
{

}

int FLVADPCM::GetOutputFormat(unsigned int *sample_rate, unsigned int *channels, unsigned int *bits)
{
	*channels = this->channels;
	*bits = 16;
	return FLV_AUDIO_SUCCESS;
}

// padded to zero where table size is less then 16
static const int swf_index_tables[4][16] = {
	/*2*/ { -1, 2 },
	/*3*/ { -1, -1, 2, 4 },
	/*4*/ { -1, -1, -1, -1, 2, 4, 6, 8 },
	/*5*/ { -1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 4, 6, 8, 10, 13, 16 }
};

static const int step_table[89] = {
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

int FLVADPCM::DecodeSample(const void *input_buffer, size_t input_buffer_bytes, void *_samples, size_t *samples_size_bytes, double *bitrate)
{
	int predictors[2] = {0, 0};
	int16_t step_indices[2] = {0, 0};

	int16_t *samples = (int16_t *)_samples;
	const size_t max_samples = *samples_size_bytes / sizeof(int16_t);

	BitReader bit_reader;
	bit_reader.data = (uint8_t *)input_buffer;
	bit_reader.numBits = (uint32_t)input_buffer_bytes * 8;

	uint32_t bits = bit_reader.getbits(2);
	const int *table = swf_index_tables[bits]; 
	int k0 = 1 << bits++;
	int signmask = 1 << bits++;

	size_t sample_num=0;
	while (bit_reader.size() >=  22*channels) 
	{
		for (size_t i = 0; i != channels; i++) 
		{
			samples[sample_num++] = predictors[i] = ((int32_t)bit_reader.getbits(16) << 16) >> 16;
			step_indices[i] = bit_reader.getbits(6);
		}

		while (bit_reader.size() > bits*channels && sample_num < max_samples)
		{
			for (size_t i = 0; i != channels; i++) 
			{
				int delta = bit_reader.getbits(bits);
				int step = step_table[step_indices[i]];
				long vpdiff = 0;
				int k = k0;

				do {
					if (delta & k)
						vpdiff += step;
					step >>= 1;
					k >>= 1;
				} while(k);
				vpdiff += step;

				if (delta & signmask)
					predictors[i] -= vpdiff;
				else
					predictors[i] += vpdiff;

				step_indices[i] += table[delta & (~signmask)];

				if (step_indices[i] < 0) step_indices[i] = 0;
				if (step_indices[i] > 88) step_indices[i] = 88;

				if (predictors[i] > 32767) predictors[i] = 32767;
				if (predictors[i] < -32768) predictors[i] = -32768;

				samples[sample_num++] = predictors[i];
			}
		}
	}

	*samples_size_bytes = sample_num * sizeof(int16_t);

	return FLV_AUDIO_SUCCESS;
}


void FLVADPCM::Close()
{
	delete this;
}

#define CBCLASS FLVADPCM
START_DISPATCH;
CB(FLV_AUDIO_GETOUTPUTFORMAT, GetOutputFormat)
CB(FLV_AUDIO_DECODE, DecodeSample)
VCB(FLV_AUDIO_CLOSE, Close)
END_DISPATCH;
#undef CBCLASS