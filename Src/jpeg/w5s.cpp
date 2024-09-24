#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "api__jpeg.h"
#include "main.h"
#include "w5s.h"
#include "writer_jpg.h"
#include "loader_jpg.h"
#include "mp4_jpeg_decoder.h"
#include "avi_mjpg_decoder.h"
#include "../nu/Singleton.h"
#include "../nu/factoryt.h"
#include "amg.h"

/*
void MMXInit()
{
DWORD retval1, retval2;

__try {
_asm {
mov eax, 1  // set up CPUID to return processor version and features
// 0 = vendor string, 1 = version info, 2 = cache info
_emit 0x0f  // code bytes = 0fh,  0a2h
_emit 0xa2
mov retval1, eax
mov retval2, edx
}
} __except(EXCEPTION_EXECUTE_HANDLER) { retval1 = retval2 = 0;}

isMMX = retval1 && (retval2 & 0x800000);

}
*/

static ServiceFactoryT<MP4VideoDecoder, MP4JPEGDecoder> mp4Factory;
static ServiceFactoryT<svc_imageWriter, JpgWrite> jpegWriterFactory;
static ServiceFactoryT<svc_imageLoader, JpgLoad> jpegLoaderFactory;
static WA5_JPEG wa5_jpeg;
static AMGSucks amgSucks;
static SingletonServiceFactory<api_amgsucks, AMGSucks> amgSucksFactory;
static AVIDecoderCreator aviCreator;
static SingletonServiceFactory<svc_avidecoder, AVIDecoderCreator> aviFactory;

//bool isMMX=false;

api_service *serviceManager=0;
api_memmgr *memoryManager=0;

void WA5_JPEG::RegisterServices(api_service *service)
{
	//MMXInit();
	WASABI_API_SVC = service;

	// get memory manager
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) memoryManager = reinterpret_cast<api_memmgr *>(sf->getInterface());

	WASABI_API_SVC->service_register(&jpegLoaderFactory);
	WASABI_API_SVC->service_register(&jpegWriterFactory);
	WASABI_API_SVC->service_register(&mp4Factory);	
	amgSucksFactory.Register(WASABI_API_SVC, &amgSucks);
	aviFactory.Register(WASABI_API_SVC, &aviCreator);
}

int WA5_JPEG::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_JPEG::DeregisterServices(api_service *service)
{
	service->service_deregister(&jpegWriterFactory);
	service->service_deregister(&jpegLoaderFactory);
	service->service_deregister(&mp4Factory);
	amgSucksFactory.Deregister(service);
	aviFactory.Deregister(WASABI_API_SVC);

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) sf->releaseInterface(memoryManager);
}

extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_jpeg;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS WA5_JPEG
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;