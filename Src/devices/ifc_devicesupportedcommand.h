#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {EFFB983B-D087-4021-8894-BA795626048B}
static const GUID  IFC_DeviceSupportedCommand = 
{ 0xeffb983b, 0xd087, 0x4021, { 0x88, 0x94, 0xba, 0x79, 0x56, 0x26, 0x4, 0x8b } };


#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_devicesupportedcommand : public Dispatchable
{
protected:
	ifc_devicesupportedcommand() {}
	~ifc_devicesupportedcommand() {}

public:
	const char *GetName(); 
	HRESULT GetFlags(DeviceCommandFlags *flags);

public:
	DISPATCH_CODES
	{
		API_GETNAME	= 10,
		API_GETFLAGS = 20,
	};
};

inline const char *ifc_devicesupportedcommand::GetName()
{
	return _call(API_GETNAME, (const char *)NULL);
}

inline HRESULT ifc_devicesupportedcommand::GetFlags(DeviceCommandFlags *flags)
{
	return _call(API_GETFLAGS, (HRESULT)E_NOTIMPL, flags);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_INTERFACE_HEADER