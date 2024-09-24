#include "WifiDevice.h"
#include "api.h"
#include "device.h"
#include "nu/ns_wc.h"
#include "resource.h"
#include "Pair.h"
#include "images.h"
#include "modelInfo.h"
#include "SongListDownloader.h"
#include <strsafe.h>

WifiDevice::WifiDevice(const char *root_url, const DeviceInfo *in_device_info)
: url(strdup(root_url))
{
	DeviceInfo_Copy(&device_info, in_device_info);
		InitializeCriticalSection(&register_lock);
	dead=0;
	connect_active=false;
	pmp_device=0;
	StringCbPrintfA(id_string, sizeof(id_string), "%016I64x", device_info.id);
	if (IsPaired(device_info.id))
	{
		char full_url[256] = {0};
		StringCbPrintfA(full_url, sizeof(full_url), "%s/library", url);
		WAC_API_DOWNLOADMANAGER->DownloadEx(full_url, new SongListDownloader(url, this), api_downloadManager::DOWNLOADEX_CALLBACK);
	}
	else
	{
		ifc_device *device = this;
		AGAVE_API_DEVICEMANAGER->DeviceRegister(&device, 1);
	}
}

WifiDevice::~WifiDevice()
{
	DeleteCriticalSection(&register_lock);
}

/* ifc_device stuff */
int WifiDevice::QueryInterface(GUID interface_guid, void **object)
{
	if (interface_guid == IFC_Device)
	{
		AddRef();
		*object = (ifc_device *)this;
		return 0;
	}
	return 1;
}

const char *WifiDevice::GetName()
{
	return id_string;
}

HRESULT WifiDevice::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	StringCchCopyW(buffer, bufferSize, device_info.name);
	return 0;
}

const char *WifiDevice::GetType()
{
	return "portable";
}

const char *WifiDevice::GetConnection()
{
	return "wifi";
}

extern ifc_devicesupportedcommandenum *command_enum;
extern ifc_devicesupportedcommandstore *command_store;
extern ifc_deviceeventmanager *device_event_manager;

HRESULT WifiDevice::EnumerateCommands(ifc_devicesupportedcommandenum **enumerator, DeviceCommandContext context)
{
	if (connect_active)
		return E_NOTIMPL;

	return command_store->Enumerate(enumerator);
}

HRESULT WifiDevice::SendCommand(const char *command, HWND hostWindow, ULONG_PTR param)
{
	if (!strcmp(command, "attach"))
	{
		return Attach(hostWindow);
	}

	return 0;
}

BOOL WifiDevice::GetAttached()
{
	return FALSE;
}

HRESULT WifiDevice::Attach(HWND hostWindow)
{
	if (!connect_active)
	{
		connect_active = true;
		device_event_manager->Notify_ActivityStarted(this, &connect_activity);

		char full_url[256] = {0};
		StringCbPrintfA(full_url, sizeof(full_url), "%s/pair", url);
		WAC_API_DOWNLOADMANAGER->DownloadEx(full_url, new PairDownloader(this), api_downloadManager::DOWNLOADEX_CALLBACK);
	}

	return S_OK;
}

HRESULT WifiDevice::Detach(HWND hostWindow)
{
	return S_OK;
}

HRESULT WifiDevice::Advise(ifc_deviceevent *handler)
{
	return device_event_manager->Advise(handler);
}

HRESULT WifiDevice::Unadvise(ifc_deviceevent *handler)
{
	return device_event_manager->Unadvise(handler);
}

HRESULT WifiDevice::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	return ModelInfo_GetIconPath(device_info.modelInfo, width, height, buffer, bufferSize, TRUE);
}

void WifiDevice::OnPaired()
{
	char full_url[256] = {0};
	StringCbPrintfA(full_url, sizeof(full_url), "%s/library", url);
	WAC_API_DOWNLOADMANAGER->DownloadEx(full_url, new SongListDownloader(url, this), api_downloadManager::DOWNLOADEX_CALLBACK);
	SetPaired(device_info.id, true);
}

void WifiDevice::OnConnected(TemplateDevice *device)
{
	EnterCriticalSection(&register_lock);
	pmp_device = device;
	connect_active = false;
	device_event_manager->Notify_ActivityFinished(this, &connect_activity);
	AGAVE_API_DEVICEMANAGER->DeviceUnregister(id_string);
	// if we disconnected/timed out on the listen server while connecting, go ahead and close the device out
	if (dead && pmp_device)
	{
		pmp_device->CloseAsync();
		pmp_device = 0;
	}
	LeaveCriticalSection(&register_lock);
}

void WifiDevice::OnDisconnect()
{
	// TODO: might actually need a crit sec here
	EnterCriticalSection(&register_lock);
	dead=1;
	if (pmp_device)
	{
		pmp_device->CloseAsync();
		pmp_device = 0;
	}
	else
	{
		AGAVE_API_DEVICEMANAGER->DeviceUnregister(id_string);
	}
	LeaveCriticalSection(&register_lock);
}

void WifiDevice::OnConnectionFailed()
{
	EnterCriticalSection(&register_lock);
	delete pmp_device;
	pmp_device = 0;
	ifc_device *device = NULL;
	bool device_exist = false;

	// see if we're already registered (e.g. we started in unpaired state)
	if (AGAVE_API_DEVICEMANAGER->DeviceFind(id_string, &device) == S_OK)
	{
		if (device == this)
			device_exist = true;

		device->Release();
	}

	if (device_exist)
	{ // if we are, then notify about activity being done
		connect_active = false;
		device_event_manager->Notify_ActivityFinished(this, &connect_activity);
	}
	else if (!dead)
	{ // if we weren't registered, we thought we were paired but failed
		device = this;
		AGAVE_API_DEVICEMANAGER->DeviceRegister(&device, 1);
	}


	LeaveCriticalSection(&register_lock);
}

HRESULT WifiDevice::GetActivity(ifc_deviceactivity **activity)
{
	if (connect_active)
	{
		*activity = &connect_activity;
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

HRESULT WifiDevice::GetTotalSpace(uint64_t *size)
{
#if 0
	if (device_info.total_space)
	{
		*size = device_info.total_space;
		return S_OK;
	}
#endif
	return E_NOTIMPL;
}

HRESULT WifiDevice::GetUsedSpace(uint64_t *size)
{
#if 0
	if (device_info.used_space)
	{
		*size = device_info.used_space;
		return S_OK;
	}
#endif
	return E_NOTIMPL;
}

HRESULT WifiDevice::GetModel(wchar_t *buffer, size_t bufferSize)
{
	return ModelInfo_CopyDisplayName(device_info.modelInfo, buffer, bufferSize);
}

#define CBCLASS WifiDevice
START_DISPATCH;
CB(QUERYINTERFACE, QueryInterface);
CB(API_GETNAME, GetName);
CB(API_GETICON, GetIcon);
CB(API_GETDISPLAYNAME, GetDisplayName);
CB(API_GETTOTALSPACE, GetTotalSpace);
CB(API_GETUSEDSPACE, GetUsedSpace);
CB(API_GETTYPE, GetType);
CB(API_GETCONNECTION, GetConnection);
CB(API_ENUMERATECOMMANDS, EnumerateCommands);
CB(API_SENDCOMMAND, SendCommand);
CB(API_GETATTACHED, GetAttached);
CB(API_ATTACH, Attach);
CB(API_DETACH, Detach);
CB(API_GETACTIVITY, GetActivity);
CB(API_ADVISE, Advise);
CB(API_UNADVISE, Unadvise);
CB(API_GETMODEL, GetModel);
REFERENCE_COUNTED;
END_DISPATCH;
