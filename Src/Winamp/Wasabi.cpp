/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description:
** Author: Ben Allison benski@nullsoft.com
** Created:
**/
#include "Main.h"
#include <api.h>

#include "VideoFeedFactory.h"
#include "TagProvider.h"
#include "Singleton.h"
#include "Random.h"
#include "DecodeFile.h"
#include "Language.h"
#include "ExplorerFindFile.h"
#include "../config/config.h"
#include "JSAPI2_Security.h"
#include "GammaManagerAPI.h"
#include "PaletteManager.h"
#include "../nu/ServiceWatcher.h"
#include "JSAPI2_Creator.h"
#include "stats.h"
#include "../nu/Singleton.h"
#include "handler.h"

ServiceWatcher serviceWatcher;

api_playlistmanager *playlistManager=0;
api_playlists *AGAVE_API_PLAYLISTS=0;
api_language *WASABI_API_LNG=0;
api_downloadManager *WAC_API_DOWNLOADMANAGER=0;
api_skin *WASABI_API_SKIN=0;
api_amgsucks *AGAVE_API_AMGSUCKS=0;
api_albumart *AGAVE_API_ALBUMART=0;
// ----- Services -----
api_service *serviceManager = 0;
VideoTextFeed *videoTextFeed = 0;
PlaylistTextFeed *playlistTextFeed = 0;
TagProvider *tagProvider = 0;

SysCallbacks *sysCallbacks = 0;
MemoryManager *memoryManager = 0;
GammaManagerAPI *gammaManager = 0;
PaletteManager *paletteManager = 0;
Metadata *metadata = 0;
Application *application = 0;
DecodeFile *decodeFile = 0;
URLManager *AGAVE_API_URLMANAGER = 0;
ThreadPool *WASABI_API_THREADPOOL = 0;
WinampApi *WASABI_API_WINAMP = 0;

Random random;
WinampURIHandler winamp_uri_handler;
Language *langManager = 0;
ExplorerFindFile *explorerFindFileManager = 0;


// ----- Service Factories -----
static VideoTextFeedFactory *videoTextFeedFactory = 0;
static PlaylistTextFeedFactory *playlistTextFeedFactory = 0;
//TagProviderFactory *tagProviderFactory = 0;

static Singleton2<api_syscb, SysCallbacks> syscbsvc;
static Singleton2<api_config, Config> configSvc;
Config config;
static Singleton2<api_stats, Stats> statsSvc;
// this class is using the RAII concept (resource allocation is initialization)
// it registers services in its constructor, and deregisters in the destructor
class WinampServices
{
public:
	WinampServices() :
		memoryManagerFactory(memoryManager),
		metadataFactory(metadata),
		applicationFactory(application),
		randomFactory(&random, true),
		decodeFileFactory(decodeFile),
		languageFactory(langManager),
		ExplorerFindFileFactory(explorerFindFileManager),
		urlManagerFactory(AGAVE_API_URLMANAGER),
		jsapi2_securityFactory(&JSAPI2::security, true),
		gammaManagerFactory(gammaManager),
		paletteManagerFactory(paletteManager),
		threadPoolFactory(WASABI_API_THREADPOOL),
		winampFactory(WASABI_API_WINAMP)
	{
		WASABI_API_SVC->service_register(&jsapi2_creator_factory);
		WASABI_API_LNG = langManager;
		uriHandlerFactory.Register(WASABI_API_SVC, &winamp_uri_handler);
	}
	
	~WinampServices()
	{
		WASABI_API_SVC->service_deregister(&jsapi2_creator_factory);
		uriHandlerFactory.Deregister(WASABI_API_SVC);
	}
	
	Singleton<api_memmgr, MemoryManager>              memoryManagerFactory;
	Singleton<api_metadata, Metadata>                 metadataFactory;
	Singleton<api_application, Application>           applicationFactory;
	Singleton<api_random, Random>                     randomFactory;
	Singleton<api_decodefile, DecodeFile>             decodeFileFactory;
	Singleton<api_language, Language>                 languageFactory;
	Singleton<api_explorerfindfile, ExplorerFindFile> ExplorerFindFileFactory;
	Singleton<api_urlmanager, URLManager>             urlManagerFactory;
	Singleton<JSAPI2::api_security, JSAPI2::Security> jsapi2_securityFactory;
	Singleton<api_colorthemes, GammaManagerAPI>       gammaManagerFactory;
	Singleton<api_palette, PaletteManager>            paletteManagerFactory;
	Singleton<api_threadpool, ThreadPool>             threadPoolFactory;
	Singleton<api_winamp, WinampApi>                  winampFactory;
	
	SingletonServiceFactory<svc_urihandler, WinampURIHandler> uriHandlerFactory;
	JSAPI2CreatorFactory jsapi2_creator_factory;
};

WinampServices *services=0;
namespace Wasabi
{
	bool loaded = false;
}

void Wasabi_Load()
{
	if (!Wasabi::loaded)
	{
		RegisterConfigGroups(); // TODO: this isn't the best place to set up this config stuff, but it's going here now 'cause it's convienent

		serviceManager = new ServiceManager;
		syscbsvc.RegisterNew(sysCallbacks);

		configSvc.Register(&config);
		statsSvc.Register(&stats);

		services = new WinampServices();

		videoTextFeed = new VideoTextFeed;
		serviceManager->service_register(videoTextFeedFactory = new VideoTextFeedFactory);

		playlistTextFeed = new PlaylistTextFeed;
		serviceManager->service_register(playlistTextFeedFactory = new PlaylistTextFeedFactory);

		tagProvider = new TagProvider;
		//serviceManager->service_register(tagProviderFactory = new TagProviderFactory);

		// watch for services that might be loaded later by plugins
		serviceWatcher.WatchWith(serviceManager);
		serviceWatcher.WatchFor(&WASABI_API_SKIN, skinApiServiceGuid);
		// register for service callbacks in case any of these don't exist yet
		WASABI_API_SYSCB->syscb_registerCallback(&serviceWatcher);

		Wasabi::loaded = true;
	}
}

void Wasabi_Unload()
{
	if (Wasabi::loaded)
	{
		//delete textFeeds;
		//textFeeds=0;

		serviceWatcher.StopWatching();
		serviceWatcher.Clear();

		// serviceManager->service_deregister(tagProviderFactory);
		serviceManager->service_deregister(videoTextFeedFactory);
		serviceManager->service_deregister(playlistTextFeedFactory);

		// delete tagProviderFactory; tagProviderFactory = 0;
		delete videoTextFeedFactory;
		videoTextFeedFactory = 0;
		
		delete playlistTextFeedFactory;
		playlistTextFeedFactory = 0;

		delete tagProvider;
		tagProvider = 0;
		
		delete videoTextFeed;
		videoTextFeed = 0;
		
		delete playlistTextFeed;
		playlistTextFeed = 0;

		WASABI_API_THREADPOOL->Kill();

		delete services;
		
		configSvc.Deregister();
		statsSvc.Deregister();
		syscbsvc.Deregister();
		
		// delete this last
		delete static_cast<ServiceManager *>(serviceManager);
		serviceManager = 0;

		Wasabi::loaded = false;
	}
}

// other people's services that we might be interested in
api_tagz *WINAMP5_API_TAGZ = 0;

void Wasabi_FindSystemServices()
{
	waServiceFactory *sf;
	sf = WASABI_API_SVC->service_getServiceByGuid(tagzGUID);
	if (sf)
		WINAMP5_API_TAGZ = (api_tagz *)sf->getInterface();

	sf = WASABI_API_SVC->service_getServiceByGuid(api_playlistmanagerGUID);
	if (sf)
		AGAVE_API_PLAYLISTMANAGER = (api_playlistmanager *)sf->getInterface();

	sf = WASABI_API_SVC->service_getServiceByGuid(DownloadManagerGUID);
	if (sf)
		WAC_API_DOWNLOADMANAGER = (api_downloadManager *)sf->getInterface();

	sf = WASABI_API_SVC->service_getServiceByGuid(amgSucksGUID);
	if (sf)
		AGAVE_API_AMGSUCKS = (api_amgsucks *)sf->getInterface();

	sf = WASABI_API_SVC->service_getServiceByGuid(api_playlistsGUID);
	if (sf)
		AGAVE_API_PLAYLISTS = (api_playlists *)sf->getInterface();

		sf = WASABI_API_SVC->service_getServiceByGuid(albumArtGUID);
	if (sf)
		AGAVE_API_ALBUMART = (api_albumart *)sf->getInterface();
}

void Wasabi_ForgetSystemServices()
{
	waServiceFactory *sf;
	if (WINAMP5_API_TAGZ)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(tagzGUID);
		if (sf)
			sf->releaseInterface(WINAMP5_API_TAGZ);
		WINAMP5_API_TAGZ = 0;
	}

	if (AGAVE_API_PLAYLISTMANAGER)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(api_playlistmanagerGUID);
		if (sf)
			sf->releaseInterface(AGAVE_API_PLAYLISTMANAGER);
		AGAVE_API_PLAYLISTMANAGER = 0;
	}

	if (WAC_API_DOWNLOADMANAGER)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(DownloadManagerGUID);
		if (sf)
			sf->releaseInterface(WAC_API_DOWNLOADMANAGER);
		WAC_API_DOWNLOADMANAGER = 0;
	}

	if (WASABI_API_SKIN)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(skinApiServiceGuid);
		if (sf)
			sf->releaseInterface(WASABI_API_SKIN);
		WASABI_API_SKIN = 0;
	}

	if (AGAVE_API_AMGSUCKS)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(amgSucksGUID);
		if (sf)
			sf->releaseInterface(AGAVE_API_AMGSUCKS);
		AGAVE_API_AMGSUCKS = 0;
	}

	if (AGAVE_API_PLAYLISTS)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(api_playlistsGUID);
		if (sf)
			sf->releaseInterface(AGAVE_API_PLAYLISTS);
		AGAVE_API_PLAYLISTS = 0;
	}

	if (AGAVE_API_ALBUMART)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(albumArtGUID);
		if (sf)
			sf->releaseInterface(AGAVE_API_ALBUMART);
		AGAVE_API_ALBUMART = 0;
	}
}