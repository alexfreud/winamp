#pragma once
#include "foundation/dispatch.h"
#include "player/ifc_playback.h"
#include "foundation/types.h"
// TODO: benski> not sure that this is the best name for it, but it works for now

class ifc_http : public Wasabi2::Dispatchable
{
protected:
	ifc_http() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_http() {}
public:

	enum
	{
		WAKE_KILL=(1<<0),
		WAKE_PLAY=(1<<1),
		WAKE_PAUSE=(1<<2), 
		WAKE_STOP=(1<<3),
		WAKE_INTERRUPT=(1<<4),
		WAKE_UNPAUSE=(1<<5), // this is actually unused in wake_flags, just used as a return value from Wake/WakeReason		
		WAKE_RESUME=(1<<6), // this is actually unused in wake_flags, just used as a return value from Wake/WakeReason		
		WAKE_START_MASK = WAKE_PLAY|WAKE_STOP, 
		WAKE_KILL_MASK = WAKE_KILL|WAKE_STOP,
		WAKE_ALL_MASK = WAKE_KILL|WAKE_PLAY|WAKE_PAUSE|WAKE_STOP|WAKE_INTERRUPT,
	};

		// these aren't the best names, either

	// if playback flag isn't ready, sleeps until a flag changes and returns changed flag
	int Wake(int mask) { return HTTP_Wake(mask); }

	// checks for pending flags and updates them
	int Check(int mask) { return HTTP_Check(mask); }

	// like wake, but only wait a specified amount of time.  will return 0 if flags didn't change
	int Wait(unsigned int milliseconds, int mask) { return HTTP_Wait(milliseconds, mask); }

	int Sleep(unsigned int milliseconds, int mask) { return HTTP_Sleep(milliseconds, mask); }

	Agave_Seek *GetSeek() { return HTTP_GetSeek(); }
	void FreeSeek(Agave_Seek *seek) { HTTP_FreeSeek(seek); }

	int Seek(uint64_t byte_position) { return HTTP_Seek(byte_position); }
	
	/* returns NErr_True / NErr_False, returns whether or not it's seekable by Range headers.  
	 NErr_True doesn't mean 100% certainity that the stream is seekable.
	 Note that some protocols (e.g. RTSP) might still be seekable by other means than Range headers. */
	int Seekable() { return HTTP_Seekable(); }

	int AudioOpen(const ifc_audioout::Parameters *format, ifc_audioout **out_output) { return HTTP_AudioOpen(format, out_output); }

	enum
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual int WASABICALL HTTP_Wake(int mask)=0;
	virtual int WASABICALL HTTP_Check(int mask)=0;
	virtual int WASABICALL HTTP_Wait(unsigned int milliseconds, int mask)=0;
	virtual int WASABICALL HTTP_Sleep(int milliseconds, int mask)=0;
	virtual Agave_Seek *WASABICALL HTTP_GetSeek()=0;
	virtual void WASABICALL HTTP_FreeSeek(Agave_Seek *seek)=0;
	virtual int WASABICALL HTTP_Seek(uint64_t byte_position)=0;
	virtual int WASABICALL HTTP_Seekable()=0;
	virtual int WASABICALL HTTP_AudioOpen(const ifc_audioout::Parameters *format, ifc_audioout **out_output)=0;

};
