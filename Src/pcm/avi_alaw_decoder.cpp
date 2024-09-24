#include "avi_alaw_decoder.h"

static int16_t ALawDecompressTable[256] =
{
     -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736,
     -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
     -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
     -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
     -22016,-20992,-24064,-23040,-17920,-16896,-19968,-18944,
     -30208,-29184,-32256,-31232,-26112,-25088,-28160,-27136,
     -11008,-10496,-12032,-11520,-8960, -8448, -9984, -9472,
     -15104,-14592,-16128,-15616,-13056,-12544,-14080,-13568,
     -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
     -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
     -88,   -72,   -120,  -104,  -24,   -8,    -56,   -40,
     -216,  -200,  -248,  -232,  -152,  -136,  -184,  -168,
     -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
     -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
     -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
     -944,  -912,  -1008, -976,  -816,  -784,  -880,  -848,
      5504,  5248,  6016,  5760,  4480,  4224,  4992,  4736,
      7552,  7296,  8064,  7808,  6528,  6272,  7040,  6784,
      2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
      3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
      22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944,
      30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
      11008, 10496, 12032, 11520, 8960,  8448,  9984,  9472,
      15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
      344,   328,   376,   360,   280,   264,   312,   296,
      472,   456,   504,   488,   408,   392,   440,   424,
      88,    72,   120,   104,    24,     8,    56,    40,
      216,   200,   248,   232,   152,   136,   184,   168,
      1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
      1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
      688,   656,   752,   720,   560,   528,   624,   592,
      944,   912,  1008,   976,   816,   784,   880,   848
};

AVIALawDecoder::AVIALawDecoder(const nsavi::audio_format *waveformat) : waveformat(waveformat)
{
}

int AVIALawDecoder::OutputFrameSize(size_t *frame_size)
{
	*frame_size = waveformat->block_align; // TODO
	return AVI_SUCCESS;
}

int AVIALawDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
{
	if (waveformat)
	{
		*sampleRate = waveformat->sample_rate;
		*channels = waveformat->channels;
		*bitsPerSample = 16;
		*isFloat = false; // TODO
		return AVI_SUCCESS;
	}
	else
	{
		return AVI_FAILURE;
	}
}

int AVIALawDecoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
size_t num_samples = min(*inputBufferBytes, *outputBufferBytes / 2);
	const uint8_t *in = (const uint8_t *)*inputBuffer;

	int16_t *out = (int16_t *)outputBuffer;
	for (size_t i=0;i!=num_samples;i++)
	{
		out[i] = ALawDecompressTable[in[i]];
	}

	*outputBufferBytes = num_samples * 2;
	*inputBufferBytes -= num_samples;
	*inputBuffer = (uint8_t *)inputBuffer + num_samples;
		
	return AVI_SUCCESS;
}

void AVIALawDecoder::Close()
{
	delete this;
}

#define CBCLASS AVIALawDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS