#include "api.h"
#include "w5s.h"
#include "GracenoteFactory.h"
#include <bfc/platform/export.h>
#include <api/service/waservicefactory.h>
#include "GracenoteApi.h"

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>( factory->getInterface() );
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
	api_t = NULL;
}

api_application *WASABI_API_APP = 0;
api_service *WASABI_API_SVC = 0;
api_config *AGAVE_API_CONFIG=0;
api_language *WASABI_API_LNG = 0;

GracenoteFactory gracenoteFactory;

void WA5_Gracenote::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;

	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	WASABI_API_SVC->service_register(&gracenoteFactory);
}

int WA5_Gracenote::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_Gracenote::DeregisterServices(api_service *service)
{
	gracenoteApi.Close();
	WASABI_API_SVC->service_deregister(&gracenoteFactory);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
}

static WA5_Gracenote wa5_gracenote;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_gracenote;
}

#define CBCLASS WA5_Gracenote
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
  CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS