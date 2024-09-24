#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_PROVIDER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_PROVIDER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {FAC6EBEE-107A-4133-A620-856A281EC704}
static const GUID IFC_DeviceProvider = 
{ 0xfac6ebee, 0x107a, 0x4133, { 0xa6, 0x20, 0x85, 0x6a, 0x28, 0x1e, 0xc7, 0x4 } };

#include <bfc/dispatch.h>

class api_devicemanager;

class __declspec(novtable) ifc_deviceprovider : public Dispatchable
{
protected:
	ifc_deviceprovider() {}
	~ifc_deviceprovider() {}

public:
	HRESULT BeginDiscovery(api_devicemanager *manager);
	HRESULT CancelDiscovery();
	HRESULT GetActive();

public:
	DISPATCH_CODES
	{
		API_BEGINDISCOVERY		= 10,
		API_CANCELDISCOVERY		= 20,
		API_GETACTIVE			= 30,
	};
};


inline HRESULT ifc_deviceprovider::BeginDiscovery(api_devicemanager *manager)
{
	return _call(API_BEGINDISCOVERY, (HRESULT)E_NOTIMPL, manager);
}

inline HRESULT ifc_deviceprovider::CancelDiscovery()
{
	return _call(API_CANCELDISCOVERY, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_deviceprovider::GetActive()
{
	return _call(API_GETACTIVE, (HRESULT)E_NOTIMPL);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_PROVIDER_INTERFACE_HEADER