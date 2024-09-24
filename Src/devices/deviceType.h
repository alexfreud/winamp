#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_TYPE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_TYPE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_devicetype.h"
#include "./ifc_devicetypeeditor.h"
#include "./ifc_deviceiconstore.h"
#include "./deviceIconStore.h"

#include <bfc/multipatch.h>

#define MPIID_DEVICETYPE				10
#define MPIID_DEVICETYPEEDITOR			20

class DeviceType :	public MultiPatch<MPIID_DEVICETYPE, ifc_devicetype>,
					public MultiPatch<MPIID_DEVICETYPEEDITOR, ifc_devicetypeeditor>
{

protected:
	DeviceType();
	~DeviceType();

public:
	static HRESULT CreateInstance(const char *name,  DeviceType **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_devicetype */
	const char *GetName(); 
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);

	/* ifc_devicetypeeditor */
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

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_TYPE_HEADER
