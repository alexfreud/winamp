#include "main.h"
#include "./wasabi.h"
#include <api/service/waservicefactory.h>

static ULONG wasabiRef = 0;

api_application *WASABI_API_APP = NULL;
api_language *WASABI_API_LNG = NULL;
api_explorerfindfile *WASABI_API_EXPLORERFINDFILE = NULL;
obj_ombrowser *browserManager = NULL;
ifc_omservicemanager *serviceManager = NULL;
ifc_omutility *omUtility = NULL;
JSAPI2::api_security *AGAVE_API_JSAPI2_SECURITY = NULL;

HINSTANCE WASABI_API_LNG_HINST = NULL;
HINSTANCE WASABI_API_ORIG_HINST = NULL;
EXTERN_C winampMediaLibraryPlugin plugin;

void *Wasabi_QueryInterface(REFGUID interfaceGuid)
{
	waServiceFactory *serviceFactory = plugin.service->service_getServiceByGuid(interfaceGuid);
	return (NULL != serviceFactory) ? serviceFactory->getInterface() : NULL;
}

void Wasabi_ReleaseInterface(REFGUID interfaceGuid, void *pInstance)
{
	if (NULL == pInstance) return;
	waServiceFactory *serviceFactory = plugin.service->service_getServiceByGuid(interfaceGuid);
	if (NULL != serviceFactory) serviceFactory->releaseInterface(pInstance);
}

void Wasabi_SafeRelease(Dispatchable *pDisp)
{
	if (NULL != pDisp)
		pDisp->Release();
}

BOOL WasabiApi_Initialize(HINSTANCE hInstance)
{
	WASABI_API_APP = QueryWasabiInterface(api_application, applicationApiServiceGuid);
	WASABI_API_LNG = QueryWasabiInterface(api_language, languageApiGUID);
	WASABI_API_EXPLORERFINDFILE = QueryWasabiInterface(api_explorerfindfile, ExplorerFindFileApiGUID);
	AGAVE_API_JSAPI2_SECURITY = QueryWasabiInterface(JSAPI2::api_security, JSAPI2::api_securityGUID);
	OMBROWSERMNGR = QueryWasabiInterface(obj_ombrowser, OBJ_OmBrowser);
	OMSERVICEMNGR = QueryWasabiInterface(ifc_omservicemanager, IFC_OmServiceManager);
	OMUTILITY = QueryWasabiInterface(ifc_omutility, IFC_OmUtility);
	
	if (NULL != WASABI_API_LNG)
		WASABI_API_START_LANG(hInstance, WebDevLangUid);

	WasabiApi_AddRef();
	return TRUE;
}

static void WasabiApi_Uninitialize()
{
	ReleaseWasabiInterface(applicationApiServiceGuid, WASABI_API_APP);
	ReleaseWasabiInterface(languageApiGUID, WASABI_API_LNG);
	ReleaseWasabiInterface(ExplorerFindFileApiGUID, WASABI_API_EXPLORERFINDFILE);
	ReleaseWasabiInterface(JSAPI2::api_securityGUID, AGAVE_API_JSAPI2_SECURITY);
	ReleaseWasabiInterface(OBJ_OmBrowser, OMBROWSERMNGR);
	ReleaseWasabiInterface(IFC_OmServiceManager, OMSERVICEMNGR);
	ReleaseWasabiInterface(IFC_OmUtility, OMUTILITY);

	WASABI_API_APP = NULL;
	WASABI_API_LNG = NULL;
	WASABI_API_EXPLORERFINDFILE = NULL;
	AGAVE_API_JSAPI2_SECURITY = NULL;
}

ULONG WasabiApi_AddRef()
{
	return InterlockedIncrement((LONG*)&wasabiRef);
}

ULONG WasabiApi_Release()
{
	if (0 == wasabiRef)
		return wasabiRef;
	
	LONG r = InterlockedDecrement((LONG*)&wasabiRef);
	if (0 == r)
	{
		WasabiApi_Uninitialize();
	}
	return r;
}