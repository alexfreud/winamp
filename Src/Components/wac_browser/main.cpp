#include "main.h"

#include "wac_browser_factory.h"

#include "bfc/platform/export.h"

api_service *WASABI_API_SVC   = NULL;
api_config  *AGAVE_API_CONFIG = NULL;

static wa::Components::WAC_BrowserFactory _wacDownloadFactory;
static wa::Components::WAC_Browser       _wac_browser;


void wa::Components::WAC_Browser::RegisterServices( api_service *p_service )
{
	WASABI_API_SVC = p_service;
	ServiceBuild( AGAVE_API_CONFIG, AgaveConfigGUID );
	WASABI_API_SVC->service_register( &_wacDownloadFactory );
}

void wa::Components::WAC_Browser::DeregisterServices( api_service *p_service )
{
	p_service->service_deregister( &_wacDownloadFactory );
	ServiceRelease( AGAVE_API_CONFIG, AgaveConfigGUID );
}


extern "C" DLLEXPORT ifc_wa5component * GetWinamp5SystemComponent()
{
	return &_wac_browser;
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS wa::Components::WAC_Browser
START_DISPATCH;
VCB( API_WA5COMPONENT_REGISTERSERVICES,           RegisterServices )
CB(  API_WA5COMPONENT_REGISTERSERVICES_SAFE_MODE, RegisterServicesSafeModeOk )
VCB( API_WA5COMPONENT_DEREEGISTERSERVICES,        DeregisterServices )
END_DISPATCH;
#undef CBCLASS
