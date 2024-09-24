#include "mp4_ulaw_decoder.h"

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

MP4ulawDecoder::MP4ulawDecoder()
{
}
int MP4ulawDecoder::Open()
{
	return MP4_SUCCESS;
}
void MP4ulawDecoder::Close()
{
}
int MP4ulawDecoder::GetOutputProperties(unsigned int *sampleRate, unsigned int *channels, unsigned int *bitsPerSample)
{
	// TODO
	*channels = 1;
	*bitsPerSample = 16;
	return MP4_SUCCESS;
}

int MP4ulawDecoder::DecodeSample(void *inputBuffer, size_t inputBufferBytes, void *outputBuffer, size_t *outputBufferBytes)
{
	const uint8_t *in = (const uint8_t *)inputBuffer;
	*outputBufferBytes=0;
	int16_t *out = (int16_t *)outputBuffer;
	while (inputBufferBytes--)
	{
		*outputBufferBytes = *outputBufferBytes + 2;
		*out++ = MuLawDecompressTable[*in++];
	}
	return MP4_SUCCESS;
}

int MP4ulawDecoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "ulaw");
}

int MP4ulawDecoder::RequireChunks()
{
	return 1;
}

#define CBCLASS MP4ulawDecoder
START_DISPATCH;
CB(MPEG4_AUDIO_OPEN, Open)
//CB(MPEG4_AUDIO_OPEN_EX, OpenEx)
//CB(MPEG4_AUDIO_ASC, AudioSpecificConfiguration)
//CB(MPEG4_AUDIO_BITRATE, GetCurrentBitrate)
//CB(MPEG4_AUDIO_FRAMESIZE, OutputFrameSize)
CB(MPEG4_AUDIO_OUTPUTINFO, GetOutputProperties)
//CB(MPEG4_AUDIO_OUTPUTINFO_EX, GetOutputPropertiesEx)
CB(MPEG4_AUDIO_DECODE, DecodeSample)
//VCB(MPEG4_AUDIO_FLUSH, Flush)
VCB(MPEG4_AUDIO_CLOSE, Close)
CB(MPEG4_AUDIO_HANDLES_CODEC, CanHandleCodec)
//CB(MPEG4_AUDIO_HANDLES_TYPE, CanHandleType)
//CB(MPEG4_AUDIO_HANDLES_MPEG4_TYPE, CanHandleMPEG4Type)
//CB(MPEG4_AUDIO_SET_GAIN, SetGain)
CB(MPEG4_AUDIO_REQUIRE_CHUNKS, RequireChunks)
END_DISPATCH;
#undef CBCLASS