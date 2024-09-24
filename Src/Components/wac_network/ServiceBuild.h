#ifndef NULLSOFT_WAC_NETWORK_SERVICEBUILD_H
#define NULLSOFT_WAC_NETWORK_SERVICEBUILD_H

template <class api_T>
static void ServiceBuild( api_T *&p_api_t, GUID p_factoryGUID_t )
{
	if ( WASABI_API_SVC )
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid( p_factoryGUID_t );
		if ( factory )
			p_api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
static void ServiceRelease( api_T *p_api_t, GUID p_factoryGUID_t )
{
	if ( WASABI_API_SVC && p_api_t )
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid( p_factoryGUID_t );
		if ( factory )
			factory->releaseInterface( p_api_t );
	}

	p_api_t = NULL;
}

#endif // !NULLSOFT_WAC_NETWORK_SERVICEBUILD_H
