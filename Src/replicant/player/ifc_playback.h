#pragma once
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "types.h"
#include "svc_output.h"
#include "ifc_playback_parameters.h"

/* Note that since a typical ifc_playback implementation
is running on another thread, most functions will always succeed (except for out of memory or thread creation problems)
but might report an error later to the ifc_player object that was passed
when creating the playback object */

class NOVTABLE ifc_playback : public Wasabi2::Dispatchable
{
protected:
	ifc_playback() : Dispatchable(NUM_DISPATCH_CODES) {}
	~ifc_playback() {}
public:
	/* optionally call Cue before calling Play to set the start and end positions 
		in a cuesheet situation, these will might also get called after
		you indicate that cue end position has been reached */
	int Cue(Agave_PositionType position_type, Agave_Position start, Agave_Position end) { return Playback_Cue(position_type, start, end); }
	// start only version of Cue
	int CueStart(Agave_PositionType position_type, Agave_Position start) { return Playback_CueStart(position_type, start); }

	// begins playback.
	int Play(svc_output *output, ifc_playback_parameters *secondary_parameters) { return Playback_Play(output, secondary_parameters); }

	int SeekSeconds(double seconds) { return Playback_SeekSeconds(seconds); }

	// called on user-initiated stop.  not called if the playback object indiciated a stop (e.g. EOF)
	int Stop() { return Playback_Stop(); }

	int Pause() { return Playback_Pause(); }
	int Unpause() { return Playback_Unpause(); }

	/* called to shut things down.  
	note that if your playback object is in a 'stopped' state, 
	it should be prepared to receive Cue/Play call until Close() is called */
	int Close() { return Playback_Close(); }

	/* most of the time, you'll pass SetVolume and SetPan on to the output object
	but some special-use playback objects (e.g. analog CD playback) might have to implement this */
	// 0 to 1.0
	int SetVolume(float volume) { return Playback_SetVolume(volume); }
	// -1.0 to 1.0
	int SetPan(float pan) { return Playback_SetPan(pan); }
	
	/* most of the time, you'll ignore SetEQ
	but some special-use playback objects (e.g. analog CD playback) might have to implement this */
	int SetEQ(float preamp, int num_bands, float *bands) { return Playback_SetEQ(preamp, num_bands, bands); }

	enum
	{
		NUM_DISPATCH_CODES,
	};

protected:
	virtual int WASABICALL Playback_Cue(Agave_PositionType position_type, Agave_Position start, Agave_Position end) { return NErr_NotImplemented; }
	virtual int WASABICALL Playback_CueStart(Agave_PositionType position_type, Agave_Position start) { return NErr_NotImplemented; }
	virtual int WASABICALL Playback_Play(svc_output *output, ifc_playback_parameters *secondary_parameters)=0;
	virtual int WASABICALL Playback_SeekSeconds(double seconds)=0;
	virtual int WASABICALL Playback_Stop()=0;
	virtual int WASABICALL Playback_Pause()=0;
	virtual int WASABICALL Playback_Unpause()=0;
	virtual int WASABICALL Playback_Close()=0;
	virtual int WASABICALL Playback_SetVolume(float volume) { return NErr_NotImplemented; }
	virtual int WASABICALL Playback_SetPan(float pan) { return NErr_NotImplemented; }
	virtual int WASABICALL Playback_SetEQ(float preamp, int num_bands, float *bands) { return NErr_NotImplemented; }
};

