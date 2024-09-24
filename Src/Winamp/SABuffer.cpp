#include "SABuffer.h"
#include "fft.h"
#include "../nsutil/window.h"
#include <windows.h>

static const float const_1_div_128_ = 1.0f / 128.0f;  /* 8 bit multiplier */
static const float const_1_div_32768_ = 1.0f / 32768.f; /* 16 bit multiplier */
static const double const_1_div_2147483648_ = 1.0 / 2147483648.0; /* 32 bit multiplier */

static void Int16_To_Float32(float *dest, void *sourceBuffer, signed int sourceStride, unsigned int count)
{
	signed short *src = (signed short*)sourceBuffer;

	while (count--)
	{
		float samp = *src * const_1_div_32768_; /* FIXME: i'm concerned about this being asymetrical with float->int16 -rb */
		*dest = samp;

		src += sourceStride;
		dest++;
	}
}

static void Int24_To_Float32(float *dest, void *sourceBuffer, signed int sourceStride, unsigned int count)
{
	unsigned char *src = (unsigned char*)sourceBuffer;

	while (count--)
	{
		signed long temp = (((long)src[0]) << 8);
		temp = temp | (((long)src[1]) << 16);
		temp = temp | (((long)src[2]) << 24);

		*dest = (float)((double)temp * const_1_div_2147483648_);

		src += sourceStride * 3;
		dest++;
	}
}

static void Int32_To_Float32(float *dest, void *sourceBuffer, signed int sourceStride, unsigned int count)
{
	int32_t *src = (int32_t *)sourceBuffer;

	while (count--)
	{
		*dest = (float)((double)*src * const_1_div_2147483648_);

		src += sourceStride;
		dest++;
	}
}

static void UInt8_To_Float32(float *dest, void *sourceBuffer, signed int sourceStride, unsigned int count)
{
	unsigned char *src = (unsigned char*)sourceBuffer;

	while (count--)
	{
		float samp = (*src - 128) * const_1_div_128_;
		*dest = samp;

		src += sourceStride;
		dest++;
	}
}

SABuffer::SABuffer()
{
	memset(&buffer, 0, sizeof(buffer));
	used=0;
	init=false;
}

void SABuffer::CopyHalf()
{
	memcpy(buffer[0], buffer[0]+SABUFFER_WINDOW_INCREMENT, (512-SABUFFER_WINDOW_INCREMENT)*sizeof(float));
	memcpy(buffer[1], buffer[1]+SABUFFER_WINDOW_INCREMENT, (512-SABUFFER_WINDOW_INCREMENT)*sizeof(float));
	used-=SABUFFER_WINDOW_INCREMENT;
}

void SABuffer::Clear()
{
	used=0;
}

void SABuffer::WindowToFFTBuffer(float *wavetrum)
{
	for (int i=0;i<512;i++)
	{
		wavetrum[i] = (buffer[0][i] + buffer[1][i]);
		//*wavetrum++=0;
	}
	nsutil_window_Multiply_F32_IP(wavetrum, window, 512);
}

unsigned int SABuffer::AddToBuffer(char *samples, int numChannels, int bps, int ts, unsigned int numSamples)
{
	if (!init) {
		nsutil_window_FillHann_F32_IP(window, 512); // TODO this could be hardcoded
		init=true;
	}

	unsigned int toCopy = min((unsigned int)(512-used), numSamples);
	switch (bps)
	{
	case 8:
		UInt8_To_Float32(buffer[0]+used, samples, numChannels, toCopy);
		if (numChannels > 1)
			UInt8_To_Float32(buffer[1]+used, samples+1, numChannels, toCopy);
		else
			UInt8_To_Float32(buffer[1]+used, samples, numChannels, toCopy);
		break;
	case 16:
		Int16_To_Float32(buffer[0]+used, samples, numChannels, toCopy);
		if (numChannels > 1)
			Int16_To_Float32(buffer[1]+used, samples+2, numChannels, toCopy);
		else
			Int16_To_Float32(buffer[1]+used, samples, numChannels, toCopy);
		break;
	case 24:
		Int24_To_Float32(buffer[0]+used, samples, numChannels, toCopy);
		if (numChannels > 1)
			Int24_To_Float32(buffer[1]+used, samples+3, numChannels, toCopy);
		else
			Int24_To_Float32(buffer[1]+used, samples, numChannels, toCopy);
		break;
	case 32:
		Int32_To_Float32(buffer[0]+used, samples, numChannels, toCopy);
		if (numChannels > 1)
			Int32_To_Float32(buffer[1]+used, samples+4, numChannels, toCopy);
		else
			Int32_To_Float32(buffer[1]+used, samples, numChannels, toCopy);
		break;
	}
	used+=toCopy;
	return toCopy;
}

