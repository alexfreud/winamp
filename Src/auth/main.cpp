#include "api.h"
#include "../Agave/Component/ifc_wa5component.h"
#include <api/service/waservicefactory.h>
#include "../nu/Singleton.h"
#include "auth.h"

EXTERN_C const GUID pngLoaderGUID = 
{ 0x5e04fb28, 0x53f5, 0x4032, { 0xbd, 0x29, 0x3, 0x2b, 0x87, 0xec, 0x37, 0x25 } };


class AuthComponent : public ifc_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};

api_service *WASABI_API_SVC=0;
api_application *WASABI_API_APP =0;
api_config *AGAVE_API_CONFIG = 0;
api_syscb *WASABI_API_SYSCB = 0;
api_winamp *WASABI_API_WINAMP = 0;
api_memmgr *WASABI_API_MEMMNGR = NULL;
svc_imageLoader *WASABI_API_PNGLOADER = NULL;

api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static Auth auth;
static SingletonServiceFactory<api_auth, Auth> authFactory;

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = (api_T *)factory->getInterface();
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC && api_t)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = 0;
}

void AuthComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceBuild(WASABI_API_WINAMP, winampApiGuid);

	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(hModule, authLangGUID);

	auth.Init();
	authFactory.Register(WASABI_API_SVC, &auth);
}

int AuthComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void AuthComponent::DeregisterServices(api_service *service)
{
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceRelease(WASABI_API_WINAMP, winampApiGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_MEMMNGR, memMgrApiServiceGuid);
	ServiceRelease(WASABI_API_PNGLOADER, pngLoaderGUID);

	authFactory.Deregister(WASABI_API_SVC);
	auth.Quit();
}

static AuthComponent authComponent;

extern "C" __declspec(dllexport) ifc_wa5component *GetWinamp5SystemComponent()
{
	return &authComponent;
}

#define CBCLASS AuthComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
