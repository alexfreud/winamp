#ifndef NULLSOFT_GAIN_LAYER_H
#define NULLSOFT_GAIN_LAYER_H

#include "WMHandler.h"
#include "AudioFormat.h"
#include "WMInformation.h"
class GainLayer : public WMHandler
{
public:
	GainLayer(AudioFormat *_audio, WMInformation *_info) 
		: audio(_audio), info(_info), enabled(false), replayGain(1.0f),
	floatData(0),floatSize(0), outData(0), outSize(0)
	{}
	~GainLayer()
	{
		delete[]floatData;
		delete[]outData;
	}
		void AudioDataReceived(void *_data, unsigned long sizeBytes, DWORD timestamp);
		void Opened();
		AudioFormat *audio;
		WMInformation *info;
		bool enabled;
		float replayGain;

		float *floatData;
		size_t floatSize;

		void *outData;
		size_t outSize;
};
#endif