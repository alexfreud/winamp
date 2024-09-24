#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_MANAGER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_MANAGER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {CB360A8F-0636-4a0d-9374-3BBCC18C8B40}
static const GUID DeviceManagerGUID = 
{ 0xcb360a8f, 0x636, 0x4a0d, { 0x93, 0x74, 0x3b, 0xbc, 0xc1, 0x8c, 0x8b, 0x40 } };


#include <bfc/dispatch.h>
#include "ifc_device.h"
#include "ifc_deviceobjectenum.h"
#include "ifc_devicetype.h"
#include "ifc_devicetypeeditor.h"
#include "ifc_devicecommand.h"
#include "ifc_devicecommandeditor.h"
#include "ifc_deviceconnection.h"
#include "ifc_deviceconnectioneditor.h"
#include "ifc_devicemanagerevent.h"
#include "ifc_deviceprovider.h"
#include "ifc_deviceeventmanager.h"
#include "ifc_devicesupportedcommand.h"
#include "ifc_devicesupportedcommandstore.h"
#include "ifc_devicesupportedcommandenum.h"
#include "ifc_deviceiconstore.h"
#include "ifc_deviceactivity.h"

typedef ifc_devicetype *(_cdecl *DeviceTypeCreator)(const char * /*name*/, void * /*user*/);
typedef ifc_devicecommand *(_cdecl *DeviceCommandCreator)(const char * /*name*/, void * /*user*/);
typedef ifc_deviceconnection *(_cdecl *DeviceConnectionCreator)(const char * /*name*/, void * /*user*/);


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) api_devicemanager : public Dispatchable
{
protected:
	api_devicemanager() {}
	~api_devicemanager() {}

public:
	
	/* types */
	size_t TypeRegister(ifc_devicetype **types, size_t count);  // return number of registered types
	size_t TypeRegisterIndirect(const char **names, size_t count, DeviceTypeCreator callback, void *user);
	HRESULT TypeUnregister(const char *name);
	HRESULT TypeFind(const char *name, ifc_devicetype **type);
	HRESULT TypeEnumerate(ifc_deviceobjectenum **enumerator);

	/* connections */
	size_t ConnectionRegister(ifc_deviceconnection **connections, size_t count);
	size_t ConnectionRegisterIndirect(const char **names, size_t count, DeviceConnectionCreator callback, void *user);
	HRESULT ConnectionUnregister(const char *name);
	HRESULT ConnectionFind(const char *name, ifc_deviceconnection **connection);
	HRESULT ConnectionEnumerate(ifc_deviceobjectenum **enumerator);

	/* commmands */
	size_t CommandRegister(ifc_devicecommand **commands, size_t count);
	size_t CommandRegisterIndirect(const char **names, size_t count, DeviceCommandCreator callback, void *user);
	HRESULT CommandUnregister(const char *name);
	HRESULT CommandFind(const char *name, ifc_devicecommand **command);
	HRESULT CommandEnumerate(ifc_deviceobjectenum **enumerator);

	/* devices */
	size_t DeviceRegister(ifc_device **devices, size_t count);
	HRESULT DeviceUnregister(const char *name);
	HRESULT DeviceFind(const char *name, ifc_device **device);
	HRESULT DeviceEnumerate(ifc_deviceobjectenum **enumerator);

	/* discovery */
	HRESULT IsDiscoveryActive();
	HRESULT BeginDiscovery();
	HRESULT CancelDiscovery();
	HRESULT RegisterProvider(ifc_deviceprovider *provider);
	HRESULT UnregisterProvider(ifc_deviceprovider *provider);
	HRESULT SetProviderActive(ifc_deviceprovider *provider, BOOL activeState);

	/* events */
	HRESULT Advise(ifc_devicemanagerevent *handler);
	HRESULT Unadvise(ifc_devicemanagerevent *handler);

	/* misc helpers */
	HRESULT CreateDeviceEventManager(ifc_deviceeventmanager **eventManager);
	HRESULT CreateSupportedCommandStore(ifc_devicesupportedcommandstore **store);
	HRESULT CreateSupportedCommandEnum(ifc_devicesupportedcommand **commands, size_t count, ifc_devicesupportedcommandenum **enumerator);
	HRESULT CreateIconStore(ifc_deviceiconstore **store);
	HRESULT CreateType(const char *name, ifc_devicetype **type);
	HRESULT CreateCommand(const char *name, ifc_devicecommand **command);
	HRESULT CreateConnection(const char *name, ifc_deviceconnection **connection);

public:
	DISPATCH_CODES
	{
		API_TYPEREGISTER = 10,
		API_TYPEREGISTERINDIRECT = 20,
		API_TYPEUNREGISTER = 30,
		API_TYPEFIND = 40,
		API_TYPEENUMERATE = 50,
		API_CONNECTIONREGISTER = 60,
		API_CONNECTIONREGISTERINDIRECT = 70,
		API_CONNECTIONUNREGISTER = 80,
		API_CONNECTIONFIND = 90,
		API_CONNECTIONENUMERATE	= 100,
		API_COMMANDREGISTER	= 110,
		API_COMMANDREGISTERINDIRECT = 120,
		API_COMMANDUNREGISTER = 130,
		API_COMMANDFIND = 140,
		API_COMMANDENUMERATE = 150,
		API_DEVICEREGISTER = 160,
		API_DEVICEUNREGISTER = 170,
		API_DEVICEFIND = 180,
		API_DEVICEENUMERATE = 190,
		API_ISDISCOVERYACTIVE = 200,
		API_BEGINDISCOVERY = 210,
		API_CANCELDISCOVERY = 220,
		API_REGISTERPROVIDER = 230,
		API_UNREGISTERPROVIDER = 240,
		API_SETPROVIDERACTIVE = 250,
		API_ADVISE = 260,
		API_UNADVISE = 270,
		API_CREATEDEVICEEVENTMANAGER = 400,
		API_CREATESUPPORTEDCOMMANDSTORE = 410,
		API_CREATESUPPORTEDCOMMANDENUM = 420,
		API_CREATEICONSTORE = 430,
		API_CREATETYPE = 440,
		API_CREATECOMMAND = 450,
		API_CREATECONNECTION = 460,
	};
};

inline size_t api_devicemanager::TypeRegister(ifc_devicetype **types, size_t count)
{
	return _call(API_TYPEREGISTER, (size_t)0, types, count);
}

inline size_t api_devicemanager::TypeRegisterIndirect(const char **names, size_t count, DeviceTypeCreator callback, void *user)
{
	return _call(API_TYPEREGISTERINDIRECT, (size_t)0, names, count, callback, user);
}

inline HRESULT api_devicemanager::TypeUnregister(const char *name)
{
	return _call(API_TYPEUNREGISTER, (HRESULT)E_NOTIMPL, name);
}

inline HRESULT api_devicemanager::TypeFind(const char *name, ifc_devicetype **type)
{
	return _call(API_TYPEFIND, (HRESULT)E_NOTIMPL, name, type);
}

inline HRESULT api_devicemanager::TypeEnumerate(ifc_deviceobjectenum **enumerator)
{
	return _call(API_TYPEENUMERATE, (HRESULT)E_NOTIMPL, enumerator);
}

inline size_t api_devicemanager::ConnectionRegister(ifc_deviceconnection **connections, size_t count)
{
	return _call(API_CONNECTIONREGISTER, (size_t)0, connections, count);
}

inline size_t api_devicemanager::ConnectionRegisterIndirect(const char **names, size_t count, DeviceConnectionCreator callback, void *user)
{
	return _call(API_CONNECTIONREGISTERINDIRECT, (size_t)0, names, count, callback, user);
}

inline HRESULT api_devicemanager::ConnectionUnregister(const char *name)
{
	return _call(API_CONNECTIONUNREGISTER, (HRESULT)E_NOTIMPL, name);
}

inline HRESULT api_devicemanager::ConnectionFind(const char *name, ifc_deviceconnection **connection)
{
	return _call(API_CONNECTIONFIND, (HRESULT)E_NOTIMPL, name, connection);
}

inline HRESULT api_devicemanager::ConnectionEnumerate(ifc_deviceobjectenum **enumerator)
{
	return _call(API_CONNECTIONENUMERATE, (HRESULT)E_NOTIMPL, enumerator);
}

inline size_t api_devicemanager::CommandRegister(ifc_devicecommand **commands, size_t count)
{
	return _call(API_COMMANDREGISTER, (size_t)0, commands, count);
}

inline size_t api_devicemanager::CommandRegisterIndirect(const char **names, size_t count, DeviceCommandCreator callback, void *user)
{
	return _call(API_COMMANDREGISTERINDIRECT, (size_t)0, names, count, callback, user);
}

inline HRESULT api_devicemanager::CommandUnregister(const char *name)
{
	return _call(API_COMMANDUNREGISTER, (HRESULT)E_NOTIMPL, name);
}

inline HRESULT api_devicemanager::CommandFind(const char *name, ifc_devicecommand **command)
{
	return _call(API_COMMANDFIND, (HRESULT)E_NOTIMPL, name, command);
}

inline HRESULT api_devicemanager::CommandEnumerate(ifc_deviceobjectenum **enumerator)
{
	return _call(API_COMMANDENUMERATE, (HRESULT)E_NOTIMPL, enumerator);
}

inline size_t api_devicemanager::DeviceRegister(ifc_device **devices, size_t count)
{
	return _call(API_DEVICEREGISTER, (size_t)0, devices, count);
}

inline HRESULT api_devicemanager::DeviceUnregister(const char *name)
{
	return _call(API_DEVICEUNREGISTER, (HRESULT)E_NOTIMPL, name);
}

inline HRESULT api_devicemanager::DeviceFind(const char *name, ifc_device **device)
{
	return _call(API_DEVICEFIND, (HRESULT)E_NOTIMPL, name, device);
}

inline HRESULT api_devicemanager::DeviceEnumerate(ifc_deviceobjectenum **enumerator)
{
	return _call(API_DEVICEENUMERATE, (HRESULT)E_NOTIMPL, enumerator);
}

inline HRESULT api_devicemanager::IsDiscoveryActive()
{
	return _call(API_ISDISCOVERYACTIVE, (HRESULT)E_NOTIMPL);
}

inline HRESULT api_devicemanager::BeginDiscovery()
{
	return _call(API_BEGINDISCOVERY, (HRESULT)E_NOTIMPL);
}

inline HRESULT api_devicemanager::CancelDiscovery()
{
	return _call(API_CANCELDISCOVERY, (HRESULT)E_NOTIMPL);
}

inline HRESULT api_devicemanager::RegisterProvider(ifc_deviceprovider *provider)
{
	return _call(API_REGISTERPROVIDER, (HRESULT)E_NOTIMPL, provider);
}

inline HRESULT api_devicemanager::UnregisterProvider(ifc_deviceprovider *provider)
{
	return _call(API_UNREGISTERPROVIDER, (HRESULT)E_NOTIMPL, provider);
}

inline HRESULT api_devicemanager::SetProviderActive(ifc_deviceprovider *provider, BOOL activeState)
{
	return _call(API_SETPROVIDERACTIVE, (HRESULT)E_NOTIMPL, provider, activeState);
}

inline HRESULT api_devicemanager::Advise(ifc_devicemanagerevent *handler)
{
	return _call(API_ADVISE, (HRESULT)E_NOTIMPL, handler);
}

inline HRESULT api_devicemanager::Unadvise(ifc_devicemanagerevent *handler)
{
	return _call(API_UNADVISE, (HRESULT)E_NOTIMPL, handler);
}

inline HRESULT api_devicemanager::CreateDeviceEventManager(ifc_deviceeventmanager **eventManager)
{
	return _call(API_CREATEDEVICEEVENTMANAGER, (HRESULT)E_NOTIMPL, eventManager);
}

inline HRESULT api_devicemanager::CreateSupportedCommandStore(ifc_devicesupportedcommandstore **store)
{
	return _call(API_CREATESUPPORTEDCOMMANDSTORE, (HRESULT)E_NOTIMPL, store);
}

inline HRESULT api_devicemanager::CreateSupportedCommandEnum(ifc_devicesupportedcommand **commands, size_t count, ifc_devicesupportedcommandenum **enumerator)
{
	return _call(API_CREATESUPPORTEDCOMMANDENUM, (HRESULT)E_NOTIMPL, commands, count, enumerator);
}

inline HRESULT api_devicemanager::CreateIconStore(ifc_deviceiconstore **store)
{
	return _call(API_CREATEICONSTORE, (HRESULT)E_NOTIMPL, store);
}

inline HRESULT api_devicemanager::CreateType(const char *name, ifc_devicetype **type)
{
	return _call(API_CREATETYPE, (HRESULT)E_NOTIMPL, name, type);
}

inline HRESULT api_devicemanager::CreateCommand(const char *name, ifc_devicecommand **command)
{
	return _call(API_CREATECOMMAND, (HRESULT)E_NOTIMPL, name, command);
}

inline HRESULT api_devicemanager::CreateConnection(const char *name, ifc_deviceconnection **connection)
{
	return _call(API_CREATECONNECTION, (HRESULT)E_NOTIMPL, name, connection);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_MANAGER_INTERFACE_HEADER