/* copyright 2008 Ben Allison */
#include "../Agave/Component/ifc_wa5component.h"
#include "factory_f263.h"
#include "flv_f263_decoder.h"
#include "mkv_f263_decoder.h"
#include "../nu/Singleton.h"

class WA5_F263 : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};

api_service *serviceManager=0;

WA5_F263 wa5_f263;
F263Factory f263Factory;
static FLVDecoderCreator flvCreator;
static SingletonServiceFactory<svc_flvdecoder, FLVDecoderCreator> flvFactory;
static MKVDecoderCreator mkvCreator;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoderCreator> mkvFactory;
void WA5_F263::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	
	WASABI_API_SVC->service_register(&f263Factory);
	flvFactory.Register(WASABI_API_SVC, &flvCreator);
	mkvFactory.Register(WASABI_API_SVC, &mkvCreator);
}

int WA5_F263::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_F263::DeregisterServices(api_service *service)
{
	service->service_deregister(&f263Factory);
	flvFactory.Deregister(WASABI_API_SVC);
	mkvFactory.Deregister(WASABI_API_SVC);
}

extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_f263;
}

#define CBCLASS WA5_F263
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS