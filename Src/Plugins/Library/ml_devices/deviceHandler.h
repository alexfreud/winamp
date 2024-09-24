#ifndef _NULLSOFT_WINAMP_ML_DEVICES_DEVICE_HANDLER_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_DEVICE_HANDLER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#include "../devices/ifc_deviceevent.h"

class DeviceHandler: public ifc_deviceevent
{
protected:
	DeviceHandler();
	~DeviceHandler();

public:
	static HRESULT CreateInstance(DeviceHandler **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_deviceevent */
	void IconChanged(ifc_device *device);
	void DisplayNameChanged(ifc_device *device, const wchar_t *displayName);
	void AttachmentChanged(ifc_device *device, BOOL attached);
	void VisibilityChanged(ifc_device *device, BOOL visible);
	void TotalSpaceChanged(ifc_device *device, size_t space);
	void UsedSpaceChanged(ifc_device *device, size_t space);
	void CommandChanged(ifc_device *device);
	void ActivityStarted(ifc_device *device, ifc_deviceactivity *activity);
	void ActivityFinished(ifc_device *device, ifc_deviceactivity *activity);
	void ActivityChanged(ifc_device *device, ifc_deviceactivity *activity);
	void ModelChanged(ifc_device *device, const wchar_t *model);
	void StatusChanged(ifc_device *device, const wchar_t *status);

public:
	HRESULT SetRelayWindow(HWND hwnd);
	HRESULT Advise(ifc_device *device);
	HRESULT Unadvise(ifc_device *device);


protected:
	size_t ref;
	HWND relayWindow;

protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_ML_DEVICES_DEVICE_HANDLER_HEADER