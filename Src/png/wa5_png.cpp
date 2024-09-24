#include "api__png.h"
#include "wa5_png.h"
#include "factory_png.h"
#include "factory_pngwrite.h"
#include <bfc/platform/export.h>

WA5_PNG wa5_png;
PNGFactory pngFactory;
PNGWriteFactory pngWriteFactory;

api_service *serviceManager=0;
api_memmgr *memoryManager=0;

void WA5_PNG::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	
	// get memory manager
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) memoryManager = reinterpret_cast<api_memmgr *>(sf->getInterface());
	
	WASABI_API_SVC->service_register(&pngFactory);
	WASABI_API_SVC->service_register(&pngWriteFactory);
}

int WA5_PNG::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_PNG::DeregisterServices(api_service *service)
{
	service->service_deregister(&pngWriteFactory);
	service->service_deregister(&pngFactory);

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(memMgrApiServiceGuid);
	if (sf) sf->releaseInterface(memoryManager);
}

extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_png;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS WA5_PNG
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;