#ifndef __C_CROSSFADER_H__
#define __C_CROSSFADER_H__

#include "../Include/c_datapump.h"

class C_CROSSFADER : public C_DATAPUMP<short> {
private:
protected:
	int BufferLength; // in milliseconds
	int srate;
	int nch;
	int crossfade;
	int mode;

	void SampleRateConvert(int newsrate);
	void ChannelConvert(int newnch);

	virtual void addItems(short *inputBuffer, size_t inputSize);  // overriding the addItems() function to do crossfading and channels
public:
	C_CROSSFADER(int length, int nCh, int sRate); // length is in milliseconds
	virtual ~C_CROSSFADER();

	void SetChannels(int nCh);
	void SetSampleRate(int sRate); // in samples per second
	void SetBufferLength(int bufferLength); // in milliseconds
	void SetCrossfading(int onoff);
	void SetCrossfadeMode(int Mode); // 0 = X-style, 1 = h-style

	virtual size_t put(short *inputBuffer, size_t inputSize); // in channel-less shorts
	virtual size_t get(short *outputBuffer, size_t outputSize, int nCh); // in channel-less shorts
};

#endif // !__C_CROSSFADER_H__