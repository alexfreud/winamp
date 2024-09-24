#define WIN32_LEAN_AND_MEAN
#include "api.h"
#include <bfc/platform/export.h>
#include "../Agave/Component/ifc_wa5component.h"
#include "../nu/Singleton.h"
#include "../nu/factoryt.h"
#include "mp4_mp4v_decoder.h"
#include "mkv_mp4v_decoder.h"
#include "avi_mp4v_decoder.h"

#define MF_INIT_GUIDS
#include <Mfapi.h>
#include <Mftransform.h>
#include <wmcodecdsp.h>

api_winamp *AGAVE_API_WINAMP=0;
api_service *WASABI_API_SVC = 0;

class MP4VComponent : public ifc_wa5component
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

static ServiceFactoryT<MP4VideoDecoder, MP4VMP4Decoder> mp4Factory;
static MKVDecoderCreator mkvCreator;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoderCreator> mkvFactory;
static AVIDecoderCreator aviCreator;
static SingletonServiceFactory<svc_avidecoder, AVIDecoderCreator> aviFactory;

void MP4VComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	ServiceBuild(AGAVE_API_WINAMP, winampApiGuid);
	if (!AGAVE_API_WINAMP || AGAVE_API_WINAMP->GetRegVer() >= 1)
	{
		mp4Factory.Register(WASABI_API_SVC);
		mkvFactory.Register(WASABI_API_SVC, &mkvCreator);
		aviFactory.Register(WASABI_API_SVC, &aviCreator);
	}
}

void MP4VComponent::DeregisterServices(api_service *service)
{
	mp4Factory.Deregister(WASABI_API_SVC);
	mkvFactory.Deregister(WASABI_API_SVC);	
	aviFactory.Deregister(WASABI_API_SVC);	
}

static MP4VComponent component;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS MP4VComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
