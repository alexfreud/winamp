#pragma once

#include "../Agave/URIHandler/svc_urihandler.h"

// {1E8830FA-0BDA-45fa-B106-BDF56C93BADD}
static const GUID ml_online_uri_handler = 
{ 0x1e8830fa, 0xbda, 0x45fa, { 0xb1, 0x6, 0xbd, 0xf5, 0x6c, 0x93, 0xba, 0xdd } };


class OnlineServicesURIHandler : public svc_urihandler
{
public:
	static const char *getServiceName() { return "Online Services URI Handler"; }
	static GUID getServiceGuid() { return ml_online_uri_handler; } 
	int ProcessFilename(const wchar_t *filename);
	int IsMine(const wchar_t *filename); // just like ProcessFilename but don't actually process

protected:
	RECVS_DISPATCH;
};