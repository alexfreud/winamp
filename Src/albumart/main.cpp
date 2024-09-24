#define WIN32_LEAN_AND_MEAN
#include "api__albumart.h"
#include <bfc/platform/export.h>
#include "../Agave/Component/ifc_wa5component.h"
#include "../nu/Singleton.h"
#include "AlbumArt.h"

api_service *WASABI_API_SVC=0;
api_memmgr *WASABI_API_MEMMGR=0;
api_metadata *AGAVE_API_METADATA=0;
api_syscb *WASABI_API_SYSCB=0;

static AlbumArt album_art;
static SingletonServiceFactory<api_albumart, AlbumArt> album_art_factory;

class AlbumArtComponent : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC && api_t)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

void AlbumArtComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;

	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);
	ServiceBuild(WASABI_API_SYSCB, syscbApiServiceGuid);
	album_art_factory.Register(WASABI_API_SVC, &album_art);
}

int AlbumArtComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void AlbumArtComponent::DeregisterServices(api_service *service)
{
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	ServiceRelease(WASABI_API_SYSCB, syscbApiServiceGuid);
	album_art_factory.Deregister(WASABI_API_SVC);	
}

static AlbumArtComponent component;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS AlbumArtComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS