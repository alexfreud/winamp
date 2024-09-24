#include "main.h"
#include <vector>

//#include <crtdbg.h>
#include <strsafe.h>

typedef std::vector<PluginUnloadCallback> UnloadCallbackList;

static int Plugin_Init();
static void Plugin_Quit();

static INT_PTR 
Plugin_MessageProc(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3);

extern "C"  winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_devices.dll)",
    Plugin_Init,
    Plugin_Quit,
    Plugin_MessageProc,
    0,
    0,
    0,
};

static UnloadCallbackList *unloadCallbacks = NULL;
static DeviceImageCache *imageCache = NULL;
static HWND eventRelayWindow = NULL;

HINSTANCE 
Plugin_GetInstance(void)
{
	return plugin.hDllInstance;
}

HWND 
Plugin_GetWinampWindow(void)
{
	return plugin.hwndWinampParent;
}

HWND 
Plugin_GetLibraryWindow(void)
{
	return plugin.hwndLibraryParent;
}

static void
Plugin_SetDescription()
{
	static wchar_t szDescription[256];
	StringCchPrintf(szDescription, ARRAYSIZE(szDescription),
					WASABI_API_LNGSTRINGW(IDS_PLUGIN_NAME),
					PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
	plugin.description = (char*)szDescription;
}

static int Plugin_Init()
{	
	unloadCallbacks = NULL;

//	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | 
//					_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF);

	if (FALSE == Wasabi_InitializeFromWinamp(plugin.hDllInstance, plugin.hwndWinampParent))
		return ML_INIT_FAILURE;

	Wasabi_LoadDefaultServices();

	Plugin_SetDescription();

	imageCache = DeviceImageCache_Create();

	if (NULL == eventRelayWindow)
	{
		eventRelayWindow = EventRelay_CreateWindow();
		if (NULL == eventRelayWindow)
			return 2;
	}
	
	if (FALSE == Navigation_Initialize())
	{
		if (NULL != eventRelayWindow)
		{
			DestroyWindow(eventRelayWindow);
			eventRelayWindow = NULL;
		}

		Wasabi_Release();
		return 3;
	}
	
	DeviceCommands_Register();

	return ML_INIT_SUCCESS;
}

static void Plugin_Quit()
{	
	if (NULL != unloadCallbacks)
	{
		size_t index = unloadCallbacks->size();
		while(index--)
			unloadCallbacks->at(index)();
		delete(unloadCallbacks);
	}

	if (NULL != eventRelayWindow)
	{
		DestroyWindow(eventRelayWindow);
		eventRelayWindow = NULL;
	}

	Navigation_Uninitialize();

	if (NULL != imageCache)
	{
		DeviceImageCache_Free(imageCache);
		imageCache = NULL;
	}

	Wasabi_Release();
}

static INT_PTR 
Plugin_MessageProc(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	INT_PTR result = 0;

	if (FALSE != Navigation_ProcessMessage(msg, param1, param2, param3, &result))
		return result;
	
	return FALSE;
}

BOOL 
Plugin_RegisterUnloadCallback(PluginUnloadCallback callback)
{
	if (NULL == callback)
		return FALSE;

	if (NULL == unloadCallbacks)
	{
		unloadCallbacks = new UnloadCallbackList();
		if (NULL == unloadCallbacks)
			return FALSE;
	}

	unloadCallbacks->push_back(callback);
	return TRUE;
}

DeviceImageCache *
Plugin_GetImageCache()
{
	return imageCache;
}

HWND
Plugin_GetEventRelayWindow()
{
	return eventRelayWindow;
}

const wchar_t *
Plugin_GetDefaultDeviceImage(unsigned int width, unsigned int height)
{
	const ImageInfo *image;
	const ImageInfo deviceImages[] = 
	{ 
		{16, 16, MAKEINTRESOURCE(IDR_GENERIC_DEVICE_16x16_IMAGE)},
		{160, 160, MAKEINTRESOURCE(IDR_GENERIC_DEVICE_160x160_IMAGE)},
	};

	image = Image_GetBestFit(deviceImages, ARRAYSIZE(deviceImages), width, height);
	if (NULL == image)
		return NULL;

	return image->path;
}

HRESULT 
Plugin_EnsurePathExist(const wchar_t *path)
{
	unsigned long errorCode;
	unsigned int errorMode;
	
	errorCode = ERROR_SUCCESS;
	errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

	if (0 == CreateDirectory(path, NULL))
	{
		errorCode = GetLastError();
		if (ERROR_PATH_NOT_FOUND == errorCode)
		{
			const wchar_t *block, *cursor;
			wchar_t buffer[MAX_PATH] = {0};
			
			block = path;
			cursor = PathFindNextComponent(block);

			errorCode = (cursor == block || 
						S_OK != StringCchCopyN(buffer, ARRAYSIZE(buffer), block, (cursor - block))) ?
						ERROR_INVALID_NAME : ERROR_SUCCESS;
			
			block = cursor;
			
			while(ERROR_SUCCESS == errorCode && 
				  NULL != (cursor = PathFindNextComponent(block)))
			{
				if (cursor == block || 
					S_OK != StringCchCatN(buffer, ARRAYSIZE(buffer), block, (cursor - block)))
				{
					errorCode = ERROR_INVALID_NAME;
				}

				if (ERROR_SUCCESS == errorCode && 
					FALSE == CreateDirectory(buffer, NULL))
				{
					errorCode = GetLastError();
					if (ERROR_ALREADY_EXISTS == errorCode) 
						errorCode = ERROR_SUCCESS;
				}
				block = cursor;
			}
		}

		if (ERROR_ALREADY_EXISTS == errorCode) 
			errorCode = ERROR_SUCCESS;
	}

	SetErrorMode(errorMode);
	SetLastError(errorCode);
	return HRESULT_FROM_WIN32(errorCode);
}

BOOL
Plugin_GetResourceString(const wchar_t *resourceName, const wchar_t *resourceType, wchar_t *buffer, size_t bufferMax)
{
	unsigned long filenameLength;

	if (NULL == resourceName)
		return FALSE;

	if (FAILED(StringCchCopyEx(buffer, bufferMax, L"res://", &buffer, &bufferMax, 0)))
		return FALSE;

	filenameLength = GetModuleFileName(Plugin_GetInstance(), buffer, (DWORD)bufferMax);
	if (0 == filenameLength || bufferMax == filenameLength)
		return FALSE;

	buffer += filenameLength;
	bufferMax -= filenameLength;

	if (NULL != resourceType)
	{
		if (FALSE != IS_INTRESOURCE(resourceType))
		{
			if (FAILED(StringCchPrintfEx(buffer, bufferMax, &buffer, &bufferMax, 0, L"/#%d", (int)(INT_PTR)resourceType)))
				return FALSE;
		}
		else
		{
			if (FAILED(StringCchPrintfEx(buffer, bufferMax, &buffer, &bufferMax, 0, L"/%s", resourceType)))
				return FALSE;
		}
	}

	if (FALSE != IS_INTRESOURCE(resourceName))
	{
		if (FAILED(StringCchPrintfEx(buffer, bufferMax, &buffer, &bufferMax, 0, L"/#%d", (int)(INT_PTR)resourceName)))
			return FALSE;
	}
	else
	{
		if (FAILED(StringCchPrintfEx(buffer, bufferMax, &buffer, &bufferMax, 0, L"/%s", resourceName)))
			return FALSE;
	}

	return TRUE;
}

HMENU 
Plugin_LoadMenu()
{
	return WASABI_API_LOADMENUW(IDR_PLUGIN_MENU);
}

BOOL
Plugin_ShowHelp()
{
	BOOL result; 
	wchar_t buffer[8192] = {0};

	WASABI_API_LNGSTRINGW_BUF(IDS_PLUGIN_HELP_URL, buffer, ARRAYSIZE(buffer));
	if (L'\0' == buffer[0])
	{
		if (FAILED(StringCchCopy(buffer, ARRAYSIZE(buffer), 
					L"https://help.winamp.com/hc/articles/8106455294612-Winamp-Portables-Guide")))
		{
			return FALSE;
		}
	}

	result = MediaLibrary_ShowHelp(plugin.hwndLibraryParent, buffer);

	return result;
}

BOOL
Plugin_BeginDiscovery()
{
	if (NULL == WASABI_API_DEVICES ||
		FAILED(WASABI_API_DEVICES->BeginDiscovery()))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL 
Plugin_OpenUrl(HWND ownerWindow, const wchar_t *url, BOOL forceExternal)
{	
	BOOL result;
	HCURSOR cursor;

	if (NULL == WASABI_API_WINAMP)
		return FALSE;

	cursor = LoadCursor(NULL, IDC_APPSTARTING);
	if (NULL != cursor) 
		cursor = SetCursor(cursor);

	if (FALSE != forceExternal)
	{
		HINSTANCE instance = ShellExecute(ownerWindow, L"open", url, NULL, NULL, SW_SHOWNORMAL);
		result = ((INT_PTR)instance > 32) ? TRUE: FALSE;
	}
	else
	{
		HRESULT hr = WASABI_API_WINAMP->OpenUrl(ownerWindow, url);
		result = SUCCEEDED(hr);
	}
		
	if (NULL != cursor) 
		SetCursor(cursor);

	return result;
}

EXTERN_C __declspec(dllexport) winampMediaLibraryPlugin *
winampGetMediaLibraryPlugin()
{
	return &plugin;
}