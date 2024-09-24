#ifndef NULLSOFT_COMPONENT_WAC_NETWORK_DNS_H
#define NULLSOFT_COMPONENT_WAC_NETWORK_DNS_H

#include "bfc/dispatch.h"
#include "bfc/platform/types.h"

#define API_DNS_AUTODNS ((api_dns *)-1)

enum
{
	DNS_RESOLVE_UNRESOLVABLE = -1,
	DNS_RESOLVE_SUCCESS      = 0,
	DNS_RESOLVE_WAIT         = 1,
};

enum
{
	DNS_REVERSE_UNRESOLVABLE = -1,
	DNS_REVERSE_SUCCESS      = 0,
	DNS_REVERSE_WAIT         = 1,
};

struct addrinfo;

class NOVTABLE api_dns : public Dispatchable
{
protected:
	api_dns()                                                         {}
	~api_dns()                                                        {}

public:
	DISPATCH_CODES
	{
		API_DNS_RESOLVE = 11,
		API_DNS_REVERSE = 20,
	};

	int resolve( char *hostname, short port, addrinfo **addr, int sockettype ); // see DNS_RESOLVE_* for return values
};

inline int api_dns::resolve( char *hostname, short port, addrinfo **addr, int sockettype )
{
	return _call( API_DNS_RESOLVE, (int)DNS_RESOLVE_UNRESOLVABLE, hostname, port, addr, sockettype );
}


// {F0435E72-5A1A-4d57-A9E2-3FDC421C1010}
static const GUID dnsFactoryGUID =
{ 0xf0435e72, 0x5a1a, 0x4d57, { 0xa9, 0xe2, 0x3f, 0xdc, 0x42, 0x1c, 0x10, 0x10 } };


#endif // !NULLSOFT_COMPONENT_WAC_NETWORK_DNS_H