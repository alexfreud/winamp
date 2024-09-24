#pragma once
#include "foundation/dispatch.h"
#include "audio/ifc_audioout.h"
#include "player/ifc_playback_parameters.h"

class ifc_player;

// {FB5E9AE3-E033-407C-942B-6C1BFAF52A5C}
static const GUID output_service_guid = 
{ 0xfb5e9ae3, 0xe033, 0x407c, { 0x94, 0x2b, 0x6c, 0x1b, 0xfa, 0xf5, 0x2a, 0x5c } };

class NOVTABLE svc_output : public Wasabi2::Dispatchable
{
protected:
	svc_output() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_output() {}
public:
	static GUID GetServiceType() { return output_service_guid; }
	/* ----- Audio Output ----- */
	// Opens winamp2-style Audio Output - good for audio-only streams (buffered, push, output-plugin-defined buffersize)
	int AudioOpen(const ifc_audioout::Parameters *format, ifc_player *player, ifc_playback_parameters *secondary_parameters, ifc_audioout **out_output) { return OutputService_AudioOpen(format, player, secondary_parameters, out_output); }

	/* ----- Video Output ----- */
	int VideoOpen();

	/* ----- Text Output ----- */
	// Opens a subtitle stream
	int TextOpenSubtitle(); 
	// Opens a video info text stream
	int TextOpenInfo();
	// Opens a lyrics text stream
	int TextOpenLyrics();

	enum
	{
		DISPATCHABLE_VERSION,
	};
protected:
	virtual int WASABICALL OutputService_AudioOpen(const ifc_audioout::Parameters *format, ifc_player *player, ifc_playback_parameters *secondary_parameters, ifc_audioout **out_output) = 0;	
};
