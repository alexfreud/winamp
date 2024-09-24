#pragma once

#include "../Agave/URIHandler/svc_urihandler.h"

// {DA5F8547-5D53-49d9-B9EC-C59318D11798}
static const GUID pcast_uri_handler_guid = 
{ 0xda5f8547, 0x5d53, 0x49d9, { 0xb9, 0xec, 0xc5, 0x93, 0x18, 0xd1, 0x17, 0x98 } };


class PCastURIHandler :
	public svc_urihandler
{
public:
	static const char *getServiceName() { return "PCast URI Handler"; }
	static GUID getServiceGuid() { return pcast_uri_handler_guid; } 
	int ProcessFilename(const wchar_t *filename);
	int IsMine(const wchar_t *filename); // just like ProcessFilename but don't actually process

protected:
	RECVS_DISPATCH;
};
