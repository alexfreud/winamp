#pragma once
#include "jnetlib/jnetlib_defines.h"
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "icy/ifc_icy_playback.h"


// {42E078D5-7D68-43b5-9AFD-0135558C799F}
static const GUID icy_playback_service_type_guid = 
{ 0x42e078d5, 0x7d68, 0x43b5, { 0x9a, 0xfd, 0x1, 0x35, 0x55, 0x8c, 0x79, 0x9f } };
	
class svc_icy_playback : public Wasabi2::Dispatchable
{
protected:
	svc_icy_playback() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_icy_playback() {}
public:
	static GUID GetServiceType() { return icy_playback_service_type_guid; }
	/* returns types to be added to "Accept" HTTP header */
	const char *EnumerateAcceptedTypes(size_t i) { return ICYPlaybackService_EnumerateAcceptedTypes(i); }

	NError CreatePlayback(jnl_http_t http, ifc_icy_playback **playback, int pass) { return ICYPlaybackService_CreateDemuxer(http, playback, pass); }
	enum
	{
		DISPATCHABLE_VERSION,
	};

protected:
	virtual const char *WASABICALL ICYPlaybackService_EnumerateAcceptedTypes(size_t i) = 0;
	virtual NError WASABICALL ICYPlaybackService_CreateDemuxer(jnl_http_t http, ifc_icy_playback **playback, int pass) = 0;
};
