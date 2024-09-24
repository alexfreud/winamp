#include "../../Library/ml_pmp/pmp.h"
#include "api.h"
#include <api/service/waservicefactory.h>

api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
extern PMPDevicePlugin plugin;

// Metadata service
api_metadata *AGAVE_API_METADATA=0;
api_playlistmanager *WASABI_API_PLAYLISTMNGR=0;
api_albumart *AGAVE_API_ALBUMART=0;
api_memmgr *WASABI_API_MEMMGR=0;
api_application *WASABI_API_APP=0;
api_threadpool *WASABI_API_THREADPOOL=0;
api_devicemanager *AGAVE_API_DEVICEMANAGER = 0;


template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (plugin.service)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (plugin.service && api_t)
	{
		waServiceFactory *factory = plugin.service->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}

void WasabiInit()
{
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);
	ServiceBuild(WASABI_API_PLAYLISTMNGR, api_playlistmanagerGUID);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceBuild(WASABI_API_THREADPOOL, ThreadPoolGUID);
	ServiceBuild(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);
}

void WasabiQuit()
{
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_PLAYLISTMNGR, api_playlistmanagerGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	ServiceRelease(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(WASABI_API_THREADPOOL, ThreadPoolGUID);
	ServiceRelease(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);
}