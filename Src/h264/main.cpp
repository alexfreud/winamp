#define WIN32_LEAN_AND_MEAN
#include "api__h264.h"
#include <bfc/platform/export.h>
#include "../Agave/Component/ifc_wa5component.h"
#include "../nu/Singleton.h"
#include "../nu/factoryt.h"
#include "h264_flv_decoder.h"
#include "h264_mp4_decoder.h"
#include "h264_mkv_decoder.h"
#include "avi_h264_decoder.h"
#include "NSVFactory.h"

api_service *WASABI_API_SVC=0;
api_memmgr *WASABI_API_MEMMGR=0;
api_winamp *AGAVE_API_WINAMP=0;

class H264Component : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
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
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

static FLVDecoderCreator flvCreator;
static SingletonServiceFactory<svc_flvdecoder, FLVDecoderCreator> flvFactory;
static MKVDecoderCreator mkvCreator;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoderCreator> mkvFactory;
static AVIDecoderCreator aviCreator;
static SingletonServiceFactory<svc_avidecoder, AVIDecoderCreator> aviFactory;
static ServiceFactoryT<MP4VideoDecoder, H264MP4Decoder> mp4Factory;
static NSVFactory nsvCreator;
static SingletonServiceFactory<svc_nsvFactory, NSVFactory> nsvFactory;

void H264Component::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	ServiceBuild(AGAVE_API_WINAMP, winampApiGuid);
	if (!AGAVE_API_WINAMP || AGAVE_API_WINAMP->GetRegVer() >= 1)
	{
		ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
		mp4Factory.Register(WASABI_API_SVC);
		flvFactory.Register(WASABI_API_SVC, &flvCreator);
		mkvFactory.Register(WASABI_API_SVC, &mkvCreator);
		aviFactory.Register(WASABI_API_SVC, &aviCreator);
		nsvFactory.Register(WASABI_API_SVC, &nsvCreator);
	}
}

void H264Component::DeregisterServices(api_service *service)
{
	mp4Factory.Deregister(WASABI_API_SVC);
	flvFactory.Deregister(WASABI_API_SVC);
	mkvFactory.Deregister(WASABI_API_SVC);	
	aviFactory.Deregister(WASABI_API_SVC);	
	nsvFactory.Deregister(WASABI_API_SVC);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(AGAVE_API_WINAMP, winampApiGuid);
}

static H264Component component;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS H264Component
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
