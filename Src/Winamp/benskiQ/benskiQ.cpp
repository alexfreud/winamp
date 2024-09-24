#include "main.h"

#include "EqBand.h"
#include "WinampAttributes.h"

#include <math.h>

extern int filter_srate,  filter_enabled, filter_top, filter_top2;
extern float preamp_val;

static int filter_nch=0;
static bool init=false;
static EqBand benskiQ[10];
//#define BENSKIQ_Q 0.70710678118654752440084436210485
#define BENSKIQ_Q 1.41
static double benskiQ_freqs_iso[10]={31.5, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000}; // ISO standard equalizer frequency table
static double benskiQ_freqs[10]={ 70, 180, 320, 600, 1000, 3000, 6000, 12000, 14000, 16000 }; // winamp style frequency table

static CRITICAL_SECTION benskiQ_cs;
void benskiQ_init()
{
	InitializeCriticalSection(&benskiQ_cs);
	for (int x=0;x<10;x++)
	{
		benskiQ[x].set_parameters((config_eq_frequencies==EQ_FREQUENCIES_WINAMP)?benskiQ_freqs[x]:benskiQ_freqs_iso[x], 1.0, BENSKIQ_Q);
	}
}


static __inline double VALTODB(int v)
{
	v -= 31;
	if (v < -31) v = -31;
	if (v > 32) v = 32;

	if (v > 0) return -12.0*(v / 32.0);
	else if (v < 0)
	{
		return -12.0*(v / 31.0);
	}
	return 0.0f;
}

static __inline double VALTOGAIN(int v)
{
	return pow(10.0, VALTODB(v)/20.0);
}

void benskiQ_eq_set(char data[10])
{
	if (!init)
	{
		init=true; benskiQ_init();
	}
	EnterCriticalSection(&benskiQ_cs);
	for (int x = 0; x < 10; x ++)
	{
		benskiQ[x].set_parameters((config_eq_frequencies==EQ_FREQUENCIES_WINAMP)?benskiQ_freqs[x]:benskiQ_freqs_iso[x], VALTOGAIN(data[x]), BENSKIQ_Q);
	}
	LeaveCriticalSection(&benskiQ_cs);
}

static void FillFloat(float **floatBuf, void *samples, size_t bps, size_t numSamples, size_t numChannels, float preamp)
{
	switch (bps)
	{
	case 8:
		{
			preamp /= 256.0f;
			unsigned __int8 *samples8 = (unsigned __int8 *)samples;
			for (size_t c=0;c<numChannels;c++)
				for (size_t x = 0; x != numSamples; x ++)
				{
					floatBuf[c][x] = (float)(samples8[c+numChannels*x]-128) * preamp;
				}
		}
		break;
	case 16:
		{
			preamp/=32768.0f;
			short *samples16 = (short *)samples;
			for (size_t c=0;c<numChannels;c++)
				for (size_t x = 0; x != numSamples; x ++)
				{
					floatBuf[c][x] = (float)samples16[c+numChannels*x] * preamp;
				}
		}
		break;
	case 24:
		{
			preamp/=2147483648.0f;
			unsigned __int8 *samples8 = (unsigned __int8 *)samples;

			long temp;

			for (size_t x = 0; x != numSamples; x ++)
				for (size_t c=0;c<numChannels;c++)
				{
					temp = (((long)samples8[0]) << 8);
					temp = temp | (((long)samples8[1]) << 16);
					temp = temp | (((long)samples8[2]) << 24);
					floatBuf[c][x] = (float)temp * preamp;
					samples8+=3;
				}
		}
		break;
	case 32:
		{
			preamp /= 2147483648.0f;
			int32_t *samples32 = (int32_t *)samples;
			for (size_t x = 0; x != numSamples; x ++)
				for (size_t c=0;c<numChannels;c++)
				{
					floatBuf[c][x] = (float)samples32[c+x*numChannels] * preamp;
				}
		}
		break;
	}
}

static void FillSamples(void *samples, float **floatBuf, size_t bps, size_t numSamples, size_t numChannels)
{
	switch (bps)
	{
	case 16:
		for (size_t i=0;i<numChannels;i++)
			Float32_To_Int16_Clip((char *)samples+i*(bps/8), (signed int)numChannels, floatBuf[i], 1, (unsigned int) numSamples);
		break;
	case 24:
		for (size_t i=0;i<numChannels;i++)
			Float32_To_Int24_Clip((char *)samples+i*(bps/8), (signed int)numChannels, floatBuf[i], 1, (unsigned int) numSamples);
		break;

	}
}

static int last_nch=0, last_numsamples=0;
static float **last_sample=0;
void DeleteSample(float **big, int nch)
{
	for (int i=0;i<nch;i++)
		delete big[i];
	delete[]big;
}

float **MakeSample(int numsamples, int nch)
{
	if (last_nch < nch || last_numsamples < numsamples)
	{
		DeleteSample(last_sample, last_nch);
		last_nch=max(nch, last_nch);
		last_numsamples=max(numsamples, last_numsamples);
		last_sample = new float*[last_nch];
		for (int i=0;i<last_nch;i++)
			last_sample [i]=new float[last_numsamples];
	}
	return last_sample;
}

void benskiQ_reset(int srate, int nch)
{
	for (int i=0;i<10;i++)
	{
		benskiQ[i].SetSampleRate(srate);
		benskiQ[i].set_num_channels(nch);
	}

	int x;
	if (config_eq_frequencies == EQ_FREQUENCIES_WINAMP)
		for (x = 0; x < 10 && benskiQ_freqs[x]*2 <= srate; x++);
	else
		for (x = 0; x < 10 && benskiQ_freqs_iso[x]*2 <= srate; x++);
	filter_top = min(x, filter_top2);
	filter_srate=srate;
	filter_nch=nch;
}

static float NonReplayGainAdjust()
{
	if (!(in_mod->UsesOutputPlug&IN_MODULE_FLAG_REPLAYGAIN) && config_replaygain.GetBool())
		return pow(10.0f, (float)config_replaygain_non_rg_gain/20.0f);
	else
		return 1.0f;
}

static float ReplayGainPreamp()
{
	if (!(in_mod->UsesOutputPlug&IN_MODULE_FLAG_REPLAYGAIN_PREAMP) && config_replaygain.GetBool())
	   return pow(10.0f, (float)config_replaygain_preamp/20.0f);
	else
		return 1.0f; 
}

int benskiQ_eq_dosamples(short *samples, int numsamples, int bps, int nch, int srate)
{
	if (filter_enabled && in_mod && !(in_mod->UsesOutputPlug&IN_MODULE_FLAG_EQ) && bps != 32)
	{
		if (srate !=filter_srate || nch != filter_nch)
			benskiQ_reset(srate, nch);

		if (!init)
		{
			init=true; benskiQ_init();
		}
		float **in = MakeSample(numsamples, nch);

		FillFloat(in, samples, bps, numsamples, nch, preamp_val*NonReplayGainAdjust()*ReplayGainPreamp());
		EnterCriticalSection(&benskiQ_cs);
		for (int x = 0; x < filter_top; x ++)
		{
			benskiQ[x].process(in, in, numsamples, nch);
		}
		LeaveCriticalSection(&benskiQ_cs);
		FillSamples(samples, in, bps, numsamples, nch);
	}
	else if (!(in_mod->UsesOutputPlug&IN_MODULE_FLAG_REPLAYGAIN) && config_replaygain.GetBool() && (config_replaygain_non_rg_gain.GetFloat() != 0) && bps != 32)
	{
		float **in = MakeSample(numsamples, nch);
		FillFloat(in, samples, bps, numsamples, nch, NonReplayGainAdjust()*ReplayGainPreamp());
		FillSamples(samples, in, bps, numsamples, nch);
	}
	else if (!(in_mod->UsesOutputPlug&IN_MODULE_FLAG_REPLAYGAIN_PREAMP) && config_replaygain.GetBool() && (config_replaygain_preamp.GetFloat() != 0) && bps != 32)
	{
		float **in = MakeSample(numsamples, nch);
		FillFloat(in, samples, bps, numsamples, nch, ReplayGainPreamp());
		FillSamples(samples, in, bps, numsamples, nch);
	}
	else
		filter_srate = 0;
	return dsp_dosamples(samples, numsamples, bps, nch, srate);
}
