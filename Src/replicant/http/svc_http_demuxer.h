#pragma once
#include "jnetlib/jnetlib_defines.h"
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "http/ifc_http_demuxer.h"

// {5E3551B0-B0FF-4997-89E4-958545C3EC19}
static const GUID demuxer_service_type_guid = 
{ 0x5E3551B0, 0xB0FF, 0x4997, { 0x89, 0xE4, 0x95, 0x85, 0x45, 0xC3, 0xEC, 0x19 } };
	
class svc_http_demuxer: public Wasabi2::Dispatchable
{
protected:
	svc_http_demuxer() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_http_demuxer() {}
public:
	static GUID GetServiceType() { return demuxer_service_type_guid; }
	/* returns types to be added to "Accept" HTTP header */
	const char *EnumerateAcceptedTypes(size_t i) { return HTTPDemuxerService_EnumerateAcceptedTypes(i); }
	/* returns a string to be added to the user-agent (e.g. Ultravox/2.1) */
	const char *GetUserAgent() { return HTTPDemuxerService_GetUserAgent(); }
	/* allows service to do any necessary customization (mainly for adding headers) */
	void CustomizeHTTP(jnl_http_t http) { HTTPDemuxerService_CustomizeHTTP(http); }

	/* if you create a demuxer, you now own http and are expected to call jnl_http_release on it when you are done */
	/* you can return NErr_TryAgain to let everyone else go first, you'll be called again with pass=1 */
	NError CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass) { return HTTPDemuxerService_CreateDemuxer(uri, http, demuxer, pass); }
	enum
	{
		DISPATCHABLE_VERSION,
	};

protected:
	virtual const char *WASABICALL HTTPDemuxerService_EnumerateAcceptedTypes(size_t i) = 0;
	virtual const char *WASABICALL HTTPDemuxerService_GetUserAgent() = 0;
	virtual void WASABICALL HTTPDemuxerService_CustomizeHTTP(jnl_http_t http) = 0;
	virtual NError WASABICALL HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass) = 0;
};
