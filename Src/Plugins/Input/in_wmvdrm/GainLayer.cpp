#include "main.h"
#include "GainLayer.h"
#include <malloc.h>
#include <math.h>
#include <locale.h>
#include "api.h"

static void FillFloat(float *floatBuf, void *samples, size_t bps, size_t numSamples, size_t numChannels, float preamp)
{
	switch (bps)
	{
	case 8:
		{
			unsigned __int8 *samples8 = (unsigned __int8 *)samples;
			size_t totalSamples = numSamples * numChannels;
			for (size_t x = 0; x != totalSamples; x ++)
			{
				floatBuf[x] = (float)(samples8[x] - 128) * preamp;
			}
		}
		break;
	case 16:
		{
			short *samples16 = (short *)samples;
			size_t totalSamples = numSamples * numChannels;
			for (size_t x = 0; x != totalSamples; x ++)
			{
				floatBuf[x] = (float)samples16[x] * preamp;
			}
		}
		break;
	case 24:
		{
			unsigned __int8 *samples8 = (unsigned __int8 *)samples;
			size_t totalSamples = numSamples * numChannels;
			for (size_t x = 0; x != totalSamples; x ++)
			{
				long temp = (((long)samples8[0]) << 8);
				temp = temp | (((long)samples8[1]) << 16);
				temp = temp | (((long)samples8[2]) << 24);
				floatBuf[x] = (float)temp * preamp;
				samples8 += 3;
			}
		}
		break;
	}
}

inline static float fastclip(float x, const float a, const float b)
{
	float x1 = (float)fabs (x - a);
	float x2 = (float)fabs (x - b);
	x = x1 + (a + b);
	x -= x2;
	x *= 0.5f;
	return (x);
}

#define PA_CLIP_( val, min, max )\
	{ val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }

void Float32_To_Int16_Clip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count)
{
	float *src = (float*)sourceBuffer;
	signed short *dest = (signed short*)destinationBuffer;

	while ( count-- )
	{
		long samp = lrint(*src);

		PA_CLIP_( samp, -0x8000, 0x7FFF );
		*dest = (signed short) samp;

		src += sourceStride;
		dest += destinationStride;
	}
}
inline static void clip(double &x, double a, double b)
{
	double x1 = fabs (x - a);
	double x2 = fabs (x - b);
	x = x1 + (a + b);
	x -= x2;
	x *= 0.5;
}

void Float32_To_Int24_Clip(
    void *destinationBuffer, signed int destinationStride,
    void *sourceBuffer, signed int sourceStride,
    unsigned int count)
{
	float *src = (float*)sourceBuffer;
	unsigned char *dest = (unsigned char*)destinationBuffer;

	while ( count-- )
	{
		/* convert to 32 bit and drop the low 8 bits */
		double scaled = *src;
		clip( scaled, -2147483648., 2147483647. );
		signed long temp = (signed long) scaled;

		dest[0] = (unsigned char)(temp >> 8);
		dest[1] = (unsigned char)(temp >> 16);
		dest[2] = (unsigned char)(temp >> 24);
		src += sourceStride;
		dest += destinationStride * 3;
	}
}

static void FillSamples(void *samples, float *floatBuf, size_t bps, size_t numSamples, size_t numChannels)
{
	switch (bps)
	{
	case 16:
		Float32_To_Int16_Clip(samples, 1, floatBuf, 1, numSamples*numChannels);
		break;
	case 24:
		Float32_To_Int24_Clip(samples, 1, floatBuf, 1, numSamples*numChannels);
		break;

	}
}

float GetGain(WMInformation *info, bool allowDefault)
{
	if (AGAVE_API_CONFIG && AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain", false))
	{
		float dB = 0, peak = 1.0f;
		wchar_t gain[64]=L"", peakVal[64]=L"";
		switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_source", 0))
		{
		case 0:    // track
			info->GetAttribute(L"replaygain_track_gain", gain, 64);
			if (!gain[0]	&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				info->GetAttribute(L"replaygain_album_gain", gain, 64);

			info->GetAttribute(L"replaygain_track_peak", peakVal, 64);
			if (!peakVal[0]	&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				info->GetAttribute(L"replaygain_album_peak", peakVal, 64);

			break;
		case 1:
			info->GetAttribute(L"replaygain_album_gain", gain, 64);
			if (!gain[0]	&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				info->GetAttribute(L"replaygain_track_gain", gain, 64);

			info->GetAttribute(L"replaygain_album_peak", peakVal, 64);
			if (!peakVal[0]	&& !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
				info->GetAttribute(L"replaygain_track_peak", peakVal, 64);

			break;
		}

		_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();

		if (gain[0])
		{
			if (gain[0] == L'+')
				dB = static_cast<float>(_wtof_l(&gain[1],C_locale));
			else
				dB = static_cast<float>(_wtof_l(gain,C_locale));
		}
		else if (allowDefault)
		{
			dB = AGAVE_API_CONFIG->GetFloat(playbackConfigGroupGUID, L"non_replaygain", -6.0);
			return powf(10.0f, dB / 20.0f);
		}

		if (peakVal[0])
		{
			peak = static_cast<float>(_wtof_l(peakVal,C_locale));
		}

		switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_mode", 1))
		{
		case 0:    // apply gain
			return powf(10.0f, dB / 20.0f);
		case 1:    // apply gain, but don't clip
			return min(powf(10.0f, dB / 20.0f), 1.0f / peak);
		case 2:    // normalize
			return 1.0f / peak;
		case 3:    // prevent clipping
			if (peak > 1.0f)
				return 1.0f / peak;
			else
				return 1.0f;
		}

	}

	return 1.0f; // no gain
}


void GainLayer::AudioDataReceived(void *_data, unsigned long sizeBytes, DWORD timestamp)
{
	if (enabled)
	{
		size_t samples = audio->AudioBytesToSamples(sizeBytes);
		int channels = audio->Channels();
		if (floatSize < (samples * channels))
		{
			delete [] floatData;
			floatSize = samples * channels;
			floatData = new float[floatSize];
		}
		if (outSize < sizeBytes)
		{
			delete [] outData;
			outSize=sizeBytes;
			outData = (void *)new __int8[sizeBytes];
		}
		
		FillFloat(floatData, _data, audio->BitSize(), samples , channels, replayGain);
		FillSamples(outData, floatData, audio->BitSize(), samples , channels);

		WMHandler::AudioDataReceived(outData, sizeBytes, timestamp);
	}
	else
		WMHandler::AudioDataReceived(_data, sizeBytes, timestamp);
}

void GainLayer::Opened()
{
	enabled=	(AGAVE_API_CONFIG && AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain", false));
	if (enabled)
		replayGain = GetGain(info, true);
	WMHandler::Opened();
}
