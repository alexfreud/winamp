#ifndef NULLSOFT_AUDIOTHREADH
#define NULLSOFT_AUDIOTHREADH

#include "WMHandler.h"
#include "MediaThread.h"
#include <wmsdk.h>

class AudioLayer;

class AudioThread : public MediaThread
{
public:
	AudioThread(AudioLayer *audio);
	void Start(WMHandler *output);

	/* AddBuffers put an audio buffer in the queue
	it returns true if it was added 
	it returns false if it was NOT added.  it is up to YOU (the caller) to sleep for a while and call again
	*/
	void AudThread();
	bool EndOfFile() 
	{
		if (buffers.empty()) // if the buffers are empty, then our thread might never get a chance to signal EOF
			return true;

		if (eof) 
			return true; 
		eof=1; 

		return false;
	}

private:
    void AddAPC(MediaBuffer *);
	int eof;
	WMHandler *output;
	AudioLayer *audioLayer;

};
#endif