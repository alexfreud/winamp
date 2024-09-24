#define GUID_EQUALS_DEFINED

#include <QtGlobal>

#include "util.h"

#include "api__wac_network.h"

#include "wac_network_ssl_connection.h"
#include "wac_network_connection_factory.h"


static const std::string _serviceName = "Connection Service";
static const std::string _testString  = "wac_connection";


FOURCC JNL_ConnectionFactory::GetServiceType()
{
	return WaSvc::OBJECT;
}

const char *JNL_ConnectionFactory::GetServiceName()
{
	return _serviceName.c_str();
}

GUID JNL_ConnectionFactory::GetGUID()
{
	return connectionFactoryGUID;
}

void *JNL_ConnectionFactory::GetInterface( int global_lock )
{
	if ( JNL::open_socketlib() )
		return NULL;

	WAC_Network_Connection *conn = new WAC_Network_Connection;
	conn->set_dns( GetGlobalDNS() );

	api_connection *ifc = static_cast<api_connection *>( conn );

	//	if (global_lock)
	//		WASABI_API_SVC->service_lock(this, (void *)ifc);

	return ifc;
}

int JNL_ConnectionFactory::SupportNonLockingInterface()
{
	return 1;
}

int JNL_ConnectionFactory::ReleaseInterface( void *ifc )
{
	//WASABI_API_SVC->service_unlock(ifc);
	api_connection *connection     = static_cast<api_connection *>( ifc );
	WAC_Network_Connection *jnl_connection = static_cast<WAC_Network_Connection *>( connection );

	delete jnl_connection;

	JNL::close_socketlib();

	return 1;
}

const char *JNL_ConnectionFactory::GetTestString()
{
	return _testString.c_str();
}

int JNL_ConnectionFactory::ServiceNotify( int msg, int param1, int param2 )
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS JNL_ConnectionFactory
START_DISPATCH;
CB( WASERVICEFACTORY_GETSERVICETYPE,                GetServiceType )
CB( WASERVICEFACTORY_GETSERVICENAME,                GetServiceName )
CB( WASERVICEFACTORY_GETGUID,                       GetGUID )
CB( WASERVICEFACTORY_GETINTERFACE,                  GetInterface )
CB( WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface )
CB( WASERVICEFACTORY_RELEASEINTERFACE,              ReleaseInterface )
CB( WASERVICEFACTORY_GETTESTSTRING,                 GetTestString )
CB( WASERVICEFACTORY_SERVICENOTIFY,                 ServiceNotify )
END_DISPATCH;
