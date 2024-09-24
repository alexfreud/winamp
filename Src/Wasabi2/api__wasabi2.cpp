#include "Winamp5ArtworkManager.h"
#include "main.h"
#include "api__wasabi2.h"
#include "application.h"
#include <Wasabi/Wasabi.h>
#include <nswasabi/singleton.h>
#include <component/win/ComponentManager.h>
#include <Replicant/metadata/metadata.h>
#include <Shlwapi.h>


Application application;
static ComponentManager component_manager;

static SingletonServiceFactory<Application, api_application> application_factory;
static SingletonService<Winamp5ArtworkManager, api_artwork> artwork_factory;

static void AddComponents(const wchar_t *directory)
{
	nx_uri_t uri_directory;
	if (NXURICreateWithUTF16(&uri_directory, directory) == NErr_Success)
	{
		component_manager.AddDirectory(uri_directory);
		NXURIRelease(uri_directory);
	}
}

void Replicant_Initialize()
{
	application.Init();
	if (Wasabi_Init() == NErr_Success)
	{
		application_factory.Register(WASABI2_API_SVC, WASABI2_API_APP);
		artwork_factory.Register(WASABI2_API_SVC);
		component_manager.SetServiceAPI(WASABI2_API_SVC);
		Replicant_Metadata_Initialize(WASABI2_API_SVC);

		wchar_t PROG_DIR[MAX_PATH] = {0};
		GetModuleFileName(0, PROG_DIR, MAX_PATH);
		PathRemoveFileSpec(PROG_DIR);
		PathAppend(PROG_DIR, L"Components");
		AddComponents(PROG_DIR);
		component_manager.Load();
	}
}