#include "workorder.h"
#include "main.h"
#include "cddb.h"
#include <shlwapi.h>
CDDBModuleWorkOrderManagerInterface *workorder=0;
static HMODULE musicIDLib = 0;

void OpenMusicIDWorkOrder()
{
	if (!workorder)
	{
		char pluginpath[MAX_PATH] = {0};
		GetModuleFileNameA(line.hDllInstance, pluginpath, MAX_PATH);
		PathRemoveFileSpecA(pluginpath);
		PathAppendA(pluginpath, "Gracenote");

		char musicidpath[MAX_PATH] = {0};
		PathCombineA(musicidpath, pluginpath, "CddbWOManagerWinamp.dll");

		musicIDLib = LoadLibraryA(musicidpath);
		if (musicIDLib)
		{
			CDDBModuleQueryInterfaceFunc qi = (CDDBModuleQueryInterfaceFunc)GetProcAddress(musicIDLib, "CDDBModuleQueryInterface");
			if (qi)
			{
				ICDDBControl *pControl;
				Cddb_GetIControl((void**)&pControl);
				workorder = (CDDBModuleWorkOrderManagerInterface*)qi("workordermanager");
				if (!(workorder && workorder->base.version == CDDBMODULE_VERSION && workorder->version == CDDBMODULE_WORKORDER_MGR_VERSION
					&& workorder->base.Init && workorder->base.Init(0) 
					&& workorder->Initialize(pControl, pluginpath) == 0))
				{
					workorder = 0;
					FreeLibrary(musicIDLib);
					musicIDLib=0;
				}
				if (pControl) pControl->Release();
			}
		}
	}
}

void ShutdownMusicIDWorkOrder()
{
	if (workorder)
		workorder->Shutdown();

	workorder=0;

	if (musicIDLib)
		FreeLibrary(musicIDLib);
	musicIDLib=0;
}