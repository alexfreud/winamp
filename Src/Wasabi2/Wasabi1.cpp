#include "api__wasabi2.h"
#include "../Winamp/wa_ipc.h"

namespace Wasabi {
#include <api/service/waservicefactory.h>
#include "../nu/ServiceWatcher.h"
}

Wasabi::ServiceWatcher serviceWatcher;

template <class api_T>
static void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		Wasabi::waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
static void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC && api_t)
	{
		Wasabi::waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

Wasabi::api_service *WASABI_API_SVC=0;
Wasabi::api_application *WASABI_API_APP=0;
Wasabi::api_memmgr *WASABI_API_MEMMGR=0;
Wasabi::api_albumart *AGAVE_API_ALBUMART=0;


void Wasabi1_Initialize(Wasabi::api_service *svc_api)
{
	WASABI_API_SVC = svc_api;
	ServiceBuild(WASABI_API_APP, Wasabi::applicationApiServiceGuid);
	ServiceBuild(AGAVE_API_ALBUMART, Wasabi::albumArtGUID);	
	ServiceBuild(WASABI_API_MEMMGR, Wasabi::memMgrApiServiceGuid);	
	
	serviceWatcher.WatchWith(WASABI_API_SVC);
	serviceWatcher.WatchFor(&AGAVE_API_ALBUMART, Wasabi::albumArtGUID);
}

void Wasabi1_Quit()
{
	serviceWatcher.StopWatching();
	serviceWatcher.Clear();

	ServiceRelease(WASABI_API_APP, Wasabi::applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_ALBUMART, Wasabi::albumArtGUID);
	ServiceRelease(WASABI_API_MEMMGR, Wasabi::memMgrApiServiceGuid);	
}

namespace Wasabi {
#include "../nu/ServiceWatcher.cpp"
}