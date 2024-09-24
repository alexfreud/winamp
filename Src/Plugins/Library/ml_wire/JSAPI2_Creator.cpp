#include "main.h"
#include "JSAPI2_Creator.h"
#include "JSAPI2_PodcastsAPI.h"
#include "api__ml_wire.h"

IDispatch *JSAPI2_Creator::CreateAPI(const wchar_t *name, const wchar_t *key, JSAPI::ifc_info *info)
{
	if (!_wcsicmp(name, L"Podcasts"))
		return new JSAPI2::PodcastsAPI(key, info);
	else
		return 0;
}

int JSAPI2_Creator::PromptForAuthorization(HWND parent, const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI2::api_security::AuthorizationData *data)
{
	if (group && !_wcsicmp(group, L"podcasts"))
	{
		const wchar_t *title_str = AGAVE_API_JSAPI2_SECURITY->GetAssociatedName(authorization_key);
		return AGAVE_API_JSAPI2_SECURITY->SecurityPrompt(parent, title_str, L"This service is trying to subscribe you to a podcast.", JSAPI2::svc_apicreator::AUTHORIZATION_FLAG_GROUP_ONLY);
	}
	else
		return JSAPI2::svc_apicreator::AUTHORIZATION_UNDEFINED;
}


#define CBCLASS JSAPI2_Creator
START_DISPATCH;
CB(JSAPI2_SVC_APICREATOR_CREATEAPI, CreateAPI);
CB(JSAPI2_SVC_APICREATOR_PROMPTFORAUTHORIZATION, PromptForAuthorization);
END_DISPATCH;
#undef CBCLASS

static JSAPI2_Creator jsapi2_svc;
static const char serviceName[] = "Podcast Javascript Objects";

// {EE2C54DB-E609-410a-A962-573BA2F9C3AC}
static const GUID jsapi2_factory_guid = 
{ 0xee2c54db, 0xe609, 0x410a, { 0xa9, 0x62, 0x57, 0x3b, 0xa2, 0xf9, 0xc3, 0xac } };

FOURCC JSAPI2Factory::GetServiceType()
{
	return jsapi2_svc.getServiceType();
}

const char *JSAPI2Factory::GetServiceName()
{
	return serviceName;
}

GUID JSAPI2Factory::GetGUID()
{
	return jsapi2_factory_guid;
}

void *JSAPI2Factory::GetInterface(int global_lock)
{
//	if (global_lock)
//		plugin.service->service_lock(this, (void *)ifc);
	return &jsapi2_svc;
}

int JSAPI2Factory::SupportNonLockingInterface()
{
	return 1;
}

int JSAPI2Factory::ReleaseInterface(void *ifc)
{
	//plugin.service->service_unlock(ifc);
	return 1;
}

const char *JSAPI2Factory::GetTestString()
{
	return 0;
}

int JSAPI2Factory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS JSAPI2Factory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;
#undef CBCLASS