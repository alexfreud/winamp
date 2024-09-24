#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_CONNECTION_EDITOR_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_CONNECTION_EDITOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {B4CAEAF3-4488-4313-8A66-DEA916DEFCCC}
static const GUID IFC_DeviceConnectionEditor = 
{ 0xb4caeaf3, 0x4488, 0x4313, { 0x8a, 0x66, 0xde, 0xa9, 0x16, 0xde, 0xfc, 0xcc } };



#include <bfc/dispatch.h>

class ifc_deviceiconstore;

class __declspec(novtable) ifc_deviceconnectioneditor : public Dispatchable
{
protected:
	ifc_deviceconnectioneditor() {}
	~ifc_deviceconnectioneditor() {}

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

inline HRESULT ifc_deviceconnectioneditor::GetIconStore(ifc_deviceiconstore **iconStore)
{
	return _call(API_GETICONSTORE, (HRESULT)E_NOTIMPL, iconStore);
}

inline HRESULT ifc_deviceconnectioneditor::SetDisplayName(const wchar_t *displayName)
{
	return _call(API_SETDISPLAYNAME, (HRESULT)E_NOTIMPL, displayName);
}


#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_CONNECTION_EDITOR_INTERFACE_HEADER