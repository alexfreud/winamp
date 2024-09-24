#include "ComponentManager.h"
#include "foundation/error.h"
#include "nx/nxuri.h"

int ComponentManager::AddComponent(nx_uri_t filename)
{
	if (phase > PHASE_LOADED)
		return NErr_Error;

	HMODULE hLib = LoadLibraryW(filename->string);
	if (hLib)
	{
		GETCOMPONENT_FUNC pr = (GETCOMPONENT_FUNC)GetProcAddress(hLib, "GetWasabi2Component");
		if (pr)
		{
			ifc_component *component = pr();
			if (component)
			{
				if (component->component_info.wasabi_version != wasabi2_component_version
					|| component->component_info.nx_api_version != nx_api_version
					|| component->component_info.nx_platform_guid != nx_platform_guid)
				{
					FreeLibrary(hLib);
					return NErr_IncompatibleVersion;
				}

				component->component_info.hModule = hLib;
				component->component_info.filename = NXURIRetain(filename);
				int ret = component->Initialize(service_api);
				if (ret != NErr_Success)
				{
					NXURIRelease(component->component_info.filename);
					FreeLibrary(hLib);
					return ret;
				}

				/* if the component was added late, we'll need to run some extra stages */
				ret = LateLoad(component);
				if (ret != NErr_Success)
				{
					NXURIRelease(component->component_info.filename);
					FreeLibrary(hLib);
					return ret;
				}

				components.push_back(component);
				return NErr_Success;
			}
		}
		return NErr_Error;
	}
	else
	{
		return NErr_FileNotFound;
	}
}

int ComponentManager::AddDirectory(nx_uri_t directory)
{
	WIN32_FIND_DATAW find_data = {0};

	nx_uri_t directory_mask;
	int ret = NXURICreateFromPath(&directory_mask, L"*.w6c", directory);
	if (ret != NErr_Success)
		return ret;

	HANDLE find_handle = FindFirstFileW(directory_mask->string, &find_data);
	if (find_handle != INVALID_HANDLE_VALUE)
	{
		do
		{			
			nx_uri_t w6c_filename;
			if (NXURICreateFromPath(&w6c_filename, find_data.cFileName, directory) == NErr_Success)
			{
				AddComponent(w6c_filename);
				NXURIRelease(w6c_filename);
			}
		}
		while (FindNextFileW(find_handle,&find_data));
		FindClose(find_handle);
	}
	return NErr_Success;
}

void ComponentManager::CloseComponent(ifc_component *component)
{
	FreeLibrary(component->component_info.hModule);
}