#ifndef NULLSOFT_OUTPLUGINH
#define NULLSOFT_OUTPLUGINH

#include "AudioOut.h"

class OutPlugin : public AudioOut
{
public:
	OutPlugin();
	void Init();
	void Quit();
	int CanWrite();
	int GetWrittenTime();
	int IsPlaying();
	int Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms);
	void Close();
	int Write(char *buf, int len);
	void Flush(int t);
	void SetVolume(int _volume);
	int Pause(int new_state);
	int GetOutputTime();
	void SetPan(int _pan);
	void About(HWND p);
	void Config(HWND w);
};

extern OutPlugin pluginOut;
#endif
