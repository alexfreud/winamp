#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_devicecommand.h"
#include "./ifc_devicecommandeditor.h"
#include "./ifc_deviceiconstore.h"
#include "./deviceIconStore.h"

#include <bfc/multipatch.h>

#define MPIID_DEVICECOMMAND				10
#define MPIID_DEVICECOMMANDEDITOR		20


class DeviceCommand :	public MultiPatch<MPIID_DEVICECOMMAND, ifc_devicecommand>,
						public MultiPatch<MPIID_DEVICECOMMANDEDITOR, ifc_devicecommandeditor>
{

protected:
	DeviceCommand();
	~DeviceCommand();

public:
	static HRESULT CreateInstance(const char *name, DeviceCommand **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_devicecommand */
	const char *GetName(); 
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);
	HRESULT GetDescription(wchar_t *buffer, size_t bufferSize);

	/* ifc_devicetypeeditor */
	HRESULT GetIconStore(ifc_deviceiconstore **store);
	HRESULT SetDisplayName(const wchar_t *displayName);
	HRESULT SetDescription(const wchar_t *description);

public:
	void Lock();
	void Unlock();


protected:
	size_t ref;
	char *name;
	wchar_t *displayName;
	wchar_t *description;
	DeviceIconStore *iconStore;
	CRITICAL_SECTION lock;

protected:
	RECVS_MULTIPATCH;
};

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_COMMAND_HEADER
