#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {4D0B21E9-B3D0-4f51-8264-575CBF6A2CFA}
static const GUID IFC_DeviceEvent = 
{ 0x4d0b21e9, 0xb3d0, 0x4f51, { 0x82, 0x64, 0x57, 0x5c, 0xbf, 0x6a, 0x2c, 0xfa } };

#include <bfc/dispatch.h>

class ifc_device;
class ifc_deviceactivity;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_deviceevent : public Dispatchable
{
protected:
	ifc_deviceevent() {}
	~ifc_deviceevent() {}

public:
	void IconChanged(ifc_device *device);
	void DisplayNameChanged(ifc_device *device, const wchar_t *displayName);
	void AttachmentChanged(ifc_device *device, BOOL attached);
	void VisibilityChanged(ifc_device *device, BOOL visible);
	void TotalSpaceChanged(ifc_device *device, uint64_t space);
	void UsedSpaceChanged(ifc_device *device, uint64_t space);
	void CommandChanged(ifc_device *device);
	void ActivityStarted(ifc_device *device, ifc_deviceactivity *activity);
	void ActivityFinished(ifc_device *device, ifc_deviceactivity *activity);
	void ActivityChanged(ifc_device *device, ifc_deviceactivity *activity);
	void ModelChanged(ifc_device *device, const wchar_t *model);
	void StatusChanged(ifc_device *device, const wchar_t *status);
	

public:
	DISPATCH_CODES
	{
		API_ICONCHANGED	= 10,
		API_DISPLAYNAMECHANGED	= 20,
		API_ATTACHMENTCHANGED = 30,
		API_VISIBILITYCHANGED = 40,
		API_TOTALSPACECHANGED = 50,
		API_USEDSPACECHANGED = 60,
		API_COMMANDCHANGED = 70,
		API_ACTIVITYSTARTED = 80,
		API_ACTIVITYFINISHED = 90,
		API_ACTIVITYCHANGED = 100,
		API_MODELCHANGED = 110,
		API_STATUSCHANGED = 120,
	};
};

inline void ifc_deviceevent::IconChanged(ifc_device *device)
{
	_voidcall(API_ICONCHANGED, device);
}

inline void ifc_deviceevent::DisplayNameChanged(ifc_device *device, const wchar_t *displayName)
{
	_voidcall(API_DISPLAYNAMECHANGED, device, displayName);
}

inline void ifc_deviceevent::AttachmentChanged(ifc_device *device, BOOL attached)
{
	_voidcall(API_ATTACHMENTCHANGED, device, attached);
}

inline void ifc_deviceevent::VisibilityChanged(ifc_device *device, BOOL visible)
{
	_voidcall(API_VISIBILITYCHANGED, device, visible);
}

inline void ifc_deviceevent::TotalSpaceChanged(ifc_device *device, uint64_t space)
{
	_voidcall(API_TOTALSPACECHANGED, device, space);
}

inline void ifc_deviceevent::UsedSpaceChanged(ifc_device *device, uint64_t space)
{
	_voidcall(API_USEDSPACECHANGED, device, space);
}

inline void ifc_deviceevent::CommandChanged(ifc_device *device)
{
	_voidcall(API_COMMANDCHANGED, device);
}

inline void ifc_deviceevent::ActivityStarted(ifc_device *device, ifc_deviceactivity *activity)
{
	_voidcall(API_ACTIVITYSTARTED, device, activity);
}

inline void ifc_deviceevent::ActivityFinished(ifc_device *device, ifc_deviceactivity *activity)
{
	_voidcall(API_ACTIVITYFINISHED, device, activity);
}

inline void ifc_deviceevent::ActivityChanged(ifc_device *device, ifc_deviceactivity *activity)
{
	_voidcall(API_ACTIVITYCHANGED, device, activity);
}

inline void ifc_deviceevent::ModelChanged(ifc_device *device, const wchar_t *model)
{
	_voidcall(API_MODELCHANGED, device, model);
}

inline void ifc_deviceevent::StatusChanged(ifc_device *device, const wchar_t *status)
{
	_voidcall(API_STATUSCHANGED, device, status);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_EVENT_INTERFACE_HEADER