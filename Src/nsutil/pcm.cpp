#include "pcm.h"
#include <math.h>
#include <ipps.h>
#include <intrin.h>
#include <mmintrin.h>

#define PA_CLIP_( val, min, max )\
	{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

#if defined(_M_IX86)
static __inline long float_to_long(double t)
{
	long r;
	__asm fld t
		__asm fistp r
		return r;
}
#else
#define float_to_long(x) ((long)( x ))
#endif

inline static void clip(double &x, double a, double b)
{
	double x1 = fabs (x - a);
	double x2 = fabs (x - b);
	x = x1 + (a + b);
	x -= x2;
	x *= 0.5;
}

static void Float32_To_Int32_Clip(void *destinationBuffer, const float *src, size_t count, double gain)
{
	int32_t *dest = (int32_t *)destinationBuffer;

	gain*=65536.*32768.;
	while ( count-- )
	{
		/* convert to 32 bit and drop the low 8 bits */
		double scaled = *src++ * gain;
		clip( scaled, -2147483648., 2147483647.);
		signed long temp = (signed long) scaled;
		*dest++ = temp;
	}
}

static void Float32_To_Int24_Clip(void *destinationBuffer, const float *src, size_t count, double gain)
{
	unsigned char *dest = (unsigned char*)destinationBuffer;
	gain*=65536.*32768.;
	while ( count-- )
	{
		/* convert to 32 bit and drop the low 8 bits */
		double scaled = *src * gain;
		clip( scaled, -2147483648., 2147483647.);
		signed long temp = (signed long) scaled;

		dest[0] = (unsigned char)(temp >> 8);
		dest[1] = (unsigned char)(temp >> 16);
		dest[2] = (unsigned char)(temp >> 24);

		src++;
		dest += 3;
	}
}

static void Float32_To_Int16_Clip(void *destinationBuffer, const float *src, size_t count, double gain)
{
	int16_t *dest = (signed short*)destinationBuffer;

	gain*=32768.0;
	while ( count-- )
	{
		long samp = float_to_long((*src) * gain/* - 0.5*/);

		PA_CLIP_( samp, -0x8000, 0x7FFF );
		*dest = (int16_t) samp;

		src ++;
		dest ++;
	}
}

static void Float32_To_UInt8_Clip(void *destinationBuffer, const float *src, size_t count, double gain)
{
	uint8_t *dest = (uint8_t *)destinationBuffer;

	gain*=128.0;
	while ( count-- )
	{
		long samp = float_to_long((*src) * gain/* - 0.5*/) + 128;

		PA_CLIP_( samp, 0, 255);
		*dest = (uint8_t) samp;

		src ++;
		dest ++;
	}
}

int nsutil_pcm_FloatToInt_Interleaved_Gain(void *pcm, const float *input, int bps, size_t num_samples, float gain)
{
	switch(bps)
	{
	case 8:
		Float32_To_UInt8_Clip(pcm, input, num_samples, gain);
		return 0;
	case 16:
		Float32_To_Int16_Clip(pcm, input, num_samples, gain);
		return 0;
	case 24:
		Float32_To_Int24_Clip(pcm, input, num_samples, gain);
		return 0;
	case 32:
		Float32_To_Int32_Clip(pcm, input, num_samples, gain);
		return 0;
	}
	return 0;
}

int nsutil_pcm_FloatToInt_Interleaved(void *pcm, const float *input, int bps, size_t num_samples)
{
	switch(bps)
	{
	case 8:
		Float32_To_UInt8_Clip(pcm, input, num_samples, 1.0f);
		return 0;
	case 16:
		Float32_To_Int16_Clip(pcm, input, num_samples, 1.0f);
		return 0;
	case 24:
		Float32_To_Int24_Clip(pcm, input, num_samples, 1.0f);
		return 0;
	case 32:
		Float32_To_Int32_Clip(pcm, input, num_samples, 1.0f);
		return 0;
	}
	return 0;
}

int nsutil_pcm_IntToFloat_Interleaved(float *output, const void *pcm, int bps, size_t num_samples)
{
	switch (bps)
	{
	case 8:
		{
			unsigned __int8 *samples8 = (unsigned __int8 *)pcm;
			for (size_t x = 0; x != num_samples; x ++)
			{
				output[x] = (float)(samples8[x]-128) * 0.00390625f /* 1/256 */;
			}
		}
		break;
	case 16:
		{
			short *samples16 = (short *)pcm;
			for (size_t x = 0; x != num_samples; x ++)
			{
				output[x] = (float)samples16[x] * 0.000030517578125f /* 1/ 32768 */;
			}
		}
		break;
	case 24:
		{
			unsigned __int8 *samples8 = (unsigned __int8 *)pcm;
			for (size_t x = 0; x != num_samples; x ++)
			{
				long temp = (((long)samples8[0]) << 8);
				temp = temp | (((long)samples8[1]) << 16);
				temp = temp | (((long)samples8[2]) << 24);
				output[x] = (float)temp * 4.656612873077393e-10f /* 1/2147483648 */;
				samples8+=3;
			}
		}
		break;
	case 32:
		{
			int32_t *samples32 = (int32_t *)pcm;
			for (size_t x = 0; x != num_samples; x ++)
			{
				output[x] = (float)samples32[x] * 4.656612873077393e-10f /* 1/2147483648 */;
			}
		}
		break;
	}
	return 0;
}

int nsutil_pcm_IntToFloat_Interleaved_Gain(float *output, const void *pcm, int bps, size_t num_samples, float gain)
{
	switch (bps)
	{
	case 8:
		{
			gain /= 256.0f;
			uint8_t *samples8 = (uint8_t *)pcm;
			for (size_t x = 0; x != num_samples; x ++)
			{
				output[x] = (float)(samples8[x]-128) * gain;
			}
		}
		break;
	case 16:
		{
			gain /= 32768.0f;
			int16_t *samples16 = (int16_t *)pcm;
			for (size_t x = 0; x != num_samples; x ++)
			{
				output[x] = (float)samples16[x] * gain;
			}
		}
		break;
	case 24:
		{
			gain /= 2147483648.0f;
			uint8_t *samples8 = (uint8_t *)pcm;
			for (size_t x = 0; x != num_samples; x ++)
			{
				long temp = (((long)samples8[0]) << 8);
				temp = temp | (((long)samples8[1]) << 16);
				temp = temp | (((long)samples8[2]) << 24);
				output[x] = (float)temp * gain;
				samples8+=3;
			}
		}
		break;
	case 32:
		{
			gain /= 2147483648.0f;
			int32_t *samples32 = (int32_t *)pcm;
			for (size_t x = 0; x != num_samples; x ++)
			{
				output[x] = (float)samples32[x] * gain;
			}
		}
		break;
	}
	return 0;
}

int nsutil_pcm_S8ToS16_Interleaved(int16_t *output, const int8_t *pcm, size_t num_samples)
{
	//__m64 mmx_zero = _mm_setzero_si64();
	__m128i sse_zero = _mm_setzero_si128();
	//while (num_samples>7)
	while (num_samples > 15)
	{
		//__m64 mmx_8 = *(const __m64 *)pcm;
		__m128i sse_8 = *(const __m128i*)pcm;
		//pcm+=8;
		pcm += 16;
		//__m64 mmx_16 = _mm_unpacklo_pi8(mmx_zero, mmx_8);
		__m128i sse_16 = _mm_unpacklo_epi8(sse_zero, sse_8);
		//*(__m64 *)output = mmx_16;
		*(__m128i*)output = sse_16;
		//output+=4;
		output += 8;
		//mmx_16 = _mm_unpackhi_pi8(mmx_zero, mmx_8);
		sse_16 = _mm_unpackhi_epi8(sse_zero, sse_8);
		//*(__m64 *)output = mmx_16;
		*(__m128i *)output = sse_16;
		//output+=4;
		output += 8;
		//num_samples-=8;
		num_samples-=16;
	}
	while(num_samples--)
	{
		*output++ = (*pcm++) << 8;
	}
	//_mm_empty();
	return 0;
}

int nsutil_pcm_U8ToS16_Interleaved(int16_t *output, const uint8_t *pcm, size_t num_samples)
{
	//__m64 mmx_zero = _mm_setzero_si64();
	__m128i sse_zero = _mm_setzero_si128();
	//__m64 mmx_128 = _mm_set1_pi8(-128);
	__m128i sse_128 = _mm_set1_epi8(-128);
	//while (num_samples>7)
	while (num_samples > 15)
	{
		//__m64 mmx_8 = *(const __m64*)pcm;
		__m128i sse_8 = *(const __m128i *)pcm;
		
		//mmx_8 = _mm_add_pi8(mmx_8, mmx_128);
		sse_8 = _mm_add_epi8(sse_8, sse_128);
		
		//pcm+=8;
		pcm += 16;
		//__m64 mmx_16 = _mm_unpacklo_pi8(mmx_zero, mmx_8);
		__m128i sse_16 = _mm_unpacklo_epi8(sse_zero, sse_8);
		//*(__m64 *)output = mmx_16;
		*(__m128i*)output = sse_16;
		//output+=4;
		output += 8;
		//mmx_16 = _mm_unpackhi_pi8(mmx_zero, mmx_8);
		sse_16 = _mm_unpackhi_epi8(sse_zero, sse_8);
		//*(__m64 *)output = mmx_16;
		*(__m128i*)output = sse_16;
		//output+=4;
		output += 8;
		//num_samples-=8;
		num_samples -= 16;
	}
	while(num_samples--)
	{
		*output++ = (*pcm++ - 128) << 8;
	}
	//_mm_empty();
	return 0;
}