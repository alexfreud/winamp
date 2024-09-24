#pragma once
#include "../Agave/URIHandler/svc_urihandler.h"

// {800A9A73-8891-4892-AEEB-FF970B16A39D}
static const GUID winamp_uri_handler_guid = 
{ 0x800a9a73, 0x8891, 0x4892, { 0xae, 0xeb, 0xff, 0x97, 0xb, 0x16, 0xa3, 0x9d } };


class WinampURIHandler : public svc_urihandler
{
public:
	static const char *getServiceName() { return "Winamp URI Handler"; }
	static GUID getServiceGuid() { return winamp_uri_handler_guid; } 
	int ProcessFilename(const wchar_t *filename);
	int IsMine(const wchar_t *filename); // just like ProcessFilename but don't actually process
	int EnumProtocols(size_t n, wchar_t *protocol, size_t protocolCch, wchar_t *description, size_t descriptionCch); // return 0 on success
	int RegisterProtocol(const wchar_t *protocol, const wchar_t *winampexe);
	int UnregisterProtocol(const wchar_t *protocol);

protected:
	RECVS_DISPATCH;
};