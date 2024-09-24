#include "main.h"
#include "./wasabi.h"

#include <api/service/waservicefactory.h>

api_service *WASABI_API_SVC = NULL;
api_application *WASABI_API_APP = NULL;
api_language *WASABI_API_LNG = NULL;
api_devicemanager *WASABI_API_DEVICES = NULL;

HINSTANCE WASABI_API_LNG_HINST = NULL;
HINSTANCE WASABI_API_ORIG_HINST = NULL;

static unsigned long wasabiReference = 0;
static BOOL defaultServicesLoaded = FALSE;

static void 
Wasabi_Uninitialize()
{
	if (NULL != WASABI_API_SVC)
	{	
		Wasabi_ReleaseInterface(applicationApiServiceGuid, WASABI_API_APP);
		Wasabi_ReleaseInterface(languageApiGUID, WASABI_API_LNG);
		Wasabi_ReleaseInterface(DeviceManagerGUID, WASABI_API_DEVICES);
	}

	WASABI_API_SVC = NULL;
	WASABI_API_APP = NULL;
	WASABI_API_LNG = NULL;
	WASABI_API_DEVICES = NULL;
	defaultServicesLoaded = FALSE;
}

BOOL 
Wasabi_Initialize(HINSTANCE instance, api_service *serviceMngr)
{
	if (NULL != WASABI_API_SVC)
		return FALSE;

	defaultServicesLoaded = FALSE;

	WASABI_API_SVC = serviceMngr;
	if ((api_service*)1 == WASABI_API_SVC)
		WASABI_API_SVC = NULL;

	if (NULL == WASABI_API_SVC) 
		return FALSE;
	
	WASABI_API_APP = NULL;
	WASABI_API_DEVICES = NULL;

	WASABI_API_LNG = NULL;
	WASABI_API_ORIG_HINST = instance;
	WASABI_API_LNG_HINST = WASABI_API_ORIG_HINST;
	
	Wasabi_AddRef();
	return TRUE;
}

BOOL 
Wasabi_InitializeFromWinamp(HINSTANCE instance, HWND winampWindow)
{
	api_service *serviceMngr;
	serviceMngr = (api_service*)SENDWAIPC(winampWindow, IPC_GET_API_SERVICE, 0);
	return Wasabi_Initialize(instance, serviceMngr);
}

BOOL
Wasabi_LoadDefaultServices(void)
{
	if (NULL == WASABI_API_SVC) 
		return FALSE;

	if (FALSE != defaultServicesLoaded)
		return FALSE;

	WASABI_API_APP = Wasabi_QueryInterface(api_application, applicationApiServiceGuid);
	WASABI_API_DEVICES = Wasabi_QueryInterface(api_devicemanager, DeviceManagerGUID);

	WASABI_API_LNG = Wasabi_QueryInterface(api_language, languageApiGUID);
	if (NULL != WASABI_API_LNG)
	{
		WASABI_API_LNG_HINST = WASABI_API_LNG->StartLanguageSupport(WASABI_API_ORIG_HINST, 
																	PLUGIN_LANGUAGE_ID);
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

void *
Wasabi_QueryInterface0(const GUID &interfaceGuid)
{
	waServiceFactory *serviceFactory;

	if (NULL == WASABI_API_SVC)
		return NULL;

	serviceFactory = WASABI_API_SVC->service_getServiceByGuid(interfaceGuid);
	if (NULL == serviceFactory)
		return NULL;
	
	return serviceFactory->getInterface();
}

void 
Wasabi_ReleaseInterface0(const GUID &interfaceGuid, void *interfaceInstance)
{
	waServiceFactory *serviceFactory;

	if (NULL == WASABI_API_SVC)
		return;

	serviceFactory = WASABI_API_SVC->service_getServiceByGuid(interfaceGuid);
	if (NULL == serviceFactory)
		return;
	
	serviceFactory->releaseInterface(interfaceInstance);
}
