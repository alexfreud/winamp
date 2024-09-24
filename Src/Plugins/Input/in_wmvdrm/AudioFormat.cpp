#include "main.h"
#include "AudioLayer.h"

unsigned long AudioFormat::AudioSamplesToMilliseconds(unsigned long samples)
{
	return MulDiv(samples, 1000, SampleRate());
}

unsigned long AudioFormat::AudioBytesToMilliseconds(unsigned long bytes)
{
	return MulDiv(AudioBytesToSamples(bytes), 1000, SampleRate());
}

unsigned long AudioFormat::AudioMillisecondsToBytes(DWORD milliseconds)
{
	return AudioSamplesToBytes(MulDiv(milliseconds, SampleRate(), 1000));
}

unsigned long AudioFormat::AudioDurationToBytes(QWORD duration)
{
	// TODO: potential integer overflow
	return AudioSamplesToBytes(MulDiv((int)duration, SampleRate(), 1000*10000));
}

unsigned long AudioFormat::AudioBytesToSamples(unsigned long bytes)
{
	return bytes / waveFormat->Format.nBlockAlign;
}

unsigned long AudioFormat::AudioSamplesToBytes(unsigned long samples)
{
	return samples * waveFormat->Format.nBlockAlign;
}

long AudioFormat::Channels()
{
	return waveFormat->Format.nChannels;
}

long AudioFormat::ValidBits()
{
	if (waveFormat->Format.wFormatTag == WAVE_FORMAT_PCM)
	{
		return waveFormat->Format.wBitsPerSample;
	}
	if (waveFormat->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		return waveFormat->Samples.wValidBitsPerSample;
	}
	return 0;
}

long AudioFormat::BitSize()
{
	return waveFormat->Format.wBitsPerSample;
}

long AudioFormat::SampleRate()
{
	return waveFormat->Format.nSamplesPerSec;
}