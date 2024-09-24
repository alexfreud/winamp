#ifndef NULLSOFT_WINAMP_RESAMPLER_H
#define NULLSOFT_WINAMP_RESAMPLER_H
#include <mmreg.h>
#include <msacm.h>
class Resampler
{
public:
	Resampler(size_t inputBits, size_t inputChannels, size_t inputSampleRate,
	          size_t outputBits, size_t outputChannels, size_t outputSampleRate, bool floatingPoint);
	~Resampler();
	size_t Convert(void *input, size_t *inputBytes, void *output, size_t outputBytes);
	bool OK();
	void Flush();
	double sizeFactor;
private:
	size_t UseInternalBuffer(void *output, size_t outputBytes);
	HACMSTREAM hStream;
	__int8 *buffer;
	size_t bufferAlloc;
	size_t bufferValid;
	bool eof;
};
#endif