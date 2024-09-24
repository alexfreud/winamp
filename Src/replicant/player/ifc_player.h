#pragma once
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "foundation/types.h"
#include "metadata/ifc_metadata.h"
#include "audio/ifc_equalizer.h"
#include "nx/nxuri.h"

/* implemented by Winamp (or whatever application)
your ifc_playback implementation should call this with events

TODO: benski> should we require the ifc_playback object to get passed in to each function? 
*/

class NOVTABLE ifc_player : public Wasabi2::Dispatchable
{
protected:
	ifc_player() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_player() {}
public:

	/* When your playback object has read enough of the file to provide metadata
	it should call this method.
	It will be called for both song metadata (artist, album, etc) and codec metadata (bitrate, samplerate, etc).
	so make sure you can provide both - this might mean you have to wait until the first frame is decoded.
	The player object will add a reference in the function, and release whenever it is no longer needed (usually on start of next track)
	*/
	void SetMetadata(ifc_metadata *metadata) { Player_SetMetadata(metadata); }

	/* Call this just once (with timestamp=0) for CBR files	or update continuously for VBR 
	bitrate should be in bits per second, e.g. 128000 (not 128!)
	*/
	void SetBitrate(uint64_t bitrate, double timestamp) { Player_SetBitrate(bitrate, timestamp); } 

	/* Playback plugin should call this when it knows the length
	can be updated if necessary (e.g. for VBR files w/o header)
	If this is not called, it assumed to be a stream (radio)
	*/
	void SetLength(double length) { Player_SetLength(length); }

	/* Output plugin should call this every once in a while,
	if your input plugin does its own audio output (e.g. analog CD) you should call this yourself
	*/
	void SetPosition(double timestamp) { Player_SetPosition(timestamp); }

	void OnLoaded(nx_uri_t filename) { Player_OnLoaded(filename); }
	/* Input plugin should call this when playback ends at the end of the file
	Do not call if playback stopped because of an error */
	void OnEndOfFile() { Player_OnEndOfFile(); }

	void OnError(NError code) { Player_OnError(code); }

	void OnStopped() { Player_OnStopped(); }

	void SetEqualizer(ifc_equalizer *equalizer) { return Player_SetEqualizer(equalizer); }

	/* percent is 0-100.  setting to 100 implies that buffering has finished */
	void SetBufferStatus(int percent) { return Player_SetBufferStatus(percent); }
	
	void OnSeekComplete(int error_code, double new_position) { return Player_OnSeekComplete(error_code, new_position); }
	
	/* seekable is 0 (false) or 1 (true) */
	void SetSeekable(int seekable) { return Player_SetSeekable(seekable); }
	
	void AsynchronousFunctionCall(void (*function)(void *, void *, double), void *param1, void *param2, double real_param) { Player_AsynchronousFunctionCall(function, param1, param2, real_param); }

	/* Call this after you've successfully opened and parsed the playback file, and are attempting to start playback */
	void OnReady() { Player_OnReady(); };

	/* Call this after EndOfFile() when either 1) You process an ifc_playback::Close() call or 2) ifc_audioout::IsPlaying() returns NErr_False while waiting for a Close() call */
	void OnClosed() { Player_OnClosed(); };

	enum
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual void WASABICALL Player_SetMetadata(ifc_metadata *metadata) = 0;
	virtual void WASABICALL Player_SetBitrate(uint64_t bitrate, double timestamp) = 0;
	virtual void WASABICALL Player_SetLength(double length) = 0;
	virtual void WASABICALL Player_SetPosition(double timestamp) = 0;
	virtual void WASABICALL Player_OnLoaded(nx_uri_t filename) = 0;
	virtual void WASABICALL Player_OnEndOfFile() = 0;
	virtual void WASABICALL Player_OnError(NError code) = 0;
	virtual void WASABICALL Player_OnStopped()=0;
	virtual void WASABICALL Player_SetEqualizer(ifc_equalizer *equalizer)=0;
	virtual void WASABICALL Player_SetBufferStatus(int percent)=0;
	virtual void WASABICALL Player_OnSeekComplete(int error_code, double new_position)=0;
	virtual void WASABICALL Player_SetSeekable(int seekable)=0;
	virtual void WASABICALL Player_AsynchronousFunctionCall(void (*function)(void *, void *, double), void *param1, void *param2, double real_param)=0;
	virtual void WASABICALL Player_OnReady()=0;
	virtual void WASABICALL Player_OnClosed()=0;
};
