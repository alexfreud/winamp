#include "JSAPI2_Creator.h"
#include "JSAPI2_Playlists.h"
#include "api__playlist.h"
#include "main.h"
#include "resource.h"

IDispatch *JSAPI2_Creator::CreateAPI(const wchar_t *name, const wchar_t *key, JSAPI::ifc_info *info)
{
	if (!wcscmp(name, L"Playlists"))
		return new JSAPI2::PlaylistsAPI(key, info);
	else
		return 0;
}

int JSAPI2_Creator::PromptForAuthorization(HWND parent, const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI2::api_security::AuthorizationData *data)
{
	if (group && !wcscmp(group, L"playlists"))
	{
		const wchar_t *title_str = AGAVE_API_JSAPI2_SECURITY->GetAssociatedName(authorization_key);
		if (action && !wcscmp(action, L"read"))
		{
			return AGAVE_API_JSAPI2_SECURITY->SecurityPrompt(parent, title_str, L"This service is requesting access to the playlists in your Library.", 0);
		}
		else if (action && !wcscmp(action, L"write"))
		{
			return AGAVE_API_JSAPI2_SECURITY->SecurityPrompt(parent, title_str, L"This service is trying modify one of the playlists in your Library.", 0);
		}
	}

	return JSAPI2::svc_apicreator::AUTHORIZATION_UNDEFINED;
}

#define CBCLASS JSAPI2_Creator
START_DISPATCH;
CB(JSAPI2_SVC_APICREATOR_CREATEAPI, CreateAPI);
CB(JSAPI2_SVC_APICREATOR_PROMPTFORAUTHORIZATION, PromptForAuthorization);
END_DISPATCH;
#undef CBCLASS

static JSAPI2_Creator jsapi2_svc;
static const char serviceName[] = "Playlist Javascript Objects";

// {CF057176-A819-4bc5-8723-6C072BB28BAA}
static const GUID jsapi2_factory_guid = 
{ 0xcf057176, 0xa819, 0x4bc5, { 0x87, 0x23, 0x6c, 0x7, 0x2b, 0xb2, 0x8b, 0xaa } };


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