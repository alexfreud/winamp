#include "main.h"

//#include "wac_download_http_receiver_factory.h"

#include "wac_downloadManager.h"

#include "bfc/platform/export.h"

static Singleton2<api_downloadManager, DownloadManager> dlMgrFactory;
static DownloadManager dlMgr;

//static wa::Components::WAC_DownloadsFactory _wac_download_factory;
static wa::Components::WAC_DownloadManager_Component  _wac_downloadManager_Component;

//wa::Factory::WAC_DownloadMabager_HTTPReceiver_Factory _wac_downloadManager_http_receiver_service;

api_service     *WASABI_API_SVC        = 0;
api_application *WASABI_API_APP        = 0;
api_config      *AGAVE_API_CONFIG      = 0;
api_threadpool  *WASABI_API_THREADPOOL = 0;


void wa::Components::WAC_DownloadManager_Component::RegisterServices( api_service *p_service )
{
	WASABI_API_SVC = p_service;

	ServiceBuild( WASABI_API_APP,        applicationApiServiceGuid );
	ServiceBuild( AGAVE_API_CONFIG,      AgaveConfigGUID );
	ServiceBuild( WASABI_API_THREADPOOL, ThreadPoolGUID );


	dlMgrFactory.Register( &dlMgr );

	//WASABI_API_SVC->service_register( &_wac_downloadManager_http_receiver_service );
}

void wa::Components::WAC_DownloadManager_Component::DeregisterServices( api_service *p_service )
{
	Q_UNUSED( p_service )

	ServiceRelease( WASABI_API_APP,        applicationApiServiceGuid );
	ServiceRelease( AGAVE_API_CONFIG,      AgaveConfigGUID );

	dlMgr.Kill();
	dlMgrFactory.Deregister();

	ServiceRelease( WASABI_API_THREADPOOL, ThreadPoolGUID );

	//p_service->service_deregister( &_wac_downloadManager_http_receiver_service );
}


extern "C" WAC_DOWNLOAD_MANAGER_EXPORT ifc_wa5component * GetWinamp5SystemComponent()
{
	return &_wac_downloadManager_Component;
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS wa::Components::WAC_DownloadManager_Component
START_DISPATCH;
VCB( API_WA5COMPONENT_REGISTERSERVICES,           RegisterServices )
CB(  API_WA5COMPONENT_REGISTERSERVICES_SAFE_MODE, RegisterServicesSafeModeOk )
VCB( API_WA5COMPONENT_DEREEGISTERSERVICES,        DeregisterServices )
END_DISPATCH;
#undef CBCLASS
