#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_MANAGER_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_MANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./api_devicemanager.h"
#include "./deviceObjectStore.h"
#include "./discoveryMonitor.h"
#include <vector>

class DeviceManager : public api_devicemanager
{

protected:
	DeviceManager();
	~DeviceManager();

public:
	static HRESULT CreateInstance(DeviceManager **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* api_devicemanager */
	size_t TypeRegister(ifc_devicetype **types, size_t count);
	size_t TypeRegisterIndirect(const char **names, size_t count, DeviceTypeCreator callback, void *user);
	HRESULT TypeUnregister(const char *name);
	HRESULT TypeFind(const char *name, ifc_devicetype **type);
	HRESULT TypeEnumerate(ifc_deviceobjectenum **enumerator);

	size_t ConnectionRegister(ifc_deviceconnection **connections, size_t count);
	size_t ConnectionRegisterIndirect(const char **names, size_t count, DeviceConnectionCreator callback, void *user);
	HRESULT ConnectionUnregister(const char *name);
	HRESULT ConnectionFind(const char *name, ifc_deviceconnection **connection);
	HRESULT ConnectionEnumerate(ifc_deviceobjectenum **enumerator);

	size_t CommandRegister(ifc_devicecommand **commands, size_t count);
	size_t CommandRegisterIndirect(const char **names, size_t count, DeviceCommandCreator callback, void *user);
	HRESULT CommandUnregister(const char *name);
	HRESULT CommandFind(const char *name, ifc_devicecommand **command);
	HRESULT CommandEnumerate(ifc_deviceobjectenum **enumerator);

	size_t DeviceRegister(ifc_device **devices, size_t count);
	HRESULT DeviceUnregister(const char *name);
	HRESULT DeviceFind(const char *name, ifc_device **device);
	HRESULT DeviceEnumerate(ifc_deviceobjectenum **enumerator);

	HRESULT IsDiscoveryActive();
	HRESULT BeginDiscovery();
	HRESULT CancelDiscovery();
	HRESULT RegisterProvider(ifc_deviceprovider *provider);
	HRESULT UnregisterProvider(ifc_deviceprovider *provider);
	HRESULT SetProviderActive(ifc_deviceprovider *provider, BOOL activeState);

	HRESULT Advise(ifc_devicemanagerevent *handler);
	HRESULT Unadvise(ifc_devicemanagerevent *handler);

	HRESULT CreateDeviceEventManager(ifc_deviceeventmanager **eventManager);
	HRESULT CreateSupportedCommandStore(ifc_devicesupportedcommandstore **store);
	HRESULT CreateSupportedCommandEnum(ifc_devicesupportedcommand **commands, size_t count, ifc_devicesupportedcommandenum **enumerator);
	HRESULT CreateIconStore(ifc_deviceiconstore **store);
	HRESULT CreateType(const char *name, ifc_devicetype **type);
	HRESULT CreateCommand(const char *name, ifc_devicecommand **command);
	HRESULT CreateConnection(const char *name, ifc_deviceconnection **connection);

protected:
	void EventTypeAdded(ifc_devicetype *type);
	void EventTypeRemoved(ifc_devicetype *type);
	void EventConnectionAdded(ifc_deviceconnection *connection);
	void EventConnectionRemoved(ifc_deviceconnection *connection);
	void EventCommandAdded(ifc_devicecommand *command);
	void EventCommandRemoved(ifc_devicecommand *command);
	void EventDeviceAdded(ifc_device *device);
	void EventDeviceRemoved(ifc_device *device);
	void EventDiscoveryStarted();
	void EventDiscoveryFinished();

protected:
	static void ObjectAddedCallback(DeviceObjectStore *store, ifc_deviceobject *object, void *userData);
	static void ObjectRemovedCallback(DeviceObjectStore *store, ifc_deviceobject *object, void *userData);

protected:
	typedef std::vector<ifc_devicemanagerevent*> EventList;
	typedef std::vector<ifc_deviceprovider*> ProviderList;

protected:
	size_t ref;
	
	DeviceObjectStore typeStore;
	DeviceObjectStore connectionStore;
	DeviceObjectStore commandStore;
	DeviceObjectStore deviceStore;

	CRITICAL_SECTION eventLock;
	EventList eventList;

	CRITICAL_SECTION providerLock;
	ProviderList providerList;

	DiscoveryMonitor discoveryMonitor;

protected:
	RECVS_DISPATCH;
};

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_MANAGER_HEADER
