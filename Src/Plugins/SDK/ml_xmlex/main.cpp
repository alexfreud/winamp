#include "main.h"
#include "../Winamp/wa_ipc.h"
#include "resource.h"
#include "api.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include <strsafe.h>
#include "..\..\General\gen_ml\menu.h"
#include "..\..\General\gen_ml\ml_ipc_0313.h"

#define PLUGIN_VERSION L"1.1"

int Init();
void Quit();
UINT_PTR xmlex_treeItem = 0;
api_service *serviceManager = 0;

EXTERN_C winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
		"Nullsoft XML Reader",
		Init,
		Quit,
		xmlex_pluginMessageProc,
		0,
		0,
		0,
};

int Init() 
{
	//starting point for wasabi, where services are shared
	WASABI_API_SVC = (api_service *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	
	// waServiceFactory *sf = plugin.service->service_getServiceByGuid(languageApiGUID);
	// if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());
	
	// wasabi based services for localisation support
	// api_language* WASABI_API_LNG = 0;
	// HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
	
	// need to have this initialised before we try to do anything with localisation features
	// WASABI_API_START_LANG(plugin.hDllInstance,MlImpexLangGUID);

	// static wchar_t szDescription[256];
	// StringCchPrintf(szDescription, ARRAYSIZE(szDescription),
	//				WASABI_API_LNGSTRINGW(IDS_NULLSOFT_XMLEX), PLUGIN_VERSION);
	// plugin.description = (char*)szDescription;

	//set up tree item, gen_ml will call xmlex_pluginMessageProc if/when the treeview item gets selected
	MLTREEITEMW newTree;
	newTree.size = sizeof(MLTREEITEMW);
	newTree.parentId = 0;
	newTree.title = L"XML Example"; 
	newTree.hasChildren = 0;
	newTree.id = 0;
	SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM) &newTree, ML_IPC_TREEITEM_ADDW);
	xmlex_treeItem = newTree.id;	
	return 0; // 0 for success.  returning non-zero will cause gen_ml to abort loading your plugin
}

void Quit() 
{
}

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}