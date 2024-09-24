#ifndef NULLSOFT_IN_MP4_AUDIOSAMPLE_H
#define NULLSOFT_IN_MP4_AUDIOSAMPLE_H

#include "main.h"

class AudioSample
{
public:
	AudioSample(size_t maxInput, size_t maxOutput)
	{
		input = (unsigned __int8 *)calloc(maxInput, sizeof(unsigned __int8));
		inputSize = maxInput;

		output = (__int8 *)calloc(maxOutput, sizeof(__int8));
		outputSize = maxOutput;

		inputValid = outputValid = result = sampleRate = numChannels =
		bitsPerSample = bitrate = sampleId = timestamp = duration = offset = 0;
		outputCursor = 0;
	}
	~AudioSample()
	{
		free(output);
		free(input);
	}
	bool OK()
	{
		return input && output;
	}
	// input
	unsigned __int8 *input;
	size_t inputSize, inputValid;
	MP4SampleId sampleId;

	// output
	__int8 *output, *outputCursor;
	size_t outputSize, outputValid;
	MP4Duration duration, offset, timestamp;
	int result;
	unsigned int sampleRate, numChannels, bitsPerSample;
	unsigned int bitrate;
};

class VideoSample
{
public:
	VideoSample(size_t maxInput) 
	{
		input = (unsigned __int8 *)calloc(maxInput, sizeof(unsigned __int8));
		inputSize = maxInput;
		timestamp = inputValid = 0;
	}
	~VideoSample()
	{
		free(input);
	}
	bool OK()
	{
		return !!input;
	}
	// input
	unsigned __int8 *input;
	size_t inputSize, inputValid;

	MP4Timestamp timestamp;
};

class DecodedVideoSample
{
public:
	~DecodedVideoSample()
	{
		decoder->FreePicture(output,decoder_data);
	}
	void *output;
	void *decoder_data;
	MP4VideoDecoder *decoder;
	MP4Timestamp timestamp;
};
#endif