#include "factory_vlbDecoder.h"
#include "factory_nsvVlb.h"
#include "api__vlb.h"
#include <bfc/platform/export.h>

#include "../Agave/Component/ifc_wa5component.h"

class VLBComponent : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
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
	if (WASABI_API_SVC && api_t)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

VLBDecoderFactory vlbDecoderFactory;
NSVVLBFactory nsvDecoderFactory;
VLBComponent vlbComponent;
api_service *WASABI_API_SVC=0;
api_memmgr *WASABI_API_MEMMGR = 0;

void VLBComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	WASABI_API_SVC->service_register(&vlbDecoderFactory);  
	WASABI_API_SVC->service_register(&nsvDecoderFactory);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}

int VLBComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void VLBComponent::DeregisterServices(api_service *service)
{
	service->service_deregister(&vlbDecoderFactory);
	service->service_deregister(&nsvDecoderFactory);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}

extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &vlbComponent;
}

#define CBCLASS VLBComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
