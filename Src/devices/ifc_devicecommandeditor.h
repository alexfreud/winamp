#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_EDITOR_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_EDITOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {94A35125-3AD5-4339-870E-4ACB24F00FE8}
static const GUID IFC_DeviceCommandEditor = 
{ 0x94a35125, 0x3ad5, 0x4339, { 0x87, 0xe, 0x4a, 0xcb, 0x24, 0xf0, 0xf, 0xe8 } };


#include <bfc/dispatch.h>

class ifc_deviceiconstore;

class __declspec(novtable) ifc_devicecommandeditor : public Dispatchable
{
protected:
	ifc_devicecommandeditor() {}
	~ifc_devicecommandeditor() {}

public:
	HRESULT GetIconStore(ifc_deviceiconstore **iconStore);
	HRESULT SetDisplayName(const wchar_t *displayName);
	HRESULT SetDescription(const wchar_t *description);

public:
	DISPATCH_CODES
	{
		API_GETICONSTORE		= 10,
		API_SETDISPLAYNAME		= 20,
		API_SETDESCRIPTION		= 30,
	};
};

inline HRESULT ifc_devicecommandeditor::GetIconStore(ifc_deviceiconstore **iconStore)
{
	return _call(API_GETICONSTORE, (HRESULT)E_NOTIMPL, iconStore);
}

inline HRESULT ifc_devicecommandeditor::SetDisplayName(const wchar_t *displayName)
{
	return _call(API_SETDISPLAYNAME, (HRESULT)E_NOTIMPL, displayName);
}

inline HRESULT ifc_devicecommandeditor::SetDescription(const wchar_t *description)
{
	return _call(API_SETDESCRIPTION, (HRESULT)E_NOTIMPL, description);
}


#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_EDITOR_INTERFACE_HEADER