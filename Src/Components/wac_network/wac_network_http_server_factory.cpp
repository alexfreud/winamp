#define GUID_EQUALS_DEFINED

#include "util.h"

#include "api__wac_network.h"

#include "wac_network_http_server.h"
#include "wac_network_http_server_api.h"
#include "wac_network_http_server_factory.h"


static const std::string _serviceName = "HTTPServ Service";


FOURCC JNL_HttpServFactory::GetServiceType()
{
	return WaSvc::OBJECT;
}

const char *JNL_HttpServFactory::GetServiceName()
{
	return _serviceName.c_str();
}

GUID JNL_HttpServFactory::GetGUID()
{
	return httpservGUID;
}

void *JNL_HttpServFactory::GetInterface( int global_lock )
{
	if ( JNL::open_socketlib() )
		return NULL;

	api_httpserv *ifc = new wa::Components::WAC_Network_HTTP_Server;

	//if (global_lock)
	//	WASABI_API_SVC->service_lock(this, (void *)ifc);


	return ifc;

}

int JNL_HttpServFactory::SupportNonLockingInterface()
{
	return 1;
}

int JNL_HttpServFactory::ReleaseInterface( void *ifc )
{
	//WASABI_API_SVC->service_unlock(ifc);
	api_httpserv *httpserv     = static_cast<api_httpserv *>( ifc );
	wa::Components::WAC_Network_HTTP_Server *jnl_httpserv = static_cast<wa::Components::WAC_Network_HTTP_Server *>( httpserv );

	delete jnl_httpserv;

	JNL::close_socketlib();

	return 1;
}

const char *JNL_HttpServFactory::GetTestString()
{
	return NULL;
}

int JNL_HttpServFactory::ServiceNotify( int msg, int param1, int param2 )
{
	return 1;
}


#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS JNL_HttpServFactory
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
