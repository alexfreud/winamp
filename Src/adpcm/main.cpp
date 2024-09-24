#include "api.h"
#include <bfc/platform/export.h>
#include "../Agave/Component/ifc_wa5component.h"
#include "flv_adpcm_decoder.h"
#include "../nu/Singleton.h"
#include "avi_adpcm_decoder.h"

api_service *WASABI_API_SVC=0;

class ADPCMComponent : public ifc_wa5component
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

static FLVDecoderCreator flvCreator;
static SingletonServiceFactory<svc_flvdecoder, FLVDecoderCreator> flvFactory;
static SingletonServiceFactory<svc_avidecoder, AVIDecoder> avi_factory;
static AVIDecoder avi_decoder;
 
void ADPCMComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	flvFactory.Register(WASABI_API_SVC, &flvCreator);
	avi_factory.Register(WASABI_API_SVC, &avi_decoder);
}

int ADPCMComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void ADPCMComponent::DeregisterServices(api_service *service)
{
	flvFactory.Deregister(WASABI_API_SVC);
	avi_factory.Deregister(WASABI_API_SVC);
}

static ADPCMComponent component;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS ADPCMComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS