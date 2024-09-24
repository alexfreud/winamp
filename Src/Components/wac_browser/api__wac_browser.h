#ifndef NULLSOFT_COMPONENT_WAC_BROWSER_H
#define NULLSOFT_COMPONENT_WAC_BROWSER_H

#include "api/service/api_service.h"
#include "../Agave/Config/api_config.h"

template <class api_T>
void ServiceBuild( api_T *&p_api_t, GUID p_factoryGUID_t )
{
	if ( WASABI_API_SVC )
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid( p_factoryGUID_t );
		if ( factory )
			p_api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
void ServiceRelease( api_T *p_api_t, GUID p_factoryGUID_t )
{
	if ( WASABI_API_SVC && p_api_t )
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid( p_factoryGUID_t );
		if ( factory )
			factory->releaseInterface( p_api_t );
	}

	p_api_t = NULL;
}

#endif // !NULLSOFT_COMPONENT_WAC_BROWSER_H

