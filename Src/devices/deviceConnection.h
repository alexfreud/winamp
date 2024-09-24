#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_CONNECTION_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_CONNECTION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_deviceconnection.h"
#include "./ifc_deviceconnectioneditor.h"
#include "./ifc_deviceiconstore.h"
#include "./deviceIconStore.h"

#include <bfc/multipatch.h>

#define MPIID_DEVICECONNECTION			10
#define MPIID_DEVICECONNECTIONEDITOR	20

class DeviceConnection :	public MultiPatch<MPIID_DEVICECONNECTION, ifc_deviceconnection>,
							public MultiPatch<MPIID_DEVICECONNECTIONEDITOR, ifc_deviceconnectioneditor>
{

protected:
	DeviceConnection();
	~DeviceConnection();

public:
	static HRESULT CreateInstance(const char *name,  DeviceConnection **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_deviceconnection */
	const char *GetName(); 
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);

	/* ifc_deviceconnectioneditor */
	HRESULT GetIconStore(ifc_deviceiconstore **store);
	HRESULT SetDisplayName(const wchar_t *displayName);

public:
	void Lock();
	void Unlock();

protected:
	size_t ref;
	char *name;
	wchar_t *displayName;
	DeviceIconStore *iconStore;
	CRITICAL_SECTION lock;

protected:
	RECVS_MULTIPATCH;
};

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_CONNECTION_HEADER
