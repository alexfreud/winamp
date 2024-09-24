#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class DeviceActivity;

class Device: public ifc_device
{

protected:
	Device();
	~Device();
public:
	static HRESULT CreateInstance(const char *name, 
								  const char *type, 
								  const char *connection, 
								  Device**instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_device */
	const char *GetName(); 
	const char *GetType();
	const char *GetConnection();
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);
	BOOL GetHidden();
	HRESULT GetTotalSpace(uint64_t *size);
	HRESULT GetUsedSpace(uint64_t *size);
	BOOL GetAttached();
	HRESULT Attach(HWND hostWindow);
	HRESULT Detach(HWND hostWindow);
	HRESULT EnumerateCommands(ifc_devicesupportedcommandenum **enumerator, DeviceCommandContext context);
	HRESULT SendCommand(const char *command, HWND hostWindow, ULONG_PTR param);
	HRESULT GetCommandFlags(const char *command, DeviceCommandFlags *flags);
	HRESULT GetActivity(ifc_deviceactivity **activity);
	HRESULT Advise(ifc_deviceevent *handler);
	HRESULT Unadvise(ifc_deviceevent *handler);

	HWND CreateView(HWND parentWindow);
	void SetNavigationItem(void *navigationItem);

	HRESULT GetModel(wchar_t *buffer, size_t bufferSize); 
	HRESULT GetStatus(wchar_t *buffer, size_t bufferSize);

public:
	HRESULT SetConnection(const char *connection);
	HRESULT SetDisplayName(const wchar_t *name);
	HRESULT SetTotalSpace(uint64_t size);
	HRESULT SetUsedSpace(uint64_t size);
	HRESULT SetHidden(BOOL hiddenState);
	HRESULT SetModel(const wchar_t *deviceModel); 
	HRESULT SetStatus(const wchar_t *deviceStatus); 
	
	HRESULT AddIcon(const wchar_t *path, unsigned int width, unsigned int height);
	HRESULT EnumerateIcons(ifc_deviceiconstore::EnumeratorCallback callback, void *user);
	HRESULT RemoveIcon(unsigned int width, unsigned int height);

	HRESULT AddCommand(const char *command, DeviceCommandFlags flags);
	HRESULT RemoveCommand(const char *command);
	HRESULT SetCommandFlags(const char *command, DeviceCommandFlags mask, DeviceCommandFlags flags);

	HRESULT IsConnected();
	HRESULT Connect();
	HRESULT Disconnect();

	HRESULT CopyTo(Device *target);
	HRESULT SetIconBase(const wchar_t *path);

	HRESULT StartSyncActivity(HWND hostWindow);


protected:
	void Lock();
	void Unlock();

	static void ActivityStartedCb(DeviceActivity *activity);
	static void ActivityFinishedCb(DeviceActivity *activity);
	static void ActivityProgressCb(DeviceActivity *activity, unsigned int progress, unsigned int duration);


protected:
	size_t ref;
	char *name;
	char *type;
	char *connection;
	wchar_t *displayName;
	wchar_t *model;
	wchar_t *status;
	uint64_t totalSpace;
	uint64_t usedSpace;
	BOOL attached;
	BOOL hidden;
	BOOL connected;
	ifc_deviceiconstore *iconStore;
	ifc_deviceeventmanager *eventManager;
	ifc_devicesupportedcommandstore *commands;
	DeviceActivity *activity;
	CRITICAL_SECTION lock;
protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_DEVICE_HEADER