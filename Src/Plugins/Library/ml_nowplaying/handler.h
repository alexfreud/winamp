#pragma once

#include "../Agave/URIHandler/svc_urihandler.h"

// {7A8BAF83-3995-4550-B15B-12D297A9220E}
static const GUID ml_nowplaying_uri_handler = 
{ 0x7a8baf83, 0x3995, 0x4550, { 0xb1, 0x5b, 0x12, 0xd2, 0x97, 0xa9, 0x22, 0xe } };

class NowPlayingURIHandler : public svc_urihandler
{
public:
	static const char *getServiceName() { return "Now Playing URI Handler"; }
	static GUID getServiceGuid() { return ml_nowplaying_uri_handler; } 
	int ProcessFilename(const wchar_t *filename);
	int IsMine(const wchar_t *filename); // just like ProcessFilename but don't actually process

protected:
	RECVS_DISPATCH;
};