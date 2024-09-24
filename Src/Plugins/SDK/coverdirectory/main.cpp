#include "api.h"
#include "../Agave/Component/ifc_wa5component.h"
#include "CoverDirectory.h"

class CoverDirectoryComponent : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};

api_service *WASABI_API_SVC=0;
api_application *WASABI_API_APP =0;
api_config *AGAVE_API_CONFIG = 0;
api_memmgr *WASABI_API_MEMMGR=0;
api_metadata *AGAVE_API_METADATA = 0;
static AlbumArtFactory albumArtFactory;

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = (api_T *)factory->getInterface();
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

void CoverDirectoryComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);
	WASABI_API_SVC->service_register(&albumArtFactory);
	/* uncomment if we need it 
	WASABI_API_LNG = GetService<api_language>(languageApiGUID);
	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(hModule,playlistLangGUID);
	*/
}

void CoverDirectoryComponent::DeregisterServices(api_service *service)
{
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	WASABI_API_SVC->service_deregister(&albumArtFactory);
	/* uncomment if we need it 
	ReleaseService(languageApiGUID, WASABI_API_LNG);
	*/
}

static CoverDirectoryComponent coverDirectoryComponent;

extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &coverDirectoryComponent;
}

#define CBCLASS CoverDirectoryComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
