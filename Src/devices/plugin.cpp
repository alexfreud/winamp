#include "main.h"
#include "./plugin.h"
#include "./component.h"

static HINSTANCE pluginInstance = NULL;
static DevicesComponent component;

HINSTANCE 
Plugin_GetInstance()
{
	return pluginInstance;
}

extern "C" __declspec(dllexport) ifc_wa5component *
GetWinamp5SystemComponent()
{
	return &component;
}


BOOL APIENTRY 
DllMain(HANDLE hModule, DWORD  uReason, void *reserved)
{
    switch(uReason) 
	{
		case DLL_PROCESS_ATTACH:
			pluginInstance = (HINSTANCE)hModule;
			break;
    }
    return TRUE;
}
