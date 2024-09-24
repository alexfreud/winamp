#include "api__xml.h"
#include "wa5_xml.h"
#include "factory_xml.h"
#include <bfc/platform/export.h>

#ifndef _DEBUG
BOOL WINAPI _DllMainCRTStartup(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hInst);
	return TRUE;
}
#endif

WA5_XML wa5_xml;
XMLFactory *xmlFactory=0;

api_service *WASABI_API_SVC = 0;

void WA5_XML::RegisterServices(api_service *service)
{
	xmlFactory = new XMLFactory;
	WASABI_API_SVC = service;
	WASABI_API_SVC->service_register(xmlFactory);  
}

int WA5_XML::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_XML::DeregisterServices(api_service *service)
{
	service->service_deregister(xmlFactory);
	delete xmlFactory;
	xmlFactory=0;
}

extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_xml;
}

#define CBCLASS WA5_XML
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS