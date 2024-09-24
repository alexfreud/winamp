#include "main.h"
#include "./wasabi.h"

#include <api/service/waservicefactory.h>

api_application *WASABI_API_APP = NULL;
api_language *WASABI_API_LNG = NULL;
api_devicemanager *WASABI_API_DEVICES = NULL;
api_winamp *WASABI_API_WINAMP = NULL;

HINSTANCE WASABI_API_LNG_HINST = NULL;
HINSTANCE WASABI_API_ORIG_HINST = NULL;

static unsigned long wasabiReference = 0;
static BOOL defaultServicesLoaded = FALSE;
EXTERN_C winampMediaLibraryPlugin plugin;

static void Wasabi_Uninitialize()
{
	Wasabi_ReleaseInterface(applicationApiServiceGuid, WASABI_API_APP);
	Wasabi_ReleaseInterface(languageApiGUID, WASABI_API_LNG);
	Wasabi_ReleaseInterface(DeviceManagerGUID, WASABI_API_DEVICES);
	Wasabi_ReleaseInterface(winampApiGuid, WASABI_API_WINAMP);

	WASABI_API_APP = NULL;
	WASABI_API_LNG = NULL;
	WASABI_API_DEVICES = NULL;
	WASABI_API_WINAMP = NULL;
	defaultServicesLoaded = FALSE;
}

BOOL Wasabi_Initialize(HINSTANCE instance)
{
	defaultServicesLoaded = FALSE;

	WASABI_API_APP = NULL;
	WASABI_API_DEVICES = NULL;

	WASABI_API_LNG = NULL;
	WASABI_API_ORIG_HINST = instance;
	WASABI_API_LNG_HINST = WASABI_API_ORIG_HINST;

	Wasabi_AddRef();
	return TRUE;
}

BOOL Wasabi_InitializeFromWinamp(HINSTANCE instance, HWND winampWindow)
{
	return Wasabi_Initialize(instance);
}

BOOL Wasabi_LoadDefaultServices(void)
{
	if (FALSE != defaultServicesLoaded)
		return FALSE;

	WASABI_API_APP = Wasabi_QueryInterface(api_application, applicationApiServiceGuid);
	WASABI_API_DEVICES = Wasabi_QueryInterface(api_devicemanager, DeviceManagerGUID);
	WASABI_API_WINAMP = Wasabi_QueryInterface(api_winamp, winampApiGuid);

	WASABI_API_LNG = Wasabi_QueryInterface(api_language, languageApiGUID);
	if (NULL != WASABI_API_LNG)
	{
		WASABI_API_LNG_HINST = WASABI_API_LNG->StartLanguageSupport(WASABI_API_ORIG_HINST, MlDevicesLangGUID);
	}

	defaultServicesLoaded = TRUE;
	return TRUE;
}

unsigned long 
Wasabi_AddRef(void)
{
	return InterlockedIncrement((LONG*)&wasabiReference);
}

unsigned long
Wasabi_Release(void)
{
	if (0 == wasabiReference)
		return wasabiReference;
	
	LONG r = InterlockedDecrement((LONG*)&wasabiReference);
	if (0 == r)
	{
		Wasabi_Uninitialize();
	}
	return r;
}

void * Wasabi_QueryInterface0(const GUID &interfaceGuid)
{
	waServiceFactory *serviceFactory = plugin.service->service_getServiceByGuid(interfaceGuid);
	if (NULL == serviceFactory)
		return NULL;

	return serviceFactory->getInterface();
}

void Wasabi_ReleaseInterface0(const GUID &interfaceGuid, void *interfaceInstance)
{
	waServiceFactory *serviceFactory = plugin.service->service_getServiceByGuid(interfaceGuid);
	if (NULL == serviceFactory)
		return;

	serviceFactory->releaseInterface(interfaceInstance);
}
