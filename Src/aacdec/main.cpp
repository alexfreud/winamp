#include "api.h"

#include "../nu/Singleton.h"
#include "../nu/factoryt.h"
#include "AVIAACDecoder.h"
#include "FLVAACDecoder.h"
#include "MKVAACDecoder.h"
#include "MP4AACDecoder.h"
#include "ADTSAACDecoder.h"
#include "NSVAACDecoder.h"
#include <bfc/platform/export.h>
#include "../Agave/Component/ifc_wa5component.h"

class AACDecoderComponent : public ifc_wa5component
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
			api_t = (api_T *)factory->getInterface();
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
	api_t = 0;
}

static SingletonServiceFactory<svc_avidecoder, AVIDecoder> avi_factory;
static AVIDecoder avi_decoder;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoder> mkv_factory;
static MKVDecoder mkv_decoder;
static SingletonServiceFactory<svc_flvdecoder, FLVDecoder> flv_factory;
static FLVDecoder flv_decoder;
static ServiceFactoryT<MP4AudioDecoder, MP4AACDecoder> mp4_factory;
static ServiceFactoryT<adts, ADTSAACDecoder> adts_factory;

static NSVDecoder nsv_decoder;
static SingletonServiceFactory<svc_nsvFactory, NSVDecoder> nsv_factory;

void AACDecoderComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	avi_factory.Register(WASABI_API_SVC, &avi_decoder);
	mkv_factory.Register(WASABI_API_SVC, &mkv_decoder);
	flv_factory.Register(WASABI_API_SVC, &flv_decoder);
	nsv_factory.Register(WASABI_API_SVC, &nsv_decoder);
	mp4_factory.Register(WASABI_API_SVC);
	adts_factory.Register(WASABI_API_SVC);
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}

void AACDecoderComponent::DeregisterServices(api_service *service)
{
	avi_factory.Deregister(WASABI_API_SVC);
	mkv_factory.Deregister(WASABI_API_SVC);
	mp4_factory.Deregister(WASABI_API_SVC);
	flv_factory.Deregister(WASABI_API_SVC);
	nsv_factory.Deregister(WASABI_API_SVC);
	adts_factory.Deregister(WASABI_API_SVC);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}

int AACDecoderComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

AACDecoderComponent aac_decoder_component;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &aac_decoder_component;
}

#define CBCLASS AACDecoderComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS