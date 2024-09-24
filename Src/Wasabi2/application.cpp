#include <stdio.h>
#include "main.h"
#include "api__wasabi2.h"
#include "application.h"
#include <nx/nxstring.h>

#include <foundation/error.h>
#include <shlobj.h>
#include <nswasabi/ReferenceCounted.h>
#include <strsafe.h>
#include <Replicant/version.h>


Application::Application() 
{
//	string_heap = 0;
	user_agent[0]=0;
	version_string=0;
	build_number=0;
}

Application::~Application()
{
	NXStringRelease(version_string);
}

int Application::Init()
{
	int ret = ApplicationBase::Initialize();
	if (ret != NErr_Success)
		return ret;
#if 0
	string_heap = HeapCreate(0, 0, 0);
	if (!string_heap)
		return INIT_ERROR_STRING_HEAP;

	ULONG argh = 2;
	HeapSetInformation(string_heap, HeapCompatibilityInformation, &argh, sizeof(ULONG));

	NXStringSetHeap(string_heap);

	
#endif
/* set the device id */
	GUID winamp_id;
	WASABI_API_APP->GetUserID(&winamp_id);
	ReferenceCountedNXString device_id;
	NXStringCreateWithFormatting(&device_id, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
				(int)winamp_id.Data1, (int)winamp_id.Data2, (int)winamp_id.Data3, (int)winamp_id.Data4[0],
				(int)winamp_id.Data4[1], (int)winamp_id.Data4[2], (int)winamp_id.Data4[3],
				(int)winamp_id.Data4[4], (int)winamp_id.Data4[5], (int)winamp_id.Data4[6], (int)winamp_id.Data4[7]);

	ApplicationBase::SetDeviceID(device_id);

	/* set the data path */
	const wchar_t *settings = WASABI_API_APP->path_getUserSettingsPath();
	ReferenceCountedNXURI settings_uri;
	NXURICreateWithUTF16(&settings_uri, settings);
	ApplicationBase::SetDataPath(settings_uri);

	build_number = WASABI_API_APP->main_getBuildNumber();
	const wchar_t *version_number_string = WASABI_API_APP->main_getVersionNumString();
	NXStringCreateWithUTF16(&version_string, version_number_string);

	OSVERSIONINFO info;
	info.dwOSVersionInfoSize=sizeof(info);
	GetVersionExW(&info);
	StringCbPrintfA(user_agent, sizeof(user_agent), "Winamp/%S (Windows NT %u.%u) Replicant/%s", WASABI_API_APP->main_getVersionNumString(), info.dwMajorVersion, info.dwMinorVersion, replicant_version);
	ApplicationBase::EnableAllPermissions();
	return NErr_Success;
}



const char *Application::Application_GetUserAgent()
{ 
	return user_agent; 
}

unsigned int Application::Application_GetBuildNumber() 
{
	return build_number;
}

int Application::Application_GetVersionString(nx_string_t *version)
{
	*version = NXStringRetain(version_string);
	return NErr_Success;
}

int Application::Application_GetProductShortName(nx_string_t *name)
{
	return NXStringCreateWithUTF8(name, "Winamp");
}