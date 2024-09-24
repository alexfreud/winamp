#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {90A1273D-3E52-415f-ADC1-F151E6240C5B}
static const GUID IFC_DeviceObject = 
{ 0x90a1273d, 0x3e52, 0x415f, { 0xad, 0xc1, 0xf1, 0x51, 0xe6, 0x24, 0xc, 0x5b } };


#include <bfc/dispatch.h>


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_deviceobject : public Dispatchable
{
protected:
	ifc_deviceobject() {}
	~ifc_deviceobject() {}

public:
	const char *GetName(); 
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);

public:
	DISPATCH_CODES
	{
		API_GETNAME				= -10,
		API_GETICON				= -11,
		API_GETDISPLAYNAME		= -12,
	};
};


inline const char *ifc_deviceobject::GetName()
{
	return _call(API_GETNAME, (const char *)NULL);
}

inline HRESULT ifc_deviceobject::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	return _call(API_GETICON, (HRESULT)E_NOTIMPL, buffer, bufferSize, width, height);
}

inline HRESULT ifc_deviceobject::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDISPLAYNAME, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_INTERFACE_HEADER