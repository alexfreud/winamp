#include "api__ml_online.h"
#include "JSAPI2_Creator.h"
#include "jsapi2_omcom.h"
#include "resource.h"


IDispatch *JSAPI2_Creator::CreateAPI(const wchar_t *name, const wchar_t *key, JSAPI::ifc_info *info)
{
	if (!wcscmp(name, L"OnlineServices"))
		return new JSAPI2::OnlineServicesAPI(key, info);
	else
		return 0;
}

int JSAPI2_Creator::PromptForAuthorization(HWND parent, const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI2::api_security::AuthorizationData *data)
{
	if (group && !wcscmp(group, L"onlineservices"))
	{
		return JSAPI2::svc_apicreator::AUTHORIZATION_DENY;
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
static const char serviceName[] = "Online Services Javascript Objects";

// {4B0FA456-2B3B-4584-A96C-54765EA46448}
static const GUID jsapi2_factory_guid = 
{ 0x4b0fa456, 0x2b3b, 0x4584, { 0xa9, 0x6c, 0x54, 0x76, 0x5e, 0xa4, 0x64, 0x48 } };

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
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return &jsapi2_svc;
}

int JSAPI2Factory::SupportNonLockingInterface()
{
	return 1;
}

int JSAPI2Factory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
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