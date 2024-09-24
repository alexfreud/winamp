#pragma once
#include "jnetlib/jnetlib_defines.h"
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "ultravox/ifc_ultravox_playback.h"
// {CAA4D831-8387-4F7D-A752-6DD0759E9C09}
static const GUID ultravox_playback_service_type_guid = 
{ 0xcaa4d831, 0x8387, 0x4f7d, { 0xa7, 0x52, 0x6d, 0xd0, 0x75, 0x9e, 0x9c, 0x9 } };


class svc_ultravox_playback : public Wasabi2::Dispatchable
{
protected:
	svc_ultravox_playback() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_ultravox_playback() {}
public:
	static GUID GetServiceType() { return ultravox_playback_service_type_guid; }

	NError CreatePlayback(jnl_http_t http, unsigned int classtype, ifc_ultravox_playback **playback) { return UltravoxPlaybackService_CreateDemuxer(http, classtype, playback); }
	enum
	{
		DISPATCHABLE_VERSION,
	};

protected:
	virtual NError WASABICALL UltravoxPlaybackService_CreateDemuxer(jnl_http_t http, unsigned int classtype, ifc_ultravox_playback **playback) = 0;
};
