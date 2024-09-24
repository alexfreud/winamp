#include "../../Library/ml_pmp/pmp.h"
#include "../Winamp/wa_ipc.h"
#include "device.h"
#include "api.h"
#include "main.h"
#include "nu/ns_wc.h"
#include "resource.h"
#include <shlwapi.h>
#include <strsafe.h>

#define PLUGIN_VERSION L"1.56"
int winampVersion = 0;
ifc_devicesupportedcommandenum *command_enum=0;
ifc_devicesupportedcommandstore *command_store=0;
ifc_deviceeventmanager *device_event_manager;
char winamp_name[260] = {0};
char winamp_id_str[40] = {0};
wchar_t inifile[MAX_PATH] = {0};
GUID winamp_id = GUID_NULL;
static int Init();
static void Quit();
static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);

PMPDevicePlugin plugin = {PMPHDR_VER,0,Init,Quit,MessageProc};
void StartListenServer();


BOOL FormatResProtocol(const wchar_t *resourceName, const wchar_t *resourceType, wchar_t *buffer, size_t bufferMax)
{
	unsigned long filenameLength;

	if (NULL == resourceName)
		return FALSE;

	if (FAILED(StringCchCopyExW(buffer, bufferMax, L"res://", &buffer, &bufferMax, 0)))
		return FALSE;

	filenameLength = GetModuleFileNameW(plugin.hDllInstance, buffer, (DWORD)bufferMax);
	if (0 == filenameLength || bufferMax == filenameLength)
		return FALSE;

	buffer += filenameLength;
	bufferMax -= filenameLength;

	if (NULL != resourceType)
	{
		if (FALSE != IS_INTRESOURCE(resourceType))
		{
			if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/#%d", (int)(INT_PTR)resourceType)))
				return FALSE;
		}
		else
		{
			if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/%s", resourceType)))
				return FALSE;
		}
	}

	if (FALSE != IS_INTRESOURCE(resourceName))
	{
		if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/#%d", (int)(INT_PTR)resourceName)))
			return FALSE;
	}
	else
	{
		if (FAILED(StringCchPrintfExW(buffer, bufferMax, &buffer, &bufferMax, 0, L"/%s", resourceName)))
			return FALSE;
	}

	return TRUE;
}


class WifiDeviceConnection : public ifc_deviceconnection
{
public:
	WifiDeviceConnection()
	{
	}
	const char *GetName()
	{ 
		return "wifi"; 
	}

	HRESULT GetIcon(wchar_t *buffer, size_t bufferMax, int width, int height)
	{
		if(FALSE == FormatResProtocol(MAKEINTRESOURCE(IDB_WIFI), L"PNG", buffer, bufferMax))
			return E_FAIL;
		
		return S_OK;
	}

	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferMax)
	{
		if (NULL == buffer)
			return E_POINTER;

		WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_CONNECTION_WIFI, buffer, bufferMax);
		return S_OK;
	}
protected:

#define CBCLASS WifiDeviceConnection
	START_DISPATCH_INLINE;
	CB(API_GETNAME, GetName);
	CB(API_GETICON, GetIcon);
	CB(API_GETDISPLAYNAME, GetDisplayName);
	END_DISPATCH;
#undef CBCLASS
};

class AttachCommand : public ifc_devicecommand
{
public:
	const char *GetName()
	{
		return "attach";
	}
	
	HRESULT GetIcon(wchar_t *buffer, size_t bufferMax, int width, int height)
	{
		int resourceId;

		if (width <= 16 && height <= 16)
			resourceId = IDB_ATTACH_16;
		else
			resourceId = IDB_ATTACH;
		
		if(FALSE == FormatResProtocol(MAKEINTRESOURCE(resourceId), L"PNG", buffer, bufferMax))
			return E_FAIL;
		
		return S_OK;
	}

	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferMax)
	{
		if (NULL == buffer)
			return E_POINTER;

		WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_CMD_ATTACH, buffer, bufferMax);
		return S_OK;
	}
	
	HRESULT GetDescription(wchar_t *buffer, size_t bufferMax)
	{
		if (NULL == buffer)
			return E_POINTER;

		WASABI_API_LNGSTRINGW_BUF(IDS_DEVICE_CMD_ATTACH_DESC, buffer, bufferMax);
		return S_OK;
	}

#define CBCLASS AttachCommand
	START_DISPATCH_INLINE;
	CB(API_GETNAME, GetName);
	CB(API_GETICON, GetIcon);
	CB(API_GETDISPLAYNAME, GetDisplayName);
	CB(API_GETDESCRIPTION, GetDescription);
	END_DISPATCH;
#undef CBCLASS
};

class DeviceCommand :  public Countable<ifc_devicesupportedcommand>
{
public:
	DeviceCommand(const char *name, DeviceCommandFlags flags);

public:
	const char *GetName();
	HRESULT GetFlags(DeviceCommandFlags *flags);
	REFERENCE_COUNT_IMPLEMENTATION;

public:
	const char *name;
	DeviceCommandFlags flags;
RECVS_DISPATCH;
};


static AttachCommand attach_command;
static WifiDeviceConnection wifi_connection;
static int Init() 
{
	winampVersion = (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETVERSION);
	WasabiInit();

	if (!AGAVE_API_DEVICEMANAGER)
		return 1;
	WASABI_API_APP->GetUserID(&winamp_id);
	StringCbPrintfA(winamp_id_str, sizeof(winamp_id_str), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", (int)winamp_id.Data1, (int)winamp_id.Data2, (int)winamp_id.Data3, (int)winamp_id.Data4[0], (int)winamp_id.Data4[1], (int)winamp_id.Data4[2], (int)winamp_id.Data4[3], (int)winamp_id.Data4[4], (int)winamp_id.Data4[5], (int)winamp_id.Data4[6], (int)winamp_id.Data4[7] );
	 
	wchar_t user_name[128] = {0};
	wchar_t computer_name[128] = {0};
	DWORD buffer_size_user = 128, buffer_size_computer=128;
	if (GetUserNameW(user_name, &buffer_size_user) && GetComputerNameW(computer_name, &buffer_size_computer))
	{
		wchar_t winamp_name_utf16[260] = {0};
		StringCbPrintfW(winamp_name_utf16, sizeof(winamp_name_utf16), L"%s (%s)", user_name, computer_name);
		WideCharToMultiByteSZ(CP_UTF8, 0, winamp_name_utf16, -1, winamp_name, sizeof(winamp_name), 0, 0);
	}
	else
		StringCbCopyA(winamp_name, sizeof(winamp_name), "Winamp");

	const wchar_t *settings_path = WASABI_API_APP->path_getUserSettingsPath();
	PathCombineW(inifile, settings_path, L"Plugins\\ml\\pmp_wifi.ini");

	// need to have this initialized before we try to do anything with localization features
	WASABI_API_START_LANG(plugin.hDllInstance,PmpWifiLangGUID);

	static wchar_t szDescription[256];
	StringCbPrintfW(szDescription, sizeof(szDescription),
					WASABI_API_LNGSTRINGW(IDS_NULLSOFT_WIFI_DEVICE_PLUGIN), PLUGIN_VERSION);
	plugin.description = szDescription;

	if (AGAVE_API_DEVICEMANAGER)
	{
		ifc_devicecommand *command = &attach_command;
		AGAVE_API_DEVICEMANAGER->CommandRegister(&command, 1);

		ifc_deviceconnection *connection = &wifi_connection;
		AGAVE_API_DEVICEMANAGER->ConnectionRegister(&connection, 1);


		AGAVE_API_DEVICEMANAGER->CreateSupportedCommandStore(&command_store);
		command_store->Add("attach", DeviceCommandFlag_Primary);

		AGAVE_API_DEVICEMANAGER->CreateDeviceEventManager(&device_event_manager);
	}
	//AGAVE_API_DEVICEMANAGER->CreateSupportedCommandEnum(&command, 1, &command_enum);
	/* TODO: Use this if your device shows up as a normal drive
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)autoDetectCallback,PMP_IPC_ENUM_ACTIVE_DRIVES);
	*/
	StartListenServer();
	return 0;
}

static void Quit() 
{
	StopListenServer();
	WasabiQuit();
}



static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) 
{
	switch(msg) {
		case PMP_DEVICECHANGE:
			// TODO: Implement
			return 0;
		case PMP_NO_CONFIG:
			return TRUE;
		case PMP_CONFIG:
			// TODO: Implement (Egg: changed from 1 to 0, for now)
			return 0;
	}
	return 0;
}

extern "C" 	__declspec(dllexport) PMPDevicePlugin *winampGetPMPDevicePlugin()
{
	return &plugin;
}