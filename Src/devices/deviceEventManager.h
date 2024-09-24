#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_EVENT_MANAGER_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_EVENT_MANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_deviceevent.h"
#include "./ifc_deviceeventmanager.h"
#include <vector>

class DeviceEventManager : public ifc_deviceeventmanager
{

protected:
	DeviceEventManager();
	~DeviceEventManager();

public:
	static HRESULT CreateInstance(DeviceEventManager **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_deviceeventmanager */
	HRESULT Advise(ifc_deviceevent *handler);
	HRESULT Unadvise(ifc_deviceevent *handler);

	void Notify_IconChanged(ifc_device *device);
	void Notify_DisplayNameChanged(ifc_device *device, const wchar_t *displayName);
	void Notify_AttachmentChanged(ifc_device *device, BOOL attached);
	void Notify_VisibilityChanged(ifc_device *device, BOOL visible);
	void Notify_TotalSpaceChanged(ifc_device *device, uint64_t space);
	void Notify_UsedSpaceChanged(ifc_device *device, uint64_t space);
	void Notfiy_CommandChanged(ifc_device *device);
	void Notify_ActivityStarted(ifc_device *device, ifc_deviceactivity *activity);
	void Notify_ActivityFinished(ifc_device *device, ifc_deviceactivity *activity);
	void Notify_ActivityChanged(ifc_device *device, ifc_deviceactivity *activity);
	void Notify_ModelChanged(ifc_device *device, const wchar_t *model);
	void Notify_StatusChanged(ifc_device *device, const wchar_t *status);

public:
	void Lock();
	void Unlock();

protected:
	typedef std::vector<ifc_deviceevent*> HandlerList;

protected:
	size_t ref;
	HandlerList handlerList;
	CRITICAL_SECTION lock;

protected:
	RECVS_DISPATCH;
	
	

};

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_EVENT_MANAGER_HEADER