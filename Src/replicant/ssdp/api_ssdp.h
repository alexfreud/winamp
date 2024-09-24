#pragma once
#include "service/types.h"
#include "nx/nxuri.h"
#include "nx/nxstring.h"
#include "cb_ssdp.h"

// {A2EF43B6-5044-4D44-AE38-71C2CE20587B}
static const GUID ssdp_api_service_guid = 
{ 0xa2ef43b6, 0x5044, 0x4d44, { 0xae, 0x38, 0x71, 0xc2, 0xce, 0x20, 0x58, 0x7b } };

class api_ssdp : public Wasabi2::Dispatchable
{
protected:
	api_ssdp() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~api_ssdp() {}
public:
	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return ssdp_api_service_guid; }
	int RegisterService(nx_uri_t location, nx_string_t st, nx_string_t usn) { return SSDP_RegisterService(location, st, usn); }
	int RegisterCallback(cb_ssdp *callback) { return SSDP_RegisterCallback(callback); }
	int UnregisterCallback(cb_ssdp *callback) { return SSDP_UnregisterCallback(callback); }
	int Search(nx_string_t st) { return SSDP_Search(st); }
private:
	virtual int WASABICALL SSDP_RegisterService(nx_uri_t location, nx_string_t st, nx_string_t usn)=0;
	virtual int WASABICALL SSDP_RegisterCallback(cb_ssdp *callback)=0;
	virtual int WASABICALL SSDP_UnregisterCallback(cb_ssdp *callback)=0;
	virtual int WASABICALL SSDP_Search(nx_string_t st)=0;
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
};