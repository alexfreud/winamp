#include "api__tagz.h"
#include "api_tagz.h"
#include "../Agave/Component/ifc_wa5component.h"
#include "factory_tagz.h"

TagzFactory tagsFactory;

class TagzComponent : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);

protected:
	RECVS_DISPATCH;
};

TagzComponent tagzComponent;
api_service *serviceManager=0;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static HINSTANCE GetMyInstance()
{
	MEMORY_BASIC_INFORMATION mbi = {0};
	if(VirtualQuery(GetMyInstance, &mbi, sizeof(mbi)))
		return (HINSTANCE)mbi.AllocationBase;
	return NULL;
}

void TagzComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	WASABI_API_SVC->service_register(&tagsFactory);

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(GetMyInstance(),tagzLangGUID);
}

void TagzComponent::DeregisterServices(api_service *service)
{
	service->service_deregister(&tagsFactory);
}

int TagzComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &tagzComponent;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS TagzComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;