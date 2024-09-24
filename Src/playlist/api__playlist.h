#ifndef NULLSOFT_PLAYLIST_API_H
#define NULLSOFT_PLAYLIST_API_H

#include <api/service/api_service.h>

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include <api/syscb/api_syscb.h>
#define WASABI_API_SYSCB sysCallbackApi

#ifndef ML_PLG_EXPORTS
#include "../Agave/Config/api_config.h"
#endif

#include <api/script/api_maki.h>
#define WASABI_API_MAKI makiApi

#include "../Winamp/JSAPI2_api_security.h"
extern JSAPI2::api_security *jsapi2_security;
#define AGAVE_API_JSAPI2_SECURITY jsapi2_security

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#include <api/service/waservicefactory.h>

template <class api_t>
class QuickService
{
public:
	QuickService( GUID p_serviceGUID )
	{
		_sf = WASABI_API_SVC->service_getServiceByGuid( p_serviceGUID );
		if ( _sf )
			_api = (api_t *)_sf->getInterface();
	}
	~QuickService()
	{
		_sf->releaseInterface( _api );
		_api = 0;
	}
	bool OK()
	{
		return _api != 0;
	}
	api_t *operator ->()
	{
		return _api;
	}
	waServiceFactory *_sf  = NULL;
	api_t            *_api = 0;
};

#include "../Agave/Language/api_language.h"

#endif  // !NULLSOFT_PLAYLIST_API_H