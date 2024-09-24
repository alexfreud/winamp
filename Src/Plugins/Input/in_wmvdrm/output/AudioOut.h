#ifndef NULLSOFT_AUDIOOUTH
#define NULLSOFT_AUDIOOUTH

#include <windows.h>
#include "../../../../Winamp/out.h"

enum InitState
{
    StatusNone = 0,
    StatusInit,
    StatusQuit
};

class AudioOut
{
public:
	AudioOut() : dllInstance(0), winampWnd(NULL) {}
	virtual void Init() = 0;
	virtual void Quit() = 0;
	virtual int CanWrite() = 0;
	virtual int GetWrittenTime() = 0;
	virtual int IsPlaying() = 0;
	virtual int Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms) = 0;
	virtual void Close() = 0;
	virtual int Write(char *buf, int len) = 0;
	virtual void Flush(int t) = 0;
	virtual void SetVolume(int _volume) = 0;
	virtual int Pause(int new_state) = 0;
	virtual int GetOutputTime() = 0;
	virtual void SetPan(int _pan) = 0;
	virtual void About(HWND p) = 0;
	virtual void Config(HWND w) = 0;

	HINSTANCE dllInstance;
	HWND winampWnd;
};

#endif
