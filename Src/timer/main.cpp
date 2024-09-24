#include "api.h"
#include "../nu/ServiceWatcher.h"
#include "../Agave/Component/ifc_wa5component.h"
#include <api/service/waServiceFactory.h>
#include "Factory.h"
#include "ScriptFactory.h"

Factory factory;
static ScriptFactory scriptFactory;
api_config *WASABI_API_CONFIG = 0;
api_syscb *WASABI_API_SYSCB = 0;
api_service *WASABI_API_SVC = 0;
api_maki *WASABI_API_MAKI = 0;

static ServiceWatcher serviceWatcher;

class TimerComponent : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};


template <class api_t>
api_t *GetService(GUID serviceGUID)
{	
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(serviceGUID);
	if (sf)
		return (api_t *)sf->getInterface();
	else
		return 0;
}

inline void ReleaseService(GUID serviceGUID, void *service)
{
	if (service)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(serviceGUID);
		if (sf)
			sf->releaseInterface(service);
	}
}

void TimerComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	WASABI_API_SYSCB = GetService<api_syscb>(syscbApiServiceGuid);
	serviceWatcher.WatchWith(WASABI_API_SVC);
	serviceWatcher.WatchFor(&WASABI_API_CONFIG, configApiServiceGuid);
	serviceWatcher.WatchFor(&WASABI_API_MAKI, makiApiServiceGuid);
	service->service_register(&factory);
	service->service_register(&scriptFactory);	

	// register for service callbacks in case any of these don't exist yet
	WASABI_API_SYSCB->syscb_registerCallback(&serviceWatcher);
}

int TimerComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void TimerComponent::DeregisterServices(api_service *service)
{
	service->service_deregister(&factory);
	service->service_deregister(&scriptFactory);
	serviceWatcher.StopWatching();
	serviceWatcher.Clear();

	ReleaseService(configApiServiceGuid,	WASABI_API_CONFIG);
	ReleaseService(syscbApiServiceGuid,	WASABI_API_SYSCB);
	ReleaseService(makiApiServiceGuid,	WASABI_API_MAKI);
	factory.Stop();
}

TimerComponent timerComponent;
extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &timerComponent;
}

#define CBCLASS TimerComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS