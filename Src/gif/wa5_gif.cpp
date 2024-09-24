#include "api__gif.h"
#include "wa5_gif.h"
#include <bfc/platform/export.h>
#include "MyFactory.h"

#include "GIFLoader.h"
#include "GIFWriter.h"

WA5_GIF wa5_gif;

// {1ED6714D-2478-45dd-A803-A193EE20B75D}
static const GUID gifWriterGUID = 
{ 0x1ed6714d, 0x2478, 0x45dd, { 0xa8, 0x3, 0xa1, 0x93, 0xee, 0x20, 0xb7, 0x5d } };

// {4D724110-1E35-406f-8976-C1E9873E0C15}
static const GUID gifLoaderGUID = 
{ 0x4d724110, 0x1e35, 0x406f, { 0x89, 0x76, 0xc1, 0xe9, 0x87, 0x3e, 0xc, 0x15 } };


MyFactory<GIFWriter, svc_imageWriter> gifWriteFactory(gifWriterGUID);
MyFactory<GIFLoader, svc_imageLoader> gifLoadFactory(gifLoaderGUID);

api_service *serviceManager=0;
api_memmgr *memoryManager=0;

void WA5_GIF::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	
	// get memory manager
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) memoryManager = reinterpret_cast<api_memmgr *>(sf->getInterface());

	WASABI_API_SVC->service_register(&gifLoadFactory);
	WASABI_API_SVC->service_register(&gifWriteFactory);
}

int WA5_GIF::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_GIF::DeregisterServices(api_service *service)
{
	service->service_deregister(&gifWriteFactory);
	service->service_deregister(&gifLoadFactory);

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) sf->releaseInterface(memoryManager);
}

extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_gif;
}

#define CBCLASS WA5_GIF
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS