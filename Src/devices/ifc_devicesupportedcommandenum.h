#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_ENUMERATOR_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_ENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {BE64390B-D4D0-41d5-87A7-60597F410D97}
static const GUID IFC_DeviceSupportedCommandEnum = 
{ 0xbe64390b, 0xd4d0, 0x41d5, { 0x87, 0xa7, 0x60, 0x59, 0x7f, 0x41, 0xd, 0x97 } };


#include <bfc/dispatch.h>

class ifc_devicesupportedcommand;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_devicesupportedcommandenum : public Dispatchable
{
protected:
	ifc_devicesupportedcommandenum() {}
	~ifc_devicesupportedcommandenum() {}

public:
	HRESULT Next(ifc_devicesupportedcommand **buffer, size_t bufferMax, size_t *count);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

public:
	DISPATCH_CODES
	{
		API_NEXT = 10,
		API_RESET = 20,
		API_SKIP = 30,
		API_GETCOUNT = 40,
	};
};

inline HRESULT ifc_devicesupportedcommandenum::Next(ifc_devicesupportedcommand **buffer, size_t bufferMax, size_t *count)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, buffer, bufferMax, count);
}

inline HRESULT ifc_devicesupportedcommandenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_devicesupportedcommandenum::Skip(size_t count)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, count);
}

inline HRESULT ifc_devicesupportedcommandenum::GetCount(size_t *count)
{
	return _call(API_GETCOUNT, (HRESULT)E_NOTIMPL, count);
}


#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_ENUMERATOR_INTERFACE_HEADER