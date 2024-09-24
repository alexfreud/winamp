
#pragma once
#ifndef NULLSOFT_UTILITY_SERVICE_BUILDER_H
#define NULLSOFT_UTILITY_SERVICE_BUILDER_H

#include <api/service/waservicefactoryi.h>
#include <api/service/services.h>

#ifndef WASABI_API_SVC
#define WASABI_API_SVC serviceManager
#endif

template <class api_T>
static void ServiceBuild(api_service *service, api_T *&api_t, GUID factoryGUID_t)
{
	if (service)
	{
		waServiceFactory *factory = service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
		{
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
		}
	}
}

template <class api_T>
static void ServiceRelease(api_service *service, api_T *&api_t, GUID factoryGUID_t)
{
	if (service && api_t)
	{
		waServiceFactory *factory = service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
		{
			factory->releaseInterface(api_t);
		}
	}
	api_t = NULL;
}

#endif
