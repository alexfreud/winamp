#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {52C5C581-65DC-435e-AEFA-1616CB81B283}
static const GUID IFC_DeviceCommand = 
{ 0x52c5c581, 0x65dc, 0x435e, { 0xae, 0xfa, 0x16, 0x16, 0xcb, 0x81, 0xb2, 0x83 } };


#include "ifc_deviceobject.h"

typedef enum DeviceCommandFlags
{
	DeviceCommandFlag_None = 0,
	DeviceCommandFlag_Primary = (1 << 0),
	DeviceCommandFlag_Disabled = (1 << 1),
	DeviceCommandFlag_Active = (1 << 2),
	DeviceCommandFlag_Hidden = (1 << 3),
	DeviceCommandFlag_Group	= (1 << 4),
}DeviceCommandFlags;
DEFINE_ENUM_FLAG_OPERATORS(DeviceCommandFlags);

typedef enum DeviceCommandContext
{
	DeviceCommandContext_Unknown = 0,
	DeviceCommandContext_NavigationMenu = 1,
	DeviceCommandContext_ViewMenu = 2,
	DeviceCommandContext_View = 3,

} DeviceCommandContext;

class __declspec(novtable) ifc_devicecommand : public ifc_deviceobject
{
protected:
	ifc_devicecommand() {}
	~ifc_devicecommand() {}

public:
	HRESULT GetDescription(wchar_t *buffer, size_t bufferSize);

public:
	DISPATCH_CODES
	{
		API_GETDESCRIPTION		= 10,
	};
};

inline HRESULT ifc_devicecommand::GetDescription(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDESCRIPTION, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_INTERFACE_HEADER