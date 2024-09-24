#pragma once
#include "foundation/dispatch.h"

/* ok, this could have easily been a config group, but I decided to make it its own interface for two reasons
1) it's a little more maintainable and easier to understand and use, rather than just using a generic key/value interface
2) the original ReplayGain specs suggests using the average gain of the last 10 played tracks in cases of missing replay gain data
http://replaygain.hydrogenaudio.org/proposal/player_scale.html
So we need a way to have the playback object pass along gain values so they can be averages in this way

*/

// {1CE24DEC-A189-4BC7-86A7-C6CDB0F8953D}
static const GUID replaygain_settings_interface_guid = 
{ 0x1ce24dec, 0xa189, 0x4bc7, { 0x86, 0xa7, 0xc6, 0xcd, 0xb0, 0xf8, 0x95, 0x3d } };

class ifc_metadata;

class ifc_replaygain_settings : public Wasabi2::Dispatchable
{
protected:
	ifc_replaygain_settings() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_replaygain_settings() {}
public:
	static GUID GetInterfaceGUID() { return replaygain_settings_interface_guid; }

	enum
	{
		REPLAYGAIN_OFF=0x0,
		REPLAYGAIN_ON=0x1,
		REPLAYGAIN_MODE_TRACK=0x2,
		REPLAYGAIN_MODE_ALBUM=0x4,
		REPLAYGAIN_MODE_MASK=REPLAYGAIN_MODE_TRACK|REPLAYGAIN_MODE_ALBUM,
		REPLAYGAIN_PREVENT_CLIPPING=0x8,
		REPLAYGAIN_AUTO=0x16, // automatically determine gain values for unscanned tracks from prior history
	};

	/* pass the ifc_metadata associated with the track
	warning values:
	NErr_Success: gain is set to the gain adjustment to play this track with
	NErr_False: gain is set to the default gain adjustment because there was no metadata
	NErr_Unknown: gain is set to the default gain adjustment, because the metadata object does not understand the ReplayGain metadata keys (MetadataKeys::TRACK_GAIN, etc) 

	return values:
	NErr_Success: *gain has been assigned and is valid
	NErr_Disabled: ReplayGain is turned off
	*/
	int GetGain(ifc_metadata *metadata, double *gain, int *warning) { return ReplayGainSettings_GetGain(metadata, gain, warning); }

	/* Adds a track to the history, which is optionally used when playing back tracks without ReplayGain
	pass the track length (or an estimate).  Pass 0.0 for length if don't know.
	You should only call this if GetGain returned NErr_Success and there was no warning! 
	*/
	int AddToHistory(double seconds, double gain) { return ReplayGainSettings_AddToHistory(seconds, gain); }
private:
	virtual int WASABICALL ReplayGainSettings_GetGain(ifc_metadata *metadata, double *gain, int *warning)=0;
	virtual int WASABICALL ReplayGainSettings_AddToHistory(double seconds, double gain)=0;

	enum
	{
		DISPATCHABLE_VERSION,
	};
};
