#include "api.h"
#include <bfc/platform/export.h>
#include "../Agave/Component/ifc_wa5component.h"
#include "../nu/Singleton.h"
#include "NSVFactory.h"
#include "duck_dxl.h"
#include "flv_vp6_decoder.h"
#include "avi_vp6_decoder.h"

api_service *WASABI_API_SVC=0;
api_memmgr *WASABI_API_MEMMGR=0;

class VP6Component : public ifc_wa5component
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

static AVIDecoderCreator aviCreator;
static SingletonServiceFactory<svc_avidecoder, AVIDecoderCreator> aviFactory;
FLVDecoderCreator flvCreator;
SingletonServiceFactory<svc_flvdecoder, FLVDecoderCreator> flvFactory;
static NSVFactory nsvFactory;
static SingletonServiceFactory<svc_nsvFactory, NSVFactory> factory;
 
extern "C"
{
    int vp60_Init(void);
    int vp60_Exit(void);
}

void VP6Component::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	factory.Register(WASABI_API_SVC, &nsvFactory);
	flvFactory.Register(WASABI_API_SVC, &flvCreator);
	aviFactory.Register(WASABI_API_SVC, &aviCreator);	

	/* since we're delay loaded via WBM, it's safe to do this at load time */
	DXL_InitVideo();
	vp60_Init();
}

int VP6Component::RegisterServicesSafeModeOk()
{
	return 1;
}

void VP6Component::DeregisterServices(api_service *service)
{
	factory.Deregister(WASABI_API_SVC);
	flvFactory.Deregister(WASABI_API_SVC);
	aviFactory.Deregister(WASABI_API_SVC);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);

	vp60_Exit();
	DXL_ExitVideo();
}

static VP6Component component;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS VP6Component
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS