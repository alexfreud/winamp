#include "MP3Coder.h"
#include <malloc.h>

static const float const_1_div_128_ = 32768.0 / 128.0;  /* 8 bit multiplier */
static const double const_1_div_2147483648_ = 32768.0 / 2147483648.0; /* 32 bit multiplier */

static void Int8_To_Float32(float *dest, void *sourceBuffer, signed int sourceStride, unsigned int count)
{
    signed char *src = (signed char*)sourceBuffer;

    while( count-- )
    {
        float samp = *src * const_1_div_128_;
        *dest = samp;

        src += sourceStride;
        dest ++;
    }
}

static void Int24_To_Float32(float *dest, void *sourceBuffer, signed int sourceStride, unsigned int count)
{
	unsigned char *src = (unsigned char*)sourceBuffer;

	while ( count-- )
	{
		signed long temp = (((long)src[0]) << 8);
		temp = temp | (((long)src[1]) << 16);
		temp = temp | (((long)src[2]) << 24);
		*dest = (float) ((double)temp * const_1_div_2147483648_);
		src += sourceStride * 3;
		dest ++;
	}
}

int AudioCoderMP3_24::Encode(int framepos, void *in, int in_avail, int *in_used, void *out, int out_avail)
{
	if (m_err) return -1;
	if (!hbeStream)
	{
		if (beInitStream(&beConfig, &ibuf_size_spls, &obuf_size, (PHBE_STREAM) &hbeStream) != BE_ERR_SUCCESSFUL)
		{
			m_err++;
			return -1;
		}
		ibuf_size = ibuf_size_spls * bytesPerSample;

		if (is_downmix) ibuf_size *= 2;

		bs = (char*)malloc(ibuf_size);
		bs_size = 0;
	}

	*in_used = 0;

	int needbytes = ibuf_size - bs_size;
	if (needbytes > 0 && in_avail > 0)
	{
		if (needbytes > in_avail)
			needbytes = in_avail;
		memcpy(bs + bs_size, in, needbytes);
		bs_size += needbytes;
		*in_used = needbytes;
	}

	if (!done)
	{
		if (bs_size < (int)ibuf_size) return 0;
	}

	if (out_avail < (int)obuf_size) return 0;

	int feedBytes = min(bs_size, (int)ibuf_size);
	int feedSamples = feedBytes / bytesPerSample;
	bs_size -= feedBytes;
	/*
		if (is_downmix)
		{
			int x;
			int l = feedBytes / 4;
			short *b = (short*)bs;
			for (x = 0; x < l; x ++)
			{
				b[x] = b[x * 2] / 2 + b[x * 2 + 1] / 2;
			}
		}
		*/
	DWORD dwo = 0;

	if (feedSamples)
	{
		if (mono_input)
		{
			float *float_l = (float *)alloca(sizeof(float) * feedSamples);
			switch(bytesPerSample)
			{
			case 8:
				Int8_To_Float32(float_l, bs, 1, feedSamples);
				break;
			case 24:
				Int24_To_Float32(float_l, bs, 1, feedSamples);
				break;
			}
			

			if (beEncodeChunkFloatS16NI(hbeStream, feedSamples, float_l, float_l, (unsigned char *)out, &dwo) != BE_ERR_SUCCESSFUL)
			{
				m_err++;
				return -1;
			}
		}
		else
		{
			float *float_l = (float *)alloca(sizeof(float) * feedSamples / 2);
			float *float_r = (float *)alloca(sizeof(float) * feedSamples / 2);
			switch(bytesPerSample)
			{
				case 1: // 8 bit
					Int8_To_Float32(float_l, bs, 2, feedSamples / 2);
					Int8_To_Float32(float_r, bs + 1, 2, feedSamples / 2);
				break;
				case 3: // 24 bit
					Int24_To_Float32(float_l, bs, 2, feedSamples / 2);
					Int24_To_Float32(float_r, bs + 3, 2, feedSamples / 2);
				break;
			}
			
			if (beEncodeChunkFloatS16NI(hbeStream, feedSamples / 2, float_l, float_r, (unsigned char *)out, &dwo) != BE_ERR_SUCCESSFUL)
			{
				m_err++;
				return -1;
			}
		}
	}

	if (!dwo && done == 1)
	{
		if (beDeinitStream(hbeStream, (unsigned char *)out, &dwo) != BE_ERR_SUCCESSFUL)
		{
			m_err++;
			return -1;
		}
		done++;
	}

	return dwo;
}