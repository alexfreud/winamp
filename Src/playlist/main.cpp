#include "api__playlist.h"
#include "main.h"

#include "factory_Handler.h"
#include "factory_playlistmanager.h"
#include "factory_playlists.h"
#include "../Winamp/api_random.h"
#include "Playlists.h"
#include "plstring.h"
#include "ScriptObjectFactory.h"
#include "../nu/ServiceWatcher.h"
#include "JSAPI2_Creator.h"

extern Playlists playlists;
int (*warand)(void) = 0;

M3UHandlerFactory       m3uHandlerFactory;
PLSHandlerFactory       plsHandlerFactory;
B4SHandlerFactory       b4sHandlerFactory;
PlaylistManagerFactory  playlistManagerFactory;
PlaylistsFactory        playlistsFactory;
ScriptObjectFactory     scriptObjectFactory;
JSAPI2Factory           jsapi2Factory;
ServiceWatcher          serviceWatcher;
PlaylistComponent       playlistComponent;

api_service            *WASABI_API_SVC            = 0;
api_application        *WASABI_API_APP            = 0;
api_config             *AGAVE_API_CONFIG          = 0;
api_syscb              *WASABI_API_SYSCB          = 0;
api_maki               *WASABI_API_MAKI           = 0;
JSAPI2::api_security   *AGAVE_API_JSAPI2_SECURITY = 0;
api_stats              *AGAVE_API_STATS           = 0;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

template <class api_t>
api_t *GetService( GUID serviceGUID )
{
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid( serviceGUID );
	if ( sf )
		return (api_t *)sf->getInterface();
	else
		return 0;

}

inline void ReleaseService( GUID serviceGUID, void *service )
{
	if ( service )
	{
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid( serviceGUID );
		if ( sf )
			sf->releaseInterface( service );
	}
}

void PlaylistComponent::RegisterServices( api_service *service )
{
	WASABI_API_SVC = service;
	
	warand = QuickService<api_random>( randomApiGUID )->GetFunction();

	WASABI_API_APP            = GetService<api_application>( applicationApiServiceGuid );
	WASABI_API_SYSCB          = GetService<api_syscb>( syscbApiServiceGuid );
	AGAVE_API_CONFIG          = GetService<api_config>( AgaveConfigGUID );
	AGAVE_API_JSAPI2_SECURITY = GetService<JSAPI2::api_security>( JSAPI2::api_securityGUID );
	AGAVE_API_STATS           = GetService<api_stats>( AnonymousStatsGUID );

	serviceWatcher.WatchWith( WASABI_API_SVC );
	serviceWatcher.WatchFor( &WASABI_API_MAKI, makiApiServiceGuid );

	// need to get WASABI_API_APP first
	plstring_init();

	WASABI_API_SVC->service_register( &m3uHandlerFactory );
	WASABI_API_SVC->service_register( &plsHandlerFactory );
	WASABI_API_SVC->service_register( &b4sHandlerFactory );
	WASABI_API_SVC->service_register( &playlistManagerFactory );
	WASABI_API_SVC->service_register( &playlistsFactory );
	WASABI_API_SVC->service_register( &scriptObjectFactory );
	WASABI_API_SVC->service_register( &jsapi2Factory );

	WASABI_API_LNG = GetService<api_language>( languageApiGUID );
	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG( hModule, playlistLangGUID );

	// register for service callbacks in case any of these don't exist yet
	WASABI_API_SYSCB->syscb_registerCallback( &serviceWatcher );
}

int PlaylistComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void PlaylistComponent::DeregisterServices( api_service *service )
{
	playlists.Flush();

	service->service_deregister( &playlistsFactory );
	service->service_deregister( &playlistManagerFactory );
	service->service_deregister( &m3uHandlerFactory );
	service->service_deregister( &plsHandlerFactory );
	service->service_deregister( &b4sHandlerFactory );
	service->service_deregister( &scriptObjectFactory );
	service->service_deregister( &jsapi2Factory );

	serviceWatcher.StopWatching();
	serviceWatcher.Clear();

	ReleaseService( makiApiServiceGuid,        WASABI_API_MAKI );
	ReleaseService( applicationApiServiceGuid, WASABI_API_APP );
	ReleaseService( AgaveConfigGUID,           AGAVE_API_CONFIG );
	ReleaseService( syscbApiServiceGuid,       WASABI_API_SYSCB );
	ReleaseService( languageApiGUID,           WASABI_API_LNG );
	ReleaseService( JSAPI2::api_securityGUID,  AGAVE_API_JSAPI2_SECURITY );
	ReleaseService( AnonymousStatsGUID,        AGAVE_API_STATS );
}

extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &playlistComponent;
}

#define CBCLASS PlaylistComponent
START_DISPATCH;
VCB( API_WA5COMPONENT_REGISTERSERVICES,           RegisterServices )
CB(  API_WA5COMPONENT_REGISTERSERVICES_SAFE_MODE, RegisterServicesSafeModeOk )
VCB( API_WA5COMPONENT_DEREEGISTERSERVICES,        DeregisterServices )
END_DISPATCH;
#undef CBCLASS