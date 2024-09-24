#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_MANAGER_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_MANAGER_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {3D58B8B4-74C8-4fa2-B28D-86E96ABC7200}
static const GUID IFC_DeviceManagerEvent = 
{ 0x3d58b8b4, 0x74c8, 0x4fa2, { 0xb2, 0x8d, 0x86, 0xe9, 0x6a, 0xbc, 0x72, 0x0 } };


#include <bfc/dispatch.h>

class api_devicemanager;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_devicemanagerevent : public Dispatchable
{
protected:
	ifc_devicemanagerevent() {}
	~ifc_devicemanagerevent() {}

public:
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
	DISPATCH_CODES
	{
		API_TYPEADDED		= 10,
		API_TYPEREMOVED		= 20,
		API_CONNECTIONADDED = 30,
		API_CONNECTIONREMOVED = 40,
		API_COMMANDADDED	= 50,
		API_COMMANDREMOVED	= 60,
		API_DEVICEADDED		= 70,
		API_DEVICEREMOVED	= 80,
		API_DISCOVERYSTARTED = 90,
		API_DISCOVERYFINISHED = 100,
	};
};

inline void ifc_devicemanagerevent::TypeAdded(api_devicemanager *manager, ifc_devicetype *type)
{
	_voidcall(API_TYPEADDED, manager, type);
}

inline void ifc_devicemanagerevent::TypeRemoved(api_devicemanager *manager, ifc_devicetype *type)
{
	_voidcall(API_TYPEREMOVED, manager, type);
}

inline void ifc_devicemanagerevent::ConnectionAdded(api_devicemanager *manager, ifc_deviceconnection *connection)
{
	_voidcall(API_CONNECTIONADDED, manager, connection);
}

inline void ifc_devicemanagerevent::ConnectionRemoved(api_devicemanager *manager, ifc_deviceconnection *connection)
{
	_voidcall(API_CONNECTIONREMOVED, manager, connection);
}

inline void ifc_devicemanagerevent::CommandAdded(api_devicemanager *manager, ifc_devicecommand *command)
{
	_voidcall(API_COMMANDADDED, manager, command);
}

inline void ifc_devicemanagerevent::CommandRemoved(api_devicemanager *manager, ifc_devicecommand *command)
{
	_voidcall(API_COMMANDREMOVED, manager, command);
}

inline void ifc_devicemanagerevent::DeviceAdded(api_devicemanager *manager, ifc_device *device)
{
	_voidcall(API_DEVICEADDED, manager, device);
}

inline void ifc_devicemanagerevent::DeviceRemoved(api_devicemanager *manager, ifc_device *device)
{
	_voidcall(API_DEVICEREMOVED, manager, device);
}

inline void ifc_devicemanagerevent::DiscoveryStarted(api_devicemanager *manager)
{
	_voidcall(API_DISCOVERYSTARTED, manager);
}

inline void ifc_devicemanagerevent::DiscoveryFinished(api_devicemanager *manager)
{
	_voidcall(API_DISCOVERYFINISHED, manager);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_MANAGER_EVENT_INTERFACE_HEADER