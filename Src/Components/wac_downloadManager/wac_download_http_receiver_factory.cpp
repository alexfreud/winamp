#define GUID_EQUALS_DEFINED

//#include "util.h"

#include "api__wac_downloadManager.h"

#include "wac_download_http_receiver.h"
//#include "wac_download_http_receiver_api.h"
#include "wac_download_http_receiver_factory.h"


static const std::string _serviceName = "DownloadManager HTTP Receiver Service";


FOURCC wa::Factory::WAC_DownloadMabager_HTTPReceiver_Factory::GetServiceType()
{
	return WaSvc::OBJECT;
}

const char *wa::Factory::WAC_DownloadMabager_HTTPReceiver_Factory::GetServiceName()
{
	return _serviceName.c_str();
}

GUID wa::Factory::WAC_DownloadMabager_HTTPReceiver_Factory::GetGUID()
{
	return httpreceiverGUID2;
}

void *wa::Factory::WAC_DownloadMabager_HTTPReceiver_Factory::GetInterface( int global_lock )
{
	//if ( JNL::open_socketlib() )
	//	return NULL;

	api_wac_download_manager_http_receiver *ifc = new wa::Components::WAC_Download_HTTP_Receiver();

	//JNL::close_socketlib(); // new JNL_HTTPGet will call open_socketlib also, so we can release now

	////	if (global_lock)
	////		WASABI_API_SVC->service_lock(this, (void *)ifc);

	return ifc;
	//return NULL;
}

int wa::Factory::WAC_DownloadMabager_HTTPReceiver_Factory::SupportNonLockingInterface()
{
	return 1;
}

int wa::Factory::WAC_DownloadMabager_HTTPReceiver_Factory::ReleaseInterface( void *ifc )
{
	//WASABI_API_SVC->service_unlock(ifc);
	api_wac_download_manager_http_receiver *httpget = static_cast<api_wac_download_manager_http_receiver *>( ifc );
	if ( httpget )
		httpget->Release();

	return 1;
}

const char *wa::Factory::WAC_DownloadMabager_HTTPReceiver_Factory::GetTestString()
{
	return NULL;
}

int wa::Factory::WAC_DownloadMabager_HTTPReceiver_Factory::ServiceNotify( int msg, int param1, int param2 )
{
	return 1;
}

#define CBCLASS wa::Factory::WAC_DownloadMabager_HTTPReceiver_Factory
START_DISPATCH;
CB( WASERVICEFACTORY_GETSERVICETYPE,                GetServiceType )
CB( WASERVICEFACTORY_GETSERVICENAME,                GetServiceName )
CB( WASERVICEFACTORY_GETGUID,                       GetGUID )
CB( WASERVICEFACTORY_GETINTERFACE,                  GetInterface )
CB( WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface )
CB( WASERVICEFACTORY_RELEASEINTERFACE,              ReleaseInterface )
CB( WASERVICEFACTORY_GETTESTSTRING,                 GetTestString )
CB( WASERVICEFACTORY_SERVICENOTIFY,                 ServiceNotify )
END_DISPATCH;
#undef CBCLASS