#ifndef NULLSOFT_WINAMP_SA_BUFFER_H
#define NULLSOFT_WINAMP_SA_BUFFER_H

#define SABUFFER_WINDOW_INCREMENT 256

class SABuffer
{
public:
	SABuffer();
	void WindowToFFTBuffer(float *wavetrum);
	unsigned int AddToBuffer(char *samples, int numChannels, int bps, int ts, unsigned int numSamples);
	bool Full() { return used == 512; }
	void CopyHalf();
	void Clear();
private:
	
	float buffer[2][512];
	float window[512];
	size_t used;
	bool init;
};


#endif