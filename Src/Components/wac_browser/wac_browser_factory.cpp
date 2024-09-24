#include "wac_browser_factory.h"

#include <QtGlobal>

static const std::string _serviceName = "Embedded Browser";
static const std::string _testString  = "wac_browser";

// {2E0F69D7-1DA4-4207-97A4-B8B084776B9C}  // It's the UUID of the project itself like indicated into the vcxproj
static const GUID api_wac_browser_GUID = { 0x2e0f69d7, 0x1da4, 0x4207, { 0x97, 0xa4, 0xb8, 0xb0, 0x84, 0x77, 0x6b, 0x9c } };


FOURCC wa::Components::WAC_BrowserFactory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *wa::Components::WAC_BrowserFactory::GetServiceName()
{
	return _serviceName.c_str();
}

const char *wa::Components::WAC_BrowserFactory::GetTestString()
{
	return _testString.c_str();;
}

GUID wa::Components::WAC_BrowserFactory::GetGUID()
{
	return api_wac_browser_GUID;
}


void *wa::Components::WAC_BrowserFactory::GetInterface( int p_global_lock )
{
	Q_UNUSED( p_global_lock )

	//OmBrowserObject *browserObject = NULL;

	//HRESULT hr = OmBrowserObject::CreateInstance( &browserObject );
	//if ( FAILED( hr ) )
	//	browserObject = NULL;

	//return browserObject;

	return NULL;
}

int wa::Components::WAC_BrowserFactory::ReleaseInterface( void *p_ifc )
{
	//obj_ombrowser *object = (obj_ombrowser *)ifc;
	//if ( object != NULL )
	//	object->Release();

	return 1;
}


int wa::Components::WAC_BrowserFactory::SupportNonLockingInterface()
{
	return 1;
}

int wa::Components::WAC_BrowserFactory::ServiceNotify( int p_msg, int p_param1, int p_param2 )
{
	Q_UNUSED( p_msg )
	Q_UNUSED( p_param1 )
	Q_UNUSED( p_param2 )

	return 1;
}


HRESULT wa::Components::WAC_BrowserFactory::Register( api_service *p_service )
{
	if ( p_service == NULL )
		return E_INVALIDARG;

	p_service->service_register( this );

	return S_OK;
}

HRESULT wa::Components::WAC_BrowserFactory::Unregister( api_service *p_service )
{
	if ( p_service == NULL )
		return E_INVALIDARG;

	p_service->service_deregister( this );

	return S_OK;
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS wa::Components::WAC_BrowserFactory
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
