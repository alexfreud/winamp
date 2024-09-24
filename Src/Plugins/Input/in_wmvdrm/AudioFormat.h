#ifndef NULLSOFT_IN_WMVDRM_AUDIOFORMAT_H
#define NULLSOFT_IN_WMVDRM_AUDIOFORMAT_H

#include <mmreg.h>
#include <wmsdk.h>

class AudioFormat
{
public:
	AudioFormat() : waveFormat(0)
	{
	}
	~AudioFormat()
	{
		delete [] waveFormat;
	}
	unsigned long AudioBytesToSamples(unsigned long bytes);
	unsigned long AudioSamplesToBytes(unsigned long samples);
	unsigned long AudioBytesToMilliseconds(unsigned long bytes);
	unsigned long AudioMillisecondsToBytes(DWORD milliseconds);
	unsigned long AudioDurationToBytes(QWORD duration);
	unsigned long AudioSamplesToMilliseconds(unsigned long samples);
	long Channels();
	long ValidBits();
	long BitSize();
	long SampleRate();
//protected:
	void Open(WM_MEDIA_TYPE *mediaType)
	{
		delete[] waveFormat;
		waveFormat = (WAVEFORMATEXTENSIBLE *) new unsigned char[mediaType->cbFormat];
		memcpy(waveFormat, mediaType->pbFormat, mediaType->cbFormat);
	}

	void Close()
	{
		delete [] waveFormat;
		waveFormat=0;
	}

private:
	WAVEFORMATEXTENSIBLE *waveFormat;
};

#endif