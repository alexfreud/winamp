#ifndef NULLSOFT_FACTORY_WAC_NETWORK_SSLCONNECTION_H
#define NULLSOFT_FACTORY_WAC_NETWORK_SSLCONNECTION_H

#include <string>

#include "api/service/waservicefactory.h"
#include "api/service/services.h"

class JNL_SSL_ConnectionFactory : public waServiceFactory
{
	public:
	FOURCC       GetServiceType();
	const char *GetServiceName();
	GUID         GetGUID();
	const char *GetTestString();

	void *GetInterface( int global_lock );
	int          ReleaseInterface( void *ifc );

	int          SupportNonLockingInterface();
	int          ServiceNotify( int msg, int param1, int param2 );

	protected:
	RECVS_DISPATCH;
};

#endif  // !NULLSOFT_FACTORY_WAC_NETWORK_SSLCONNECTION_H
