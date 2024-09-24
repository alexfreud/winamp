#include "api__bmp.h"
#include "wa5_bmp.h"
#include <bfc/platform/export.h>
#include "MyFactory.h"
#include "../nu/Singleton.h"
#include "BMPLoader.h"
#include "BMPWriter.h"
#include "avi_decoder.h"

WA5_BMP wa5_bmp;

// {BE7F448F-9107-489a-B3B0-7B1563C92BFE}
static const GUID bmpWriterGUID = 
{ 0xbe7f448f, 0x9107, 0x489a, { 0xb3, 0xb0, 0x7b, 0x15, 0x63, 0xc9, 0x2b, 0xfe } };

// {D984CD4A-9D1E-4060-A624-5BFD0BF37050}
static const GUID bmpLoaderGUID = 
{ 0xd984cd4a, 0x9d1e, 0x4060, { 0xa6, 0x24, 0x5b, 0xfd, 0xb, 0xf3, 0x70, 0x50 } };

MyFactory<BMPWriter, svc_imageWriter> bmpWriteFactory(bmpWriterGUID);
MyFactory<BMPLoader, svc_imageLoader> bmpLoadFactory(bmpLoaderGUID);

api_service *WASABI_API_SVC    = 0;
api_memmgr  *WASABI_API_MEMMGR = 0;

static AVIDecoderCreator aviCreator;
static SingletonServiceFactory<svc_avidecoder, AVIDecoderCreator> aviFactory;

void WA5_BMP::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	
	// get memory manager
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) memoryManager = reinterpret_cast<api_memmgr *>(sf->getInterface());

	WASABI_API_SVC->service_register(&bmpLoadFactory);
	WASABI_API_SVC->service_register(&bmpWriteFactory);
	aviFactory.Register(WASABI_API_SVC, &aviCreator);
}

int WA5_BMP::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_BMP::DeregisterServices(api_service *service)
{
	service->service_deregister(&bmpWriteFactory);
	service->service_deregister(&bmpLoadFactory);
	aviFactory.Deregister(WASABI_API_SVC);

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) sf->releaseInterface(memoryManager);
}

extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_bmp;
}

#define CBCLASS WA5_BMP
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS