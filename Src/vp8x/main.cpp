#define WIN32_LEAN_AND_MEAN
#include "api.h"
#include <bfc/platform/export.h>
#include "../Agave/Component/ifc_wa5component.h"
#include "../nu/Singleton.h"
#include "mkv_vp8x_decoder.h"
#include "nsv_vp8_decoder.h"

api_service *WASABI_API_SVC=0;
api_memmgr *WASABI_API_MEMMGR=0;

class VP8XComponent : public ifc_wa5component
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

MKVDecoder mkv_decoder;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoder> mkv_factory;
static NSVFactory nsv_decoder;
static SingletonServiceFactory<svc_nsvFactory, NSVFactory> nsv_factory;

void VP8XComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	mkv_factory.Register(WASABI_API_SVC, &mkv_decoder);
	nsv_factory.Register(WASABI_API_SVC, &nsv_decoder);
}

int VP8XComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void VP8XComponent::DeregisterServices(api_service *service)
{
	mkv_factory.Deregister(WASABI_API_SVC);
	nsv_factory.Deregister(WASABI_API_SVC);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}

static VP8XComponent component;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS VP8XComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS