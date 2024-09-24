#ifndef _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_IMPLEMENTATION_HEADER
#define _NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_IMPLEMENTATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../../devices/ifc_deviceprovider.h"
#include "./testSuite.h"

class DeviceProvider : public ifc_deviceprovider
{
protected:
	DeviceProvider();
	~DeviceProvider();
public:
	static HRESULT CreateInstance(DeviceProvider **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_deviceprovider */
	HRESULT BeginDiscovery(api_devicemanager *manager);
	HRESULT CancelDiscovery();
	HRESULT GetActive();
public:
	HRESULT Register(api_devicemanager *manager);
	HRESULT Unregister(api_devicemanager *manager);

private:
	DWORD DiscoveryThread(api_devicemanager *manager);
	friend static DWORD CALLBACK DeviceProvider_DiscoveryThreadStarter(void *param);

protected:
	size_t ref;
	HANDLE discoveryThread;
	HANDLE cancelEvent;
	TestSuite testSuite;
	CRITICAL_SECTION lock;

protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_GEN_DEVICE_PROVIDER_IMPLEMENTATION_HEADER