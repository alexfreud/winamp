#include "../../Library/ml_pmp/pmp.h"
#include "api.h"
#include <api/service/waservicefactory.h>

api_albumart *AGAVE_API_ALBUMART=0;
api_language *WASABI_API_LNG = 0;
api_application *WASABI_API_APP = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_downloadManager *WAC_API_DOWNLOADMANAGER =0;
api_config *AGAVE_API_CONFIG=0;
api_devicemanager *AGAVE_API_DEVICEMANAGER = 0;
api_metadata *AGAVE_API_METADATA=0;
api_memmgr *WASABI_API_MEMMGR=0;
Wasabi2::api_service *WASABI2_API_SVC=0;

Wasabi2::api_ssdp *REPLICANT_API_SSDP=0;
extern PMPDevicePlugin plugin;

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
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild( WAC_API_DOWNLOADMANAGER, DownloadManagerGUID);
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);
	ServiceBuild(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceBuild(WASABI2_API_SVC, Wasabi2::api_service::GetServiceGUID());
	// need to have this initialised before we try to do anything with localisation features
	// TODO: WASABI_API_START_LANG(plugin.hDllInstance,InAviLangGUID);
	if (WASABI2_API_SVC)
		WASABI2_API_SVC->GetService(&REPLICANT_API_SSDP);
}

void WasabiQuit()
{
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease( WAC_API_DOWNLOADMANAGER, DownloadManagerGUID);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);
	ServiceRelease(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(WASABI2_API_SVC, Wasabi2::api_service::GetServiceGUID());	
	if (REPLICANT_API_SSDP)
		REPLICANT_API_SSDP->Release();
}