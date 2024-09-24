#include "main.h"
#include "./navigation.h"
#include "./wasabi.h"
#include "./resource.h"
#include "./external.h"
#include "./serviceHost.h"
#include "./import.h"
#include "../winamp/wa_ipc.h"


#include <initguid.h>
#include <strsafe.h>

static DWORD externalCookie = 0;
static Navigation *navigation = NULL;

// {BF4F80A7-7470-4b08-8A4C-34382C146202}
DEFINE_GUID(WebDevLangUid, 0xbf4f80a7, 0x7470, 0x4b08, 0x8a, 0x4c, 0x34, 0x38, 0x2c, 0x14, 0x62, 0x2);

static int Plugin_Init();
static void Plugin_Quit();
static INT_PTR Plugin_MessageProc(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3);

extern "C" winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_webdev.dll)",
    Plugin_Init,
    Plugin_Quit,
    Plugin_MessageProc,
    0,
    0,
    0,
};

HINSTANCE Plugin_GetInstance(void)
{
	return plugin.hDllInstance;
}

HWND Plugin_GetWinamp(void)
{
	return plugin.hwndWinampParent;
}

HWND Plugin_GetLibrary(void)
{
	return plugin.hwndLibraryParent;
}

HRESULT Plugin_GetExternal(ExternalDispatch **ppExternal)
{
	if (NULL == ppExternal) return E_POINTER;
	HRESULT hr = ExternalDispatch::CreateInstance(ppExternal);
	return hr;
}

static int Plugin_Init()
{
	if (!WasabiApi_Initialize(Plugin_GetInstance()))
		return 1;

	if (NULL == OMBROWSERMNGR ||
		NULL == OMSERVICEMNGR ||
		NULL == OMUTILITY )
	{
		return 2;
	}

	if (NULL != WASABI_API_LNG)
	{
		static wchar_t szDescription[256];
		StringCchPrintf(szDescription, ARRAYSIZE(szDescription),
						WASABI_API_LNGSTRINGW(IDS_PLUGIN_NAME),
						PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
		plugin.description = (char*)szDescription;
	}
	
	ExternalDispatch *externalDispatch;
	if (SUCCEEDED(ExternalDispatch::CreateInstance(&externalDispatch)))
	{
		DispatchInfo dispatchInfo;
		dispatchInfo.id = 0;
		dispatchInfo.name =(LPWSTR)externalDispatch->GetName();
		dispatchInfo.dispatch = externalDispatch;

		if (0 == SENDWAIPC(Plugin_GetWinamp(), IPC_ADD_DISPATCH_OBJECT, (WPARAM)&dispatchInfo))
			externalCookie = dispatchInfo.id;

		externalDispatch->Release();
	}
	
	if (NULL == navigation)
	{
		if (FAILED(Navigation::CreateInstance(&navigation)))
		{
			navigation = NULL;

			if (0 != externalCookie)
			{
				HWND hWinamp = Plugin_GetWinamp();
				SENDWAIPC(hWinamp, IPC_REMOVE_DISPATCH_OBJECT, (WPARAM)externalCookie);
				externalCookie = 0;
			}

			return 2;
		}
	}
	
	return 0;
}

static void Plugin_Quit()
{	
	if (0 != externalCookie)
	{
		HWND hWinamp = Plugin_GetWinamp();
		SENDWAIPC(hWinamp, IPC_REMOVE_DISPATCH_OBJECT, (WPARAM)externalCookie);
		externalCookie = 0;
	}

	WebDevServiceHost::ReleseCache();
	
	if (NULL != navigation)
	{
		navigation->Finish();
		navigation->Release();
		navigation = NULL;
	}

	ImportService_SaveRecentUrl();

	WasabiApi_Release();
}

static INT_PTR Plugin_MessageProc(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	INT_PTR result = 0;
	if (NULL != navigation && 
		FALSE != navigation->ProcessMessage(msg, param1, param2, param3, &result))
	{
		return result;
	}
	
	return FALSE;
}

HRESULT Plugin_GetNavigation(Navigation **instance)
{
	if(NULL == instance) return E_POINTER;

	if (NULL == navigation) 
	{
		*instance = NULL;
		return E_UNEXPECTED;
	}

	*instance = navigation;
	navigation->AddRef();

	return S_OK;
}

EXTERN_C __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}