#include "main.h"
#include <math.h>
#include "../ReplayGainAnalysis/gain_analysis.h"
#include "api__ml_rg.h"
#include <shlwapi.h>
#include <strsafe.h>
#include <locale.h>

#pragma intrinsic(fabs)

static inline float fastmax(float x, float a)
{
	x -= a;
	x += (float)fabs(x);
	x *= 0.5f;
	x += a;
	return (x);
}

static HMODULE rgLib = 0;
typedef int(*INITGAINANALYSIS)(void *context, long samplefreq);
static INITGAINANALYSIS InitGainAnalysis = 0;
typedef int(*ANALYZESAMPLES)(void *context, const float * left_samples, const float * right_samples, size_t num_samples, int num_channels);
static ANALYZESAMPLES AnalyzeSamples = 0;
typedef int(*RESETSAMPLEFREQUENCY)(void *context, long samplefreq);
static RESETSAMPLEFREQUENCY ResetSampleFrequency = 0;
typedef float(*GETTITLEGAIN)(void *context);
static GETTITLEGAIN GetTitleGain = 0;
typedef float(*GETALBUMGAIN)(void *context);
static GETALBUMGAIN GetAlbumGain = 0;
typedef void *(* CREATERGCONTEXT)();
static CREATERGCONTEXT CreateRGContext = 0;
typedef void(*FREERGCONTEXT)(void *context);
static FREERGCONTEXT FreeRGContext = 0;

void LoadRG()
{
	if (rgLib)
		return ;

	wchar_t path[MAX_PATH] = {0};
	PathCombineW(path, (wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETSHAREDDLLDIRECTORYW), L"ReplayGainAnalysis.dll");
	rgLib = LoadLibraryW(path);

	if (rgLib)
	{
		InitGainAnalysis = (INITGAINANALYSIS)GetProcAddress(rgLib, "WAInitGainAnalysis");
		AnalyzeSamples = (ANALYZESAMPLES)GetProcAddress(rgLib, "WAAnalyzeSamples");
		GetTitleGain = (GETTITLEGAIN)GetProcAddress(rgLib, "WAGetTitleGain");
		ResetSampleFrequency = (RESETSAMPLEFREQUENCY)GetProcAddress(rgLib, "WAResetSampleFrequency");
		GetAlbumGain = (GETALBUMGAIN)GetProcAddress(rgLib, "WAGetAlbumGain");
		CreateRGContext = (CREATERGCONTEXT)GetProcAddress(rgLib, "WACreateRGContext");
		FreeRGContext = (FREERGCONTEXT)GetProcAddress(rgLib, "WAFreeRGContext");
	}
}

void *CreateRG()
{
	LoadRG();

	return CreateRGContext();
}

void DestroyRG(void *context)
{
	FreeRGContext(context);
}

#define CHUNKSIZE 16384
static void CalculateRG_float(void *context, ifc_audiostream *decoder, AudioParameters *parameters, wchar_t track_gain[64], wchar_t track_peak[64], ProgressCallback *callback, int *killSwitch, float &albumPeak)
{
	float data[2*CHUNKSIZE] = {0};
	float right[CHUNKSIZE] = {0};
	float peak = 0;

	if (parameters->channels > 2)
	{
		char titleStr[32] = {0};
		MessageBoxA(GetDialogBoxParent(),
			WASABI_API_LNGSTRING(IDS_TOO_MANY_CHANNELS),
			WASABI_API_LNGSTRING_BUF(IDS_REPLAYGAIN,titleStr,32),
			MB_OK);
		decodeFile->CloseAudio(decoder);
		return ;
	}
	ResetSampleFrequency(context, parameters->sampleRate);
	if (callback) callback->InformSize((parameters->sizeBytes == -1) ? 0 : parameters->sizeBytes);
	while (1)
	{
		if (*killSwitch)
		{
			decodeFile->CloseAudio(decoder);
			return ;
		}
		int error=0;
		size_t bytesRead = decoder->ReadAudio((void *)data, sizeof(data), killSwitch, &error);
		if (*killSwitch)
		{
			decodeFile->CloseAudio(decoder);
			return ;
		}
		else if (error)
		{
			break;
		}
		if (callback) callback->Progress(bytesRead);

		size_t samples = bytesRead / sizeof(*data);

		if (!samples)
			break;

		for (size_t i = 0;i != samples;i++)
		{
			peak = fastmax(peak, (float)fabs(data[i]));
			data[i] *= 32768.0f;
		}
		albumPeak = fastmax(peak, albumPeak);

		if (parameters->channels == 1)
			AnalyzeSamples(context, data, NULL, samples, 1);
		else
		{
			size_t samples2 = samples / 2;
			for (size_t i = 0;i != samples2;i++)
			{
				data[i] = data[i * 2];
				right[i] = data[i * 2 + 1];
			}
			AnalyzeSamples(context, data, right, samples2, 2);
		}
	}
	decodeFile->CloseAudio(decoder);
	float gain = GetTitleGain(context);
	if (gain != GAIN_NOT_ENOUGH_SAMPLES)
	{
		_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();
		_snwprintf_l(track_gain, 64, L"%-+.2f dB", C_locale, gain);
		_snwprintf_l(track_peak, 64, L"%-.9f", C_locale, peak);
	}
}

static void FillFloat(float *left, float *right, void *samples, size_t bps, size_t numSamples, size_t numChannels, float &peak, float &albumPeak, float gain)
{
	switch (bps)
	{
	case 8:
		{
			unsigned __int8 *samples8 = (unsigned __int8 *)samples;
			size_t t = 0;
			for (size_t x = 0; x != numSamples; x ++)
			{
				left[x] = (float)(samples8[t++] - 128) * 256.0f * gain;

				if (numChannels == 2)
				{
					right[x] = (float)(samples8[t++] - 128) * 256.0f* gain;
				}
				else
					right[x] = left[x];		
				peak = fastmax(peak, (float)fabs(left[x]));
				peak = fastmax(peak, (float)fabs(right[x]));
				albumPeak=fastmax(albumPeak, peak);
			}
		}
		break;
	case 16:
		{
			short *samples16 = (short *)samples;
			size_t t = 0;
			if (numChannels == 1)
			{
				for (size_t x = 0; x != numSamples; x ++)
				{
					left[x] = (float)samples16[t++] * gain;
					right[x] = left[x];
					peak = fastmax(peak, (float)fabs(left[x]));
					albumPeak=fastmax(albumPeak, peak);

				}
			}
			else if (numChannels == 2)
			{
				for (size_t x = 0; x != numSamples; x ++)
				{
					left[x] = (float)samples16[t++] * gain ;
					right[x] = (float)samples16[t++] * gain;

					peak = fastmax(peak, (float)fabs(left[x]));
					peak = fastmax(peak, (float)fabs(right[x]));
					albumPeak=fastmax(albumPeak, peak);
				}
			}
		}
		break;
	case 24:
		{
			unsigned __int8 *samples8 = (unsigned __int8 *)samples;
			for (size_t x = 0; x != numSamples; x ++)
			{
				long temp = (((long)samples8[0]) << 8);
				temp = temp | (((long)samples8[1]) << 16);
				temp = temp | (((long)samples8[2]) << 24);
				left[x] = (float)temp* gain / 65536.0f;
				samples8 += 3;
				if (numChannels == 2)
				{
					temp = (((long)samples8[0]) << 8);
					temp = temp | (((long)samples8[1]) << 16);
					temp = temp | (((long)samples8[2]) << 24);
					right[x] = (float)temp* gain / 65536.0f;
					samples8 += 3;
				}
				else
					right[x] = left[x];
				peak = fastmax(peak, (float)fabs(left[x]));
				peak = fastmax(peak, (float)fabs(right[x]));
				albumPeak=fastmax(albumPeak, peak);

			}
		}
		break;
	}

}
#undef CHUNKSIZE
#define CHUNKSIZE 4096
static void CalculateRG_pcm(void *context, ifc_audiostream *decoder, AudioParameters *parameters, wchar_t track_gain[64], wchar_t track_peak[64], ProgressCallback *callback, int *killSwitch, float &albumPeak)
{
	char data[4*2*CHUNKSIZE] = {0};
	float left[CHUNKSIZE] = {0};
	float right[CHUNKSIZE] = {0};
	float peak = 0;
	if (parameters->channels > 2)
	{
		char titleStr[32];
		MessageBoxA(GetDialogBoxParent(),
			WASABI_API_LNGSTRING(IDS_TOO_MANY_CHANNELS),
			WASABI_API_LNGSTRING_BUF(IDS_REPLAYGAIN,titleStr,32),
			MB_OK);
		decodeFile->CloseAudio(decoder);
		return ;
	}

	int padded_bits = (parameters->bitsPerSample + 7) & (~7);
	albumPeak *= 32768.0f;
	ResetSampleFrequency(context, parameters->sampleRate);
	if (callback) callback->InformSize((parameters->sizeBytes == -1) ? 0 : parameters->sizeBytes);
	while (1)
	{
		if (*killSwitch)
		{
			decodeFile->CloseAudio(decoder);
			return ;
		}
		int error=0;
		size_t bytesRead = decoder->ReadAudio((void *)data, 4096 * parameters->channels * (padded_bits / 8), killSwitch, &error);
		if (*killSwitch)
		{
			decodeFile->CloseAudio(decoder);
			return ;
		}
		else if (error)
		{
			break;
		}

		if (callback) callback->Progress(bytesRead);

		size_t samples = bytesRead / (padded_bits / 8);

		if (!samples)
			break;
		
		FillFloat(left, right, data, padded_bits, samples / parameters->channels, parameters->channels, peak, albumPeak, (float)pow(2., (double)(padded_bits - parameters->bitsPerSample)));

		size_t samples2 = samples / 2;
		AnalyzeSamples(context, left, right, samples2, 2);
	}
	decodeFile->CloseAudio(decoder);
	float gain = GetTitleGain(context);
	if (gain != GAIN_NOT_ENOUGH_SAMPLES)
	{
		StringCchPrintfW(track_gain, 64, L"%-+.2f dB", gain);
		StringCchPrintfW(track_peak, 64, L"%-.9f", peak / 32768.0f);
	}

	albumPeak /= 32768.0f;
}

void CalculateRG(void *context, const wchar_t *filename, wchar_t track_gain[64], wchar_t track_peak[64], ProgressCallback *callback, int *killSwitch, float &albumPeak)
{
	LoadRG();
	if (!rgLib)
	{
		char titleStr[32] = {0};
		MessageBoxA(GetDialogBoxParent(),
			WASABI_API_LNGSTRING(IDS_NOT_ABLE_TO_OPEN_RG_DLL),
			WASABI_API_LNGSTRING_BUF(IDS_REPLAYGAIN,titleStr,32),
			MB_OK);
		return ;
	}

	wchar_t dummy[64] = {0};
	if (!GetFileInfo(filename, L"replaygain_track_gain", dummy, 64)) // check if the plugin even supports replaygain
		return ;

	/*
	TODO: want to do something like this, but have to do it on the main thread (ugh)
	if (!_wcsnicmp(dummy, "-24601", 6))
	{
	SetFileInfo(itr->filename, L"replaygain_track_gain", L"");
	SetFileInfo(itr->filename, L"replaygain_track_peak", L"");
	SetFileInfo(itr->filename, L"replaygain_album_gain", L"");
	SetFileInfo(itr->filename, L"replaygain_album_peak", L"");
	WriteFileInfo();
	}
	*/

	AudioParameters parameters;
	parameters.flags = AUDIOPARAMETERS_FLOAT | AUDIOPARAMETERS_MAXCHANNELS | AUDIOPARAMETERS_MAXSAMPLERATE;
	parameters.channels = 2;
	parameters.sampleRate = 192000;

	ifc_audiostream *decoder = decodeFile->OpenAudioBackground(filename, &parameters);
	if (decoder)
		CalculateRG_float(context, decoder, &parameters, track_gain, track_peak, callback, killSwitch, albumPeak);
	else
	{
		// try PCM
		memset(&parameters, 0, sizeof(AudioParameters));
		parameters.flags = AUDIOPARAMETERS_MAXCHANNELS | AUDIOPARAMETERS_MAXSAMPLERATE;
		parameters.channels = 2;
		parameters.sampleRate = 192000;

		ifc_audiostream *decoder = decodeFile->OpenAudioBackground(filename, &parameters);
		if (decoder)
			CalculateRG_pcm(context, decoder, &parameters, track_gain, track_peak, callback, killSwitch, albumPeak);
	}

}

void CalculateAlbumRG(void *context, wchar_t album_gain[64], wchar_t album_peak[64], float &albumPeak)
{
	float gain = GetAlbumGain(context);
	if (gain != GAIN_NOT_ENOUGH_SAMPLES)
	{
		/*StringCchPrintfW(album_gain, 64, L"%-+.2f dB", gain);
		StringCchPrintfW(album_peak, 64, L"%-.9f", albumPeak);*/
		_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();
		_snwprintf_l(album_gain, 64, L"%-+.2f dB", C_locale, gain);
		_snwprintf_l(album_peak, 64, L"%-.9f", C_locale, albumPeak);
	}
}

void StartRG(void *context)
{
	LoadRG();
	if (!rgLib)
	{
		char titleStr[32] = {0};
		MessageBoxA(GetDialogBoxParent(),
			WASABI_API_LNGSTRING(IDS_NOT_ABLE_TO_OPEN_RG_DLL),
			WASABI_API_LNGSTRING_BUF(IDS_REPLAYGAIN,titleStr,32),
			MB_OK);
		return ;
	}

	InitGainAnalysis(context, 44100); // since this is most common.  We'll reset it before doing a real calculation anyway
}
