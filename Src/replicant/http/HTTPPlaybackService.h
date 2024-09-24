#pragma once
#include "player/svc_playback.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"

// {672AF800-F239-40e5-8C87-3B4D305B72B2}
static const GUID http_playback_guid = 
{ 0x672af800, 0xf239, 0x40e5, { 0x8c, 0x87, 0x3b, 0x4d, 0x30, 0x5b, 0x72, 0xb2 } };

class HTTPPlaybackService : public svc_playback
{
public:
	WASABI_SERVICE_NAME("HTTP Playback Service");
	static GUID GetServiceGUID() { return http_playback_guid; }
	int WASABICALL PlaybackService_CreatePlayback(unsigned int pass, nx_uri_t filename, ifc_player *player, ifc_playback **out_playback_object);
};