#include "api.h"
#include "jnetlib/jnetlib.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "HTTPPlaybackService.h"

static SingletonService<HTTPPlaybackService, svc_playback> playback_factory;

// {446BFBF6-8CE9-4697-844E-8386B5037685}
static const GUID http_component_guid = 
{ 0x446bfbf6, 0x8ce9, 0x4697, { 0x84, 0x4e, 0x83, 0x86, 0xb5, 0x3, 0x76, 0x85 } };


class HTTPComponent : public ifc_component
{
public:
	HTTPComponent() : ifc_component(http_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service);
	int WASABICALL Component_RegisterServices(api_service *service);

	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_Quit(api_service *_service_manager);
};

static HTTPComponent http_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;

int HTTPComponent::Component_Initialize(api_service *service)
{
	int ret = jnl_init();
	if (ret != NErr_Success)
		return ret;

	return NErr_Success;
}

int HTTPComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	
	// get application API
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);

	playback_factory.Register(WASABI2_API_SVC);
	return NErr_Success;
}

void HTTPComponent::Component_DeregisterServices(api_service *service)
{
	playback_factory.Deregister(WASABI2_API_SVC);

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
}

int HTTPComponent::Component_Quit(api_service *_service_manager)
{
	jnl_quit();
	return NErr_Success;
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &http_component;
}
