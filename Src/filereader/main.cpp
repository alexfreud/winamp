#include "api__filereader.h"
#include "WA5_FileReader.h"
#ifdef _WIN32
#include "ResourceReaderFactory.h"
#endif
#include "HTTPReaderFactory.h"
#include <bfc/platform/export.h>

#ifdef _WIN32
ResourceReaderFactory resourceReaderFactory;
#endif

HTTPReaderFactory httpReaderFactory;
WA5_FileReader wa5_FileReader;
api_service *WASABI_API_SVC=0;
api_application *WASABI_API_APP=0;
api_config *AGAVE_API_CONFIG=0;

void WA5_FileReader::RegisterServices(api_service *service)
{
  WASABI_API_SVC = service;
  waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf)
		WASABI_API_APP = (api_application *)sf->getInterface();
	sf =  WASABI_API_SVC->service_getServiceByGuid(AgaveConfigGUID);
	if (sf)
		AGAVE_API_CONFIG = (api_config *)sf->getInterface();
	
  if (WASABI_API_APP && AGAVE_API_CONFIG)
    WASABI_API_SVC->service_register(&httpReaderFactory);
	
#ifdef _WIN32
  WASABI_API_SVC->service_register(&resourceReaderFactory);
#endif
}

int WA5_FileReader::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_FileReader::DeregisterServices(api_service *service)
{
  
  if (WASABI_API_APP && AGAVE_API_CONFIG)
		service->service_deregister(&httpReaderFactory);
#ifdef _WIN32
  service->service_deregister(&resourceReaderFactory);
#endif
	waServiceFactory *sf=0;    
  if (WASABI_API_APP)
  {
    sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
    if (sf)
      sf->releaseInterface(WASABI_API_APP);
  }
  
  if (AGAVE_API_CONFIG)
  {
    sf =  WASABI_API_SVC->service_getServiceByGuid(AgaveConfigGUID);
    if (sf)
      sf->releaseInterface(AGAVE_API_CONFIG);
  }
}

extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_FileReader;
}

#define CBCLASS WA5_FileReader
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
