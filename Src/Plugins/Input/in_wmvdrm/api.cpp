#include "main.h"
#include "api.h"
#include <windows.h>
#include "../Winamp/wa_ipc.h"
#include "FactoryHelper.h"
#include "MetaTagFactory.h"
#include "factory_Handler.h"
#include "AlbumArt.h"
#include "RawReader.h"
#include "../nu/Singleton.h"
MetaTagFactory metaTagFactory;

api_service *serviceManager = 0;
api_playlistmanager *playlistManager=0;
api_config *AGAVE_API_CONFIG=0;
api_application *applicationApi=0;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_memmgr *WASABI_API_MEMMGR = 0;
WPLHandlerFactory wplHandlerFactory;
ASXHandlerFactory asxHandlerFactory;
AlbumArtFactory albumArtFactory;
static RawMediaReaderService raw_media_reader_service;
static SingletonServiceFactory<svc_raw_media_reader, RawMediaReaderService> raw_factory;

int LoadWasabi()
{
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(playlistManager, api_playlistmanagerGUID);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	plugin.service->service_register(&metaTagFactory);
	plugin.service->service_register(&wplHandlerFactory);
	plugin.service->service_register(&asxHandlerFactory);
	plugin.service->service_register(&albumArtFactory);
	raw_factory.Register(plugin.service, &raw_media_reader_service);

	return TRUE;
}

void UnloadWasabi()
{
	plugin.service->service_deregister(&metaTagFactory);
	plugin.service->service_deregister(&wplHandlerFactory);
	plugin.service->service_deregister(&asxHandlerFactory);
	plugin.service->service_deregister(&albumArtFactory);
	plugin.service->service_deregister(&raw_factory);
	ServiceRelease(playlistManager, api_playlistmanagerGUID);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}