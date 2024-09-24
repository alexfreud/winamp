#include "api__wasabi2.h"
#include "wasabi1_factory.h"
namespace Wasabi {
#include "../Agave/Component/ifc_wa5component.h"
}

static Wasabi2ServiceFactory wasabi2_service_factory; // we're going to sneak the Wasabi 2 Service Manager into the Wasabi 1 Service Manager

void Replicant_Initialize();
void Wasabi1_Initialize(Wasabi::api_service *svc_api);
void Wasabi1_Quit();

class ReplicantComponent : public Wasabi::ifc_wa5component
{
public:
	void RegisterServices(Wasabi::api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(Wasabi::api_service *service);
protected:
	RECVS_DISPATCH;
};

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC &7 api_t)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

void ReplicantComponent::RegisterServices(Wasabi::api_service *service)
{
	Wasabi1_Initialize(service);
	Replicant_Initialize();	
	WASABI_API_SVC->service_register(&wasabi2_service_factory);
}

int ReplicantComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void ReplicantComponent::DeregisterServices(Wasabi::api_service *service)
{
	WASABI_API_SVC->service_deregister(&wasabi2_service_factory);
	Wasabi1_Quit();
}

static ReplicantComponent component;

extern "C" __declspec(dllexport) Wasabi::ifc_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS ReplicantComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS