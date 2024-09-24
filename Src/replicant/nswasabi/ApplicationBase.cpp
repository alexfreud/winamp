#include "ApplicationBase.h"
#include "foundation/error.h"
#include "application/features.h"
#include <stdio.h> // for sprintf
#ifdef __ANDROID__
#include <android/log.h>
#endif

ApplicationBase::ApplicationBase()
{
	data_path = 0;
	all_permissions_enabled = false;
	device_id = 0;
}

ApplicationBase::~ApplicationBase()
{
    NXURIRelease(data_path);
	data_path = 0;
	NXStringRelease(device_id);
	device_id = 0;
}

int ApplicationBase::Initialize()
{
	return NErr_Success;
}

/* and call this after doing your own shutdown */
void ApplicationBase::Shutdown()
{
	NXURIRelease(data_path);
	data_path = 0;
}

void ApplicationBase::SetDataPath(nx_uri_t new_data_path)
{
	nx_uri_t old_path = data_path;
	data_path = NXURIRetain(new_data_path);
	NXURIRelease(old_path);
}

void ApplicationBase::SetPermission(GUID feature)
{
	permissions.insert(feature);
}

void ApplicationBase::RemovePermission(GUID permission)
{
	permissions.erase(permission);
}

void ApplicationBase::EnableAllPermissions()
{
	all_permissions_enabled=true;
}

void ApplicationBase::ClearPermissions()
{
	permissions.clear();
}

void ApplicationBase::NotifyPermissions(api_syscb *system_callbacks)
{
	if (system_callbacks)
		system_callbacks->IssueCallback(Features::event_type,	Features::permissions_changed);
}

int ApplicationBase::Application_GetDataPath(nx_uri_t *path)
{
	*path=NXURIRetain(data_path);
	if (data_path)
		return NErr_Success;
	else
		return NErr_Empty;
}

int ApplicationBase::Application_GetPermission(GUID feature)
{
	if (all_permissions_enabled)
		return NErr_True;
	else if (permissions.find(feature) == permissions.end())
		return NErr_False;
	else
		return NErr_True;
}

int ApplicationBase::Application_GetFeature(GUID feature)
{
	if (features.find(feature) == features.end())
		return NErr_False;
	else
		return NErr_True;
}

void ApplicationBase::Application_SetFeature(GUID feature)
{
	features.insert(feature);
}

void ApplicationBase::SetDeviceID(nx_string_t device_id)
{
	nx_string_t old = this->device_id;
	this->device_id = NXStringRetain(device_id);
	NXStringRelease(old);
}

static void GUIDtoCString(const GUID &guid, char *target) 
{
	//{2E9CE2F8-E26D-4629-A3FF-5DF619136B2C}
	sprintf(target, "{%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x}",
		(int)guid.Data1, (int)guid.Data2, (int)guid.Data3,
		(int)guid.Data4[0], (int)guid.Data4[1],
		(int)guid.Data4[2], (int)guid.Data4[3],
		(int)guid.Data4[4], (int)guid.Data4[5],
		(int)guid.Data4[6], (int)guid.Data4[7] );

}

static const char *GetFeatureName(const GUID &guid)
{
	if (guid == Features::aac_playback)
		return "AAC Playback";
	else if (guid == Features::gapless)
		return "Gapless Playback";
	else if (guid == Features::flac_playback)
		return "FLAC Playback";
	else if (guid == Features::gracenote_autotag)
		return "Gracenote Autotagger";
	else 
		return 0; /* the lack of of return 0 by default here was why it was crashing */

}

void ApplicationBase::DumpPermissions()
{
#ifdef __ANDROID__
	char guid_string[64];
	FeatureList::iterator itr;
	for (itr=features.begin();itr!=features.end();itr++)
	{
		GUIDtoCString(*itr, guid_string);
		const char *feature_name = GetFeatureName(*itr);
		if (feature_name)
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Feature] %s (%s)", guid_string, feature_name);
		else
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Feature] %s", guid_string);
	}

	for (itr=permissions.begin();itr!=permissions.end();itr++)
	{
		GUIDtoCString(*itr, guid_string);
		const char *feature_name = GetFeatureName(*itr);
		if (feature_name)
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Permission] %s (%s)", guid_string, feature_name);
		else
			__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[Permission] %s", guid_string);
	}
#endif
}

int ApplicationBase::Application_GetDeviceID(nx_string_t *value)
{
	if (!device_id)
		return NErr_Empty;
	*value = NXStringRetain(device_id);
	return NErr_Success;
}
