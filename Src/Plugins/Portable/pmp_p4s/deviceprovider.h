#pragma once

#include <wtypes.h>
#include "../devices/ifc_deviceprovider.h"
#include "..\..\Library\ml_pmp/pmp.h"

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
	HRESULT Unregister();
	size_t IncrementActivity();
	size_t DecrementActivity();

private:
	void Lock();
	void Unlock();
	DWORD DiscoveryThread();
	friend static int DeviceProvider_DiscoveryThreadStarter(HANDLE handle, void *user_data, intptr_t id);

protected:
	size_t ref;
	size_t activity;
	CRITICAL_SECTION lock;
	api_devicemanager *manager;
	HANDLE readyEvent;
	BOOL cancelDiscovery;

protected:
	RECVS_DISPATCH;
};