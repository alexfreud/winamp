#include "api__mp3-mpg123.h"

#include "../nu/Singleton.h"
#include "../nu/factoryt.h"
#include "avi_mp3_decoder.h"
#include "flv_mp3_decoder.h"
#include "mkv_mp3_decoder.h"
#include "mp3_in_mp4.h"
#include "adts_mpg123.h"
#include "MP4Factory.h"
#include "NSVFactory.h"
#include <bfc/platform/export.h>
#include "../Agave/Component/ifc_wa5component.h"
#include "../nu/ServiceBuilder.h"
#include <mpg123.h>

api_service *WASABI_API_SVC    = 0;
api_config  *AGAVE_API_CONFIG  = 0;
api_memmgr  *WASABI_API_MEMMGR = 0;

class MP3DecoderComponent : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);

protected:
	RECVS_DISPATCH;
};

static SingletonServiceFactory<svc_avidecoder, AVIDecoder> avi_factory;
static AVIDecoder avi_decoder;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoder> mkv_factory;
static MKVDecoder mkv_decoder;
static SingletonServiceFactory<svc_flvdecoder, FLVDecoderCreator> flv_factory;
static FLVDecoderCreator flv_decoder;
static MPEG4Factory mp4_factory;
static ServiceFactoryT<adts, ADTS_MPG123> adts_factory;

static NSVFactory nsv_decoder;
static SingletonServiceFactory<svc_nsvFactory, NSVFactory> nsv_factory;

void MP3DecoderComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	if (mpg123_init() == MPG123_OK) {
		WASABI_API_SVC->service_register(&mp4_factory);
		avi_factory.Register(WASABI_API_SVC, &avi_decoder);
		mkv_factory.Register(WASABI_API_SVC, &mkv_decoder);
		flv_factory.Register(WASABI_API_SVC, &flv_decoder);
		nsv_factory.Register(WASABI_API_SVC, &nsv_decoder);	
		adts_factory.Register(WASABI_API_SVC);
	}
	ServiceBuild(WASABI_API_SVC, AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_SVC, WASABI_API_MEMMGR, memMgrApiServiceGuid);
}

void MP3DecoderComponent::DeregisterServices(api_service *service)
{
	avi_factory.Deregister(WASABI_API_SVC);
	mkv_factory.Deregister(WASABI_API_SVC);
	WASABI_API_SVC->service_deregister(&mp4_factory);
	flv_factory.Deregister(WASABI_API_SVC);
	nsv_factory.Deregister(WASABI_API_SVC);
	adts_factory.Deregister(WASABI_API_SVC);
	ServiceRelease(WASABI_API_SVC, AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_SVC, WASABI_API_MEMMGR, memMgrApiServiceGuid);
	mpg123_exit();
}

int MP3DecoderComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

MP3DecoderComponent mp3_decoder_component;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &mp3_decoder_component;
}

#define CBCLASS MP3DecoderComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS