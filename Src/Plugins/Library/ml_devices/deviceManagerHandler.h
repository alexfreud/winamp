#ifndef _NULLSOFT_WINAMP_ML_DEVICES_DEVICE_MANAGER_HANDLER_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_DEVICE_MANAGER_HANDLER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../devices/api_devicemanager.h"
//#include <ifc_devicemanagerevent.h>

class DeviceManagerHandler: public ifc_devicemanagerevent
{
protected:
	DeviceManagerHandler();
	~DeviceManagerHandler();

public:
	static HRESULT CreateInstance(DeviceManagerHandler **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_devicemanagerevent */
	void TypeAdded(api_devicemanager *manager, ifc_devicetype *type);
	void TypeRemoved(api_devicemanager *manager, ifc_devicetype *type);
	void ConnectionAdded(api_devicemanager *manager, ifc_deviceconnection *connection);
	void ConnectionRemoved(api_devicemanager *manager, ifc_deviceconnection *connection);
	void CommandAdded(api_devicemanager *manager, ifc_devicecommand *command);
	void CommandRemoved(api_devicemanager *manager, ifc_devicecommand *command);
	void DeviceAdded(api_devicemanager *manager, ifc_device *device);
	void DeviceRemoved(api_devicemanager *manager, ifc_device *device);
	void DiscoveryStarted(api_devicemanager *manager);
	void DiscoveryFinished(api_devicemanager *manager);

public:
	HRESULT SetRelayWindow(HWND hwnd);
	HRESULT Advise(api_devicemanager *manager);
	HRESULT Unadvise(api_devicemanager *manager);


protected:
	size_t ref;
	HWND relayWindow;

protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_ML_DEVICES_DEVICE_MANAGER_HANDLER_HEADER