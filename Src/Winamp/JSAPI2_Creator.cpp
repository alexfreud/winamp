#include "main.h"
#include "resource.h"
#include "JSAPI2_Creator.h"
#include "JSAPI2_TransportAPI.h"
#include "JSAPI2_PlayerAPI.h"
#include "JSAPI2_Downloader.h"
#include "JSAPI2_SecurityAPI.h"
#include "JSAPI2_Security.h"
#include "JSAPI2_Bookmarks.h"
#include "JSAPI2_Application.h"
#include "JSAPI2_SkinAPI.h"
#include "JSAPI2_MediaCore.h"
#include "JSAPI2_AsyncDownloader.h"
#include "api.h"
#include "language.h"

IDispatch *JSAPI2_Creator::CreateAPI(const wchar_t *name, const wchar_t *key, JSAPI::ifc_info *info)
{
	if (!wcscmp(name, L"Transport"))
		return new JSAPI2::TransportAPI(key, info);
	else if (!wcscmp(name, L"PlayQueue"))
		return new JSAPI2::PlayerAPI(key, info);
	else if (!wcscmp(name, L"Downloader"))
		return new JSAPI2::DownloaderAPI(key, info);
	else if (!wcscmp(name, L"AsyncDownloader"))
		return new JSAPI2::AsyncDownloaderAPI(key, info);
	else if (!wcscmp(name, L"Security"))
		return new JSAPI2::SecurityAPI(key, info);
	else if (!wcscmp(name, L"Bookmarks"))
		return new JSAPI2::BookmarksAPI(key, info);
	else if (!wcscmp(name, L"Application"))
		return new JSAPI2::ApplicationAPI(key, info);
	else if (!wcscmp(name, L"Skin"))
		return new JSAPI2::SkinAPI(key, info);
	else if (!wcscmp(name, L"MediaCore"))
		return new JSAPI2::MediaCoreAPI(key, info);
	else
		return 0;
}

static int GetDescription(const wchar_t *group, const wchar_t *action, wchar_t *str, size_t str_len, int *flags)
{
	if (!wcscmp(group, L"application"))
	{
		if (action && !wcscmp(action, L"launchurl"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_APPLICATION_LAUNCHURL, NULL, 0));
		}
		else
		return 1;
		return 0;

	}
	else if (!wcscmp(group, L"transport"))
	{
		if (action && !wcscmp(action, L"events"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_TRANSPORT_EVENTS, NULL, 0));
		}
		else if (action && !wcscmp(action, L"metadata"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_TRANSPORT_METADATA, NULL, 0));
		}
		else if (action && !wcscmp(action, L"controls"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_TRANSPORT_CONTROLS, NULL, 0));
		}
		else
			return 1;

		return 0;
	}
	else if(!wcscmp(group, L"player"))
	{
		if (action && !wcscmp(action, L"playlist"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_PLAYER_PLAYLIST, NULL, 0));
		}
		else if (action && !wcscmp(action, L"metadata"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_PLAYER_METADATA, NULL, 0));
		}
		else
			return 1;
		return 0;
	}
	else if (!wcscmp(group, L"downloader"))
	{
		if (action && !wcscmp(action, L"events"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_DOWNLOADER_EVENTS, NULL, 0));
		}
		else if (action && !wcscmp(action, L"downloadmedia"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_DOWNLOADER_DOWNLOADMEDIA, NULL, 0));
		}
		else
			return 1;

		return 0;
	}
	else if (!wcscmp(group, L"security"))
	{
		*flags = JSAPI2::svc_apicreator::AUTHORIZATION_FLAG_GROUP_ONLY;
		StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_SECURITY_AUTH, NULL, 0));
		return 0;
	}
	else if (!wcscmp(group, L"bookmarks"))
	{
		*flags = JSAPI2::svc_apicreator::AUTHORIZATION_FLAG_GROUP_ONLY;
		StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_BOOKMARKS_AUTH, NULL, 0));
		return 0;
	}
	else if (!wcscmp(group, L"skin"))
	{
		*flags = JSAPI2::svc_apicreator::AUTHORIZATION_FLAG_GROUP_ONLY;
		StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_SKIN_AUTH, NULL, 0));
		return 0;
	}
	else if (!wcscmp(group, L"mediacore"))
	{
		if (action && !wcscmp(action, L"metadatahook"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_MEDIACORE_METADATAHOOK, NULL, 0));
		}
		else if (action && !wcscmp(action, L"metadata"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_MEDIACORE_METADATA, NULL, 0));
		}
		else if (action && !wcscmp(action, L"extensions"))
		{
			*flags = 0;
			StringCchCopyW(str, str_len, getStringW(IDS_SECURITY_MEDIACORE_EXTENSIONS, NULL, 0));
		}
		else
			return 1;
		return 0;
	}
	else
		return 1;


}



int JSAPI2_Creator::PromptForAuthorization(HWND parent, const wchar_t *group, const wchar_t *action, const wchar_t *authorization_key, JSAPI2::api_security::AuthorizationData *data)
{
	int flags = 0;
	wchar_t display_str[1024] = {0};
	if (GetDescription(group, action, display_str, 1024, &flags) == 0)
	{
		const wchar_t *title_str = JSAPI2::security.GetAssociatedName(authorization_key);
		return JSAPI2::security.SecurityPrompt(parent, title_str, display_str, flags);
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
static const char serviceName[] = "Winamp Javascript Objects";

// {53CCACEF-1EFE-4060-8D09-329AD0D4F9C4}
static const GUID jsapi2_factory_guid = 
{ 0x53ccacef, 0x1efe, 0x4060, { 0x8d, 0x9, 0x32, 0x9a, 0xd0, 0xd4, 0xf9, 0xc4 } };



FOURCC JSAPI2CreatorFactory::GetServiceType()
{
	return jsapi2_svc.getServiceType();
}

const char *JSAPI2CreatorFactory::GetServiceName()
{
	return serviceName;
}

GUID JSAPI2CreatorFactory::GetGUID()
{
	return jsapi2_factory_guid;
}

void *JSAPI2CreatorFactory::GetInterface(int global_lock)
{
	//	if (global_lock)
	//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return &jsapi2_svc;
}

int JSAPI2CreatorFactory::SupportNonLockingInterface()
{
	return 1;
}

int JSAPI2CreatorFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	return 1;
}

const char *JSAPI2CreatorFactory::GetTestString()
{
	return 0;
}

int JSAPI2CreatorFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS JSAPI2CreatorFactory
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