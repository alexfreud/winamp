#pragma once
#include "foundation/dispatch.h"

class cb_ssdp : public Wasabi2::Dispatchable
{
protected:
	cb_ssdp() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~cb_ssdp() {}

public:
	void OnServiceConnected(nx_uri_t location, nx_string_t type, nx_string_t usn) { return SSDPCallback_OnServiceConnected(location, type, usn); }
	void OnServiceDisconnected(nx_string_t usn) { return SSDPCallback_OnServiceDisconnected(usn); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual void WASABICALL SSDPCallback_OnServiceConnected(nx_uri_t location, nx_string_t type, nx_string_t usn)=0;
	virtual void WASABICALL SSDPCallback_OnServiceDisconnected(nx_string_t usn)=0;
};