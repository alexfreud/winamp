#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_STORE_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_STORE_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {098C1639-7E02-4f03-9F75-B871EC867E61}
static const GUID IFC_DeviceSupportedCommandStore = 
{ 0x98c1639, 0x7e02, 0x4f03, { 0x9f, 0x75, 0xb8, 0x71, 0xec, 0x86, 0x7e, 0x61 } };


#include <bfc/dispatch.h>

class ifc_devicesupportedcommand;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_devicesupportedcommandstore : public Dispatchable
{
protected:
	ifc_devicesupportedcommandstore() {}
	~ifc_devicesupportedcommandstore() {}

public:
	HRESULT Add(const char *name, DeviceCommandFlags flags);
	HRESULT Remove(const char *name);
	HRESULT RemoveAll();

	HRESULT GetFlags(const char *name, DeviceCommandFlags *flagsOut);
	HRESULT SetFlags(const char *name, DeviceCommandFlags mask, DeviceCommandFlags value);
	
	HRESULT Get(const char *name, ifc_devicesupportedcommand **command);
	HRESULT GetActive(ifc_devicesupportedcommand **command);
	
	HRESULT Enumerate(ifc_devicesupportedcommandenum **enumerator);

	HRESULT Clone(ifc_devicesupportedcommandstore **instance, BOOL fullCopy);

public:
	DISPATCH_CODES
	{
		API_ADD = 10,
		API_REMOVE = 20,
		API_REMOVEALL = 30,
		API_GETFLAGS = 40,
		API_SETFLAGS = 50,
		API_GET = 60,
		API_GETACTIVE = 70,
		API_ENUMERATE = 80,
		API_CLONE = 90,
	};
};


inline HRESULT ifc_devicesupportedcommandstore::Add(const char *name, DeviceCommandFlags flags)
{
	return _call(API_ADD, (HRESULT)E_NOTIMPL, name, flags);
}

inline HRESULT ifc_devicesupportedcommandstore::Remove(const char *name)
{
	return _call(API_REMOVE, (HRESULT)E_NOTIMPL, name);
}

inline HRESULT ifc_devicesupportedcommandstore::RemoveAll()
{
	return _call(API_REMOVEALL, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_devicesupportedcommandstore::GetFlags(const char *name, DeviceCommandFlags *flagsOut)
{
	return _call(API_GETFLAGS, (HRESULT)E_NOTIMPL, name, flagsOut);
}

inline HRESULT ifc_devicesupportedcommandstore::SetFlags(const char *name, DeviceCommandFlags mask, DeviceCommandFlags value)
{
	return _call(API_SETFLAGS, (HRESULT)E_NOTIMPL, name, mask, value);
}
	
inline HRESULT ifc_devicesupportedcommandstore::Get(const char *name, ifc_devicesupportedcommand **command)
{
	return _call(API_GET, (HRESULT)E_NOTIMPL, name, command);
}

inline HRESULT ifc_devicesupportedcommandstore::GetActive(ifc_devicesupportedcommand **command)
{
	return _call(API_GETACTIVE, (HRESULT)E_NOTIMPL, command);
}
	
inline HRESULT ifc_devicesupportedcommandstore::Enumerate(ifc_devicesupportedcommandenum **enumerator)
{
	return _call(API_ENUMERATE, (HRESULT)E_NOTIMPL, enumerator);
}

inline HRESULT ifc_devicesupportedcommandstore::Clone(ifc_devicesupportedcommandstore **instance, BOOL fullCopy)
{
	return _call(API_CLONE, (HRESULT)E_NOTIMPL, instance, fullCopy);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_STORE_INTERFACE_HEADER