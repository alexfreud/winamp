#include "api__ssdp.h"
#include "foundation/export.h"
#include "component/ifc_component.h"
#include "nx/nxuri.h"
#include "nswasabi/ReferenceCounted.h"
#include "nswasabi/singleton.h"
#include "SSDPAPI.h"

api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_syscb *WASABI2_API_SYSCB=0;
//api_android *WASABI2_API_ANDROID=0;

SSDPAPI ssdpapi;
static SingletonServiceFactory<SSDPAPI, api_ssdp> ssdp_factory;


// {F47F99AB-F90F-4623-B8BE-454555A8FC79}
static const GUID ssdp_component_guid = 
{ 0xf47f99ab, 0xf90f, 0x4623, { 0xb8, 0xbe, 0x45, 0x45, 0x55, 0xa8, 0xfc, 0x79 } };


class SSDPComponent : public ifc_component
{
public:
	SSDPComponent() : ifc_component(ssdp_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service);
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_Quit(api_service *service);
};

	
int SSDPComponent::Component_Initialize(api_service *service)
{
	WASABI2_API_SVC = service;

	int ret = jnl_init();
	if (ret != NErr_Success)
	{
		return ret;
	}
	return NErr_Success;
}

int SSDPComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	if (WASABI2_API_SVC)
	{
		WASABI2_API_SVC->GetService(&WASABI2_API_APP);
		WASABI2_API_SVC->GetService(&WASABI2_API_SYSCB);
		//WASABI2_API_SVC->GetService(&WASABI2_API_ANDROID);
	}
	
	ssdpapi.Initialize();
	ssdp_factory.Register(WASABI2_API_SVC, &ssdpapi);
	return NErr_Success;
}

void SSDPComponent::Component_DeregisterServices(api_service *service)
{
	if (WASABI2_API_SVC)
		WASABI2_API_SVC->Release();

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();

	if (WASABI2_API_SYSCB)
		WASABI2_API_SYSCB->Release();

	//if (WASABI2_API_ANDROID)
	//	WASABI2_API_ANDROID->Release();

	ssdp_factory.Deregister(WASABI2_API_SVC);
}

int SSDPComponent::Component_Quit(api_service *_service_manager)
{
	jnl_quit();
	return NErr_Success;
}

static SSDPComponent component;
extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &component;
}
