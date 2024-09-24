#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_EVENT_MANAGER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_EVENT_MANAGER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {C563C537-DFFC-4210-BB1E-E424426789E2}
static const GUID IFC_DeviceEventManager = 
{ 0xc563c537, 0xdffc, 0x4210, { 0xbb, 0x1e, 0xe4, 0x24, 0x42, 0x67, 0x89, 0xe2 } };

#include <bfc/dispatch.h>

class ifc_deviceevent;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_deviceeventmanager : public Dispatchable
{
protected:
	ifc_deviceeventmanager() {}
	~ifc_deviceeventmanager() {}

public:
	HRESULT Advise(ifc_deviceevent *handler);
	HRESULT Unadvise(ifc_deviceevent *handler);

	void Notify_IconChanged(ifc_device *device);
	void Notify_DisplayNameChanged(ifc_device *device, const wchar_t *displayName);
	void Notify_AttachmentChanged(ifc_device *device, BOOL attached);
	void Notify_VisibilityChanged(ifc_device *device, BOOL visible);
	void Notify_TotalSpaceChanged(ifc_device *device, uint64_t space);
	void Notify_UsedSpaceChanged(ifc_device *device, uint64_t space);
	void Notify_CommandChanged(ifc_device *device);
	void Notify_ActivityStarted(ifc_device *device, ifc_deviceactivity *activity);
	void Notify_ActivityFinished(ifc_device *device, ifc_deviceactivity *activity);
	void Notify_ActivityChanged(ifc_device *device, ifc_deviceactivity *activity);
	void Notify_ModelChanged(ifc_device *device, const wchar_t *model);
	void Notify_StatusChanged(ifc_device *device, const wchar_t *status);
		
public:
	DISPATCH_CODES
	{
		API_ADVISE = 10,
		API_UNADVISE = 20,
		API_NOTIFY_ICONCHANGED	= 30,
		API_NOTIFY_DISPLAYNAMECHANGED	= 40,
		API_NOTIFY_ATTACHMENTCHANGED = 50,
		API_NOTIFY_VISIBILITYCHANGED = 60,
		API_NOTIFY_TOTALSPACECHANGED = 70,
		API_NOTIFY_USEDSPACECHANGED = 80,
		API_NOTIFY_COMMANDCHANGED = 90,
		API_NOTIFY_ACTIVITYSTARTED = 100,
		API_NOTIFY_ACTIVITYFINISHED = 110,
		API_NOTIFY_ACTIVITYCHANGED = 120,
		API_NOTIFY_MODELCHANGED = 130,
		API_NOTIFY_STATUSCHANGED = 140,
	};
};

inline HRESULT ifc_deviceeventmanager::Advise(ifc_deviceevent *handler)
{
	return _call(API_ADVISE, (HRESULT)E_NOTIMPL, handler);
}

inline HRESULT ifc_deviceeventmanager::Unadvise(ifc_deviceevent *handler)
{
	return _call(API_UNADVISE, (HRESULT)E_NOTIMPL, handler);
}

inline void ifc_deviceeventmanager::Notify_IconChanged(ifc_device *device)
{
	_voidcall(API_NOTIFY_ICONCHANGED, device);
}

inline void ifc_deviceeventmanager::Notify_DisplayNameChanged(ifc_device *device, const wchar_t *displayName)
{
	_voidcall(API_NOTIFY_DISPLAYNAMECHANGED, device, displayName);
}

inline void ifc_deviceeventmanager::Notify_AttachmentChanged(ifc_device *device, BOOL attached)
{
	_voidcall(API_NOTIFY_ATTACHMENTCHANGED, device, attached);
}

inline void ifc_deviceeventmanager::Notify_VisibilityChanged(ifc_device *device, BOOL visible)
{
	_voidcall(API_NOTIFY_VISIBILITYCHANGED, device, visible);
}

inline void ifc_deviceeventmanager::Notify_TotalSpaceChanged(ifc_device *device, uint64_t space)
{
	_voidcall(API_NOTIFY_TOTALSPACECHANGED, device, space);
}

inline void ifc_deviceeventmanager::Notify_UsedSpaceChanged(ifc_device *device, uint64_t space)
{
	_voidcall(API_NOTIFY_USEDSPACECHANGED, device, space);
}


inline void ifc_deviceeventmanager::Notify_CommandChanged(ifc_device *device)
{
	_voidcall(API_NOTIFY_COMMANDCHANGED, device);
}

inline void ifc_deviceeventmanager::Notify_ActivityStarted(ifc_device *device, ifc_deviceactivity *activity)
{
	_voidcall(API_NOTIFY_ACTIVITYSTARTED, device, activity);
}

inline void ifc_deviceeventmanager::Notify_ActivityFinished(ifc_device *device, ifc_deviceactivity *activity)
{
	_voidcall(API_NOTIFY_ACTIVITYFINISHED, device, activity);
}

inline void ifc_deviceeventmanager::Notify_ActivityChanged(ifc_device *device, ifc_deviceactivity *activity)
{
	_voidcall(API_NOTIFY_ACTIVITYCHANGED, device, activity);
}

inline void ifc_deviceeventmanager::Notify_ModelChanged(ifc_device *device, const wchar_t *model)
{
	_voidcall(API_NOTIFY_MODELCHANGED, device, model);
}

inline void ifc_deviceeventmanager::Notify_StatusChanged(ifc_device *device, const wchar_t *status)
{
	_voidcall(API_NOTIFY_STATUSCHANGED, device, status);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_EVENT_MANAGER_INTERFACE_HEADER