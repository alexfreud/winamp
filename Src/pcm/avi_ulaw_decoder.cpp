#include "avi_ulaw_decoder.h"

static int16_t MuLawDecompressTable[256] =
{
     -32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,
     -23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,
     -15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,
     -11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,
      -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
      -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
      -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
      -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
      -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
      -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
       -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
       -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
       -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
       -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
       -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
        -56,   -48,   -40,   -32,   -24,   -16,    -8,     0,
      32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
      23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
      15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
      11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
       7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
       5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
       3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
       2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
       1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
       1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
        876,   844,   812,   780,   748,   716,   684,   652,
        620,   588,   556,   524,   492,   460,   428,   396,
        372,   356,   340,   324,   308,   292,   276,   260,
        244,   228,   212,   196,   180,   164,   148,   132,
        120,   112,   104,    96,    88,    80,    72,    64,
         56,    48,    40,    32,    24,    16,     8,     0
};

AVIULawDecoder::AVIULawDecoder(const nsavi::audio_format *waveformat) : waveformat(waveformat)
{
}

int AVIULawDecoder::OutputFrameSize(size_t *frame_size)
{
	*frame_size = waveformat->block_align; // TODO
	return AVI_SUCCESS;
}

int AVIULawDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample, bool *isFloat)
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

int AVIULawDecoder::DecodeChunk(uint16_t type, void **inputBuffer, size_t *inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
size_t num_samples = min(*inputBufferBytes, *outputBufferBytes / 2);
	const uint8_t *in = (const uint8_t *)*inputBuffer;

	int16_t *out = (int16_t *)outputBuffer;
	for (size_t i=0;i!=num_samples;i++)
	{
		out[i] = MuLawDecompressTable[in[i]];
	}

	*outputBufferBytes = num_samples * 2;
	*inputBufferBytes -= num_samples;
	*inputBuffer = (uint8_t *)inputBuffer + num_samples;
		
	return AVI_SUCCESS;
}

void AVIULawDecoder::Close()
{
	delete this;
}

#define CBCLASS AVIULawDecoder
START_DISPATCH;
CB(OUTPUT_FRAME_SIZE, OutputFrameSize)
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS