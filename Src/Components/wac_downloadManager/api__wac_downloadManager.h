#ifndef NULLSOFT_WAC_DOWNLOAD_MANAGER_API_H
#define NULLSOFT_WAC_DOWNLOAD_MANAGER_API_H

#include "api/service/api_service.h"

#include "api/application/api_application.h"
#define WASABI_API_APP applicationApi

#include "../Agave/Config/api_config.h"

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi


template <class api_T>
static void ServiceBuild( api_T *&api_t, GUID factoryGUID_t )
{
	if ( WASABI_API_SVC )
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid( factoryGUID_t );
		if ( factory )
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
static void ServiceRelease( api_T *api_t, GUID factoryGUID_t )
{
	if ( WASABI_API_SVC && api_t )
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid( factoryGUID_t );
		if ( factory )
			factory->releaseInterface( api_t );
	}

	api_t = NULL;
}

#endif  // !NULLSOFT_WAC_DOWNLOAD_MANAGER_API_H