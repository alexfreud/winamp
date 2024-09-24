#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_TYPE_EDITOR_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_TYPE_EDITOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {D6CE88AA-94F1-4c7f-BEDB-A45B637CF804}
static const GUID IFC_DeviceTypeEditor = 
{ 0xd6ce88aa, 0x94f1, 0x4c7f, { 0xbe, 0xdb, 0xa4, 0x5b, 0x63, 0x7c, 0xf8, 0x4 } };

#include <bfc/dispatch.h>

class ifc_deviceiconstore;

class __declspec(novtable) ifc_devicetypeeditor : public Dispatchable
{
protected:
	ifc_devicetypeeditor() {}
	~ifc_devicetypeeditor() {}

public:
	HRESULT GetIconStore(ifc_deviceiconstore **iconStore);
	HRESULT SetDisplayName(const wchar_t *displayName);

public:
	DISPATCH_CODES
	{
		API_GETICONSTORE		= 10,
		API_SETDISPLAYNAME		= 20,
	};
};

inline HRESULT ifc_devicetypeeditor::GetIconStore(ifc_deviceiconstore **iconStore)
{
	return _call(API_GETICONSTORE, (HRESULT)E_NOTIMPL, iconStore);
}

inline HRESULT ifc_devicetypeeditor::SetDisplayName(const wchar_t *displayName)
{
	return _call(API_SETDISPLAYNAME, (HRESULT)E_NOTIMPL, displayName);
}


#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_TYPE_EDITOR_INTERFACE_HEADER