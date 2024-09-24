#include "main.h"
#include "./api__ml_online.h"
#include <api/service/waservicefactory.h>
#include "../nu/Singleton.h"
#include "handler.h"
#include "JSAPI2_Creator.h"

static ULONG wasabiRef = 0;

api_service *WASABI_API_SVC = NULL;
api_application *WASABI_API_APP = NULL;
api_config *AGAVE_API_CONFIG = NULL;
api_language *WASABI_API_LNG = NULL;
api_explorerfindfile *WASABI_API_EXPLORERFINDFILE = NULL;
api_memmgr *WASABI_API_MEMMNGR = NULL;
svc_imageLoader *pngLoaderApi = NULL;
obj_ombrowser *browserManager = NULL;
ifc_omservicemanager *serviceManager = NULL;
ifc_omutility *omUtility = NULL;
JSAPI2::api_security *AGAVE_API_JSAPI2_SECURITY = NULL;
api_auth *AGAVE_API_AUTH = 0;
static JSAPI2Factory jsapi2Factory;
HINSTANCE WASABI_API_LNG_HINST = NULL;
HINSTANCE WASABI_API_ORIG_HINST = NULL;
static BOOL fDefaultsLoaded = FALSE;
static OnlineServicesURIHandler uri_handler;
static SingletonServiceFactory<svc_urihandler, OnlineServicesURIHandler> uri_handler_factory;

EXTERN_C const GUID pngLoaderGUID = 
{ 0x5e04fb28, 0x53f5, 0x4032, { 0xbd, 0x29, 0x3, 0x2b, 0x87, 0xec, 0x37, 0x25 } };

void *Wasabi_QueryInterface(REFGUID interfaceGuid)
{
	waServiceFactory *serviceFactory = WASABI_API_SVC->service_getServiceByGuid(interfaceGuid);
	return (NULL != serviceFactory) ? serviceFactory->getInterface() : NULL;
}

void Wasabi_ReleaseInterface(REFGUID interfaceGuid, void *pInstance)
{
	if (NULL == pInstance) return;
	waServiceFactory *serviceFactory = WASABI_API_SVC->service_getServiceByGuid(interfaceGuid);
	if (NULL != serviceFactory) serviceFactory->releaseInterface(pInstance);
}


HRESULT WasabiApi_Initialize(HINSTANCE hInstance, api_service *serviceApi)
{
	if (NULL != WASABI_API_SVC)
		return S_FALSE;

	fDefaultsLoaded = FALSE;

	WASABI_API_SVC = serviceApi;
	if ((api_service *)1 == WASABI_API_SVC)
		WASABI_API_SVC = NULL;

	if (NULL == WASABI_API_SVC) 
		return E_FAIL;
	
	WASABI_API_ORIG_HINST = hInstance;
	WASABI_API_LNG_HINST = WASABI_API_ORIG_HINST;

	WasabiApi_AddRef();
	return S_OK;
}

HRESULT WasabiApi_LoadDefaults()
{
	if (NULL == WASABI_API_SVC) 
		return E_UNEXPECTED;

	if (FALSE != fDefaultsLoaded)
		return S_FALSE;

	WASABI_API_APP = QueryWasabiInterface(api_application, applicationApiServiceGuid);
	WASABI_API_LNG = QueryWasabiInterface(api_language, languageApiGUID);
	WASABI_API_EXPLORERFINDFILE = QueryWasabiInterface(api_explorerfindfile, ExplorerFindFileApiGUID);
	AGAVE_API_CONFIG = QueryWasabiInterface(api_config, AgaveConfigGUID);
	AGAVE_API_JSAPI2_SECURITY = QueryWasabiInterface(JSAPI2::api_security, JSAPI2::api_securityGUID);
	OMBROWSERMNGR = QueryWasabiInterface(obj_ombrowser, OBJ_OmBrowser);
	OMSERVICEMNGR = QueryWasabiInterface(ifc_omservicemanager, IFC_OmServiceManager);
	OMUTILITY = QueryWasabiInterface(ifc_omutility, IFC_OmUtility);
	AGAVE_API_AUTH = QueryWasabiInterface(api_auth, AuthApiGUID);
	
	if (NULL != WASABI_API_LNG)
		WASABI_API_LNG_HINST = WASABI_API_LNG->StartLanguageSupport(WASABI_API_ORIG_HINST, MlOnlineLangGUID);

	WASABI_API_SVC->service_register(&jsapi2Factory);	
	uri_handler_factory.Register(WASABI_API_SVC, &uri_handler);

	fDefaultsLoaded = TRUE;
	return S_OK;
}

static void WasabiApi_Uninitialize()
{
	if (NULL != WASABI_API_SVC)
	{	
		ReleaseWasabiInterface(applicationApiServiceGuid, WASABI_API_APP);
		ReleaseWasabiInterface(languageApiGUID, WASABI_API_LNG);
		ReleaseWasabiInterface(ExplorerFindFileApiGUID, WASABI_API_EXPLORERFINDFILE);
		ReleaseWasabiInterface(AgaveConfigGUID, AGAVE_API_CONFIG);
		ReleaseWasabiInterface(memMgrApiServiceGuid, WASABI_API_MEMMNGR);
		ReleaseWasabiInterface(pngLoaderGUID, WASABI_API_PNGLOADER);
		ReleaseWasabiInterface(JSAPI2::api_securityGUID, AGAVE_API_JSAPI2_SECURITY);
		ReleaseWasabiInterface(AuthApiGUID, AGAVE_API_AUTH);
		ReleaseWasabiInterface(OBJ_OmBrowser, OMBROWSERMNGR);
		ReleaseWasabiInterface(IFC_OmServiceManager, OMSERVICEMNGR);
		ReleaseWasabiInterface(IFC_OmUtility, OMUTILITY);
		WASABI_API_SVC->service_deregister(&jsapi2Factory);
		uri_handler_factory.Deregister(WASABI_API_SVC);
	}

	WASABI_API_SVC = NULL;
	WASABI_API_APP = NULL;
	WASABI_API_LNG = NULL;
	WASABI_API_EXPLORERFINDFILE = NULL;
	AGAVE_API_CONFIG = NULL;
	AGAVE_API_JSAPI2_SECURITY = NULL;
	AGAVE_API_AUTH = NULL;
	OMBROWSERMNGR = NULL;
	OMSERVICEMNGR = NULL;
	OMUTILITY = NULL;

	fDefaultsLoaded = FALSE;
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