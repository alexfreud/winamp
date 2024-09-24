#include "wac_downloadManager_factory.h"

static const char serviceName[] = "WAC Downloads";
static const char testString[]  = "Downloads Component";

// {B000EE81-199F-48C2-BDCB-F7E3C31A2A13}
static const GUID api_downloads_GUID = { 0xb000ee81, 0x199f, 0x48c2, { 0xbd, 0xcb, 0xf7, 0xe3, 0x1a, 0x12, 0x2a, 0x13 } };


FOURCC wa::Components::WAC_DownloadManagerFactory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *wa::Components::WAC_DownloadManagerFactory::GetServiceName()
{
	return serviceName;
}

GUID wa::Components::WAC_DownloadManagerFactory::GetGUID()
{
	return api_downloads_GUID;
}

const char *wa::Components::WAC_DownloadManagerFactory::GetTestString()
{
	return testString;
}


void *wa::Components::WAC_DownloadManagerFactory::GetInterface( int global_lock )
{
	return nullptr;
}

int wa::Components::WAC_DownloadManagerFactory::ReleaseInterface( void *ifc )
{
	return 1;
}


int wa::Components::WAC_DownloadManagerFactory::SupportNonLockingInterface()
{
	return 1;
}

int wa::Components::WAC_DownloadManagerFactory::ServiceNotify( int msg, int param1, int param2 )
{
	return 1;
}


HRESULT wa::Components::WAC_DownloadManagerFactory::Register( api_service *p_service )
{
	if ( p_service == NULL )
		return E_INVALIDARG;

	p_service->service_register( this );

	return S_OK;
}

HRESULT wa::Components::WAC_DownloadManagerFactory::Unregister( api_service *p_service )
{
	if ( p_service == NULL )
		return E_INVALIDARG;

	p_service->service_deregister( this );

	return S_OK;
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS wa::Components::WAC_DownloadManagerFactory
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