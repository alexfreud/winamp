#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_INFO_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_INFO_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


// {EFFB983B-D087-4021-8894-BA795626048B}
static const GUID IFC_DeviceCommandInfo = 
{ 0xeffb983b, 0xd087, 0x4021, { 0x88, 0x94, 0xba, 0x79, 0x56, 0x26, 0x4, 0x8b } };

#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_devicecommandinfo : public Dispatchable
{
protected:
	ifc_devicecommandinfo() {}
	~ifc_devicecommandinfo() {}

public:
	const wchar_t *GetName(); 
	HRESULT GetState(DeviceCommandState *state);

public:
	DISPATCH_CODES
	{
		API_GETNAME				= 10,
		API_GETSTATE			= 20,
	};
};


inline const wchar_t *ifc_devicetype::GetName()
{
	return _call(API_GETNAME, (const wchar_t *)NULL);
}

inline HRESULT ifc_devicetype::GetIcon(wchar_t *buffer, size_t bufferSize, DeviceIconType preferredType)
{
	return _call(API_GETICON, (HRESULT)E_NOTIMPL, buffer, bufferSize, preferredType);
}

inline HRESULT ifc_devicetype::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDISPLAYNAME, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_INFO_INTERFACE_HEADER