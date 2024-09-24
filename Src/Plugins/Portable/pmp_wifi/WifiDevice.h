#pragma once
#include <bfc/platform/types.h>
#include "../devices/ifc_device.h"
#include "device.h"
#include "../nu/refcount.h"
#include "ConnectActivity.h"
#include "main.h"
/* this one inherits from ifc_device (not Device from ml_pmp) and is used to manage
  attaching/detaching, etc from the device view */
class TemplateDevice;
class WifiDevice : public Countable<ifc_device>
{
public:
	WifiDevice(const char *root_url, const DeviceInfo *device_info);
	~WifiDevice();
		/* --- ifc_device interface --- */
	int QueryInterface(GUID interface_guid, void **object);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);
	const char *GetName();
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	const char *GetType();
	const char *GetConnection();

	BOOL GetHidden();
	
	HRESULT GetTotalSpace(uint64_t *size);
	HRESULT GetUsedSpace(uint64_t *size);

	BOOL GetAttached();
	HRESULT Attach(HWND hostWindow);
	HRESULT Detach(HWND hostWindow);
	
	HRESULT EnumerateCommands(ifc_devicesupportedcommandenum **enumerator, DeviceCommandContext context);
	HRESULT SendCommand(const char *command, HWND hostWindow, ULONG_PTR param);
	HRESULT GetActiveCommand(char *buffer, size_t bufferSize);
	HRESULT CancelCommand(const char *command, HWND hostWindow);
	HRESULT GetCommandFlags(const char *command, DeviceCommandFlags *flags);

	HRESULT Advise(ifc_deviceevent *handler);
	HRESULT Unadvise(ifc_deviceevent *handler);

	HWND CreateView(HWND parentWindow);
	void SetNavigationItem(void *navigationItem); 

	void OnPaired();
	void OnConnected(TemplateDevice *device);
	void OnConnectionFailed();
	void OnDisconnect();
	HRESULT GetModel(wchar_t *buffer, size_t bufferSize);

	HRESULT GetActivity(ifc_deviceactivity **activity);

	REFERENCE_COUNT_IMPLEMENTATION;
private:	
	DeviceInfo device_info;
	RECVS_DISPATCH;
	char id_string[32];
	char *url;
	TemplateDevice *pmp_device;
	ConnectActivity connect_activity;
	bool connect_active;
	volatile int dead;
	CRITICAL_SECTION register_lock;
};