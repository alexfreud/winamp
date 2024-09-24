#pragma once
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "audio/parameters.h"


class NOVTABLE ifc_audioout : public Wasabi2::Dispatchable
{
protected:
	ifc_audioout() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_audioout() {}

public:
	enum
	{
		CHANNEL_LAYOUT_MICROSOFT = 0x0, // microsoft channel order - http://www.microsoft.com/whdc/device/audio/multichaud.mspx#E4C
		CHANNEL_LAYOUT_MPEG = 0x1, 
	};


	enum
	{
		EXTENDED_FLAG_APPLY_GAIN=0x1, /* apply the gain value specified in Parameters::gain */
		EXTENDED_FLAG_REPLAYGAIN=0x2, /* pass if you tried to figure out ReplayGain on your own. otherwise the Audio Output object will apply the default gain */
		EXTENDED_FLAG_GAIN_MASK=EXTENDED_FLAG_APPLY_GAIN|EXTENDED_FLAG_REPLAYGAIN, /* a mask to check whether or not the gain value is valid */
		/* so that you can check if a flag was set that you don't understand */
		EXTENDED_FLAG_VALID_MASK=EXTENDED_FLAG_APPLY_GAIN|EXTENDED_FLAG_REPLAYGAIN,
	};

	struct Parameters
	{
		size_t sizeof_parameters;
		nsaudio::Parameters audio;
		/* anything after this needs sizeof_parameters to be large enough 
		AND a flag set in extended_fields_flags
		if there's no flag for the field, it's because a default value of 0 can be assumed */
		unsigned int extended_fields_flags; // set these if you use any of the following fields. see comment above
		double gain; // additional gain specified by client.  usually used for replaygain (so it can be combined with EQ pre-amp or float/pcm conversion)
		size_t frames_trim_start; // number of frames to trim from the start 
		size_t frames_trim_end; // number of frames to trim from the start 
	};

	int Output(const void *data, size_t data_size) { return AudioOutput_Output(data, data_size); }
	// returns number of bytes that you can write
	size_t CanWrite() { return AudioOutput_CanWrite(); }
	void Flush(double seconds) { AudioOutput_Flush(seconds); } 
	void Pause(int state) { AudioOutput_Pause(state); } 

	/* called by the input plugin when no more output will be sent */
	void Done() { AudioOutput_Done(); } 
	/* called by the input plugin when playback was forcefully stopped */
	void Stop() { AudioOutput_Stop(); }

	/* returns the latency in seconds (how many seconds until samples you're about to write show up at the audio output */
	double Latency() { return AudioOutput_Latency(); }

	/* only valid after a call to Done(). Returns NErr_True if there is still data in the buffer, NErr_False otherwise */
	int Playing() { return AudioOutput_Playing(); }

protected:
	virtual int WASABICALL AudioOutput_Output(const void *data, size_t data_size)=0;
	virtual size_t WASABICALL AudioOutput_CanWrite()=0; // returns number of bytes that you can write
	virtual void WASABICALL AudioOutput_Flush(double seconds)=0;
	virtual void WASABICALL AudioOutput_Pause(int state)=0;

	/* called by the input plugin when no more output will be sent */
	virtual void WASABICALL AudioOutput_Done()=0;
	/* called by the input plugin when playback was forcefully stopped */
	virtual void WASABICALL AudioOutput_Stop()=0;
	virtual double WASABICALL AudioOutput_Latency()=0;
	virtual int WASABICALL AudioOutput_Playing()=0;
	enum
	{
		DISPATCHABLE_VERSION,
	};
};
