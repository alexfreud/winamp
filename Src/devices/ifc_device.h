#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {CAC1D7F6-AA27-47eb-BAF7-512BAAE04F07}
static const GUID IFC_Device = 
{ 0xcac1d7f6, 0xaa27, 0x47eb, { 0xba, 0xf7, 0x51, 0x2b, 0xaa, 0xe0, 0x4f, 0x7 } };

#include "ifc_deviceobject.h"

class ifc_deviceevent;
class ifc_devicesupportedcommandenum;
class ifc_deviceactivity;

typedef enum DeviceCommandFlags DeviceCommandFlags;
typedef enum DeviceCommandContext DeviceCommandContext;


class __declspec(novtable) ifc_device : public ifc_deviceobject
{
protected:
	ifc_device() {}
	~ifc_device() {}

public:
	const char *GetType();
	const char *GetDisplayType();
	const char *GetConnection();

	BOOL GetHidden();
	
	HRESULT GetTotalSpace(uint64_t *size);
	HRESULT GetUsedSpace(uint64_t *size);

	BOOL GetAttached();
	HRESULT Attach(HWND hostWindow);
	HRESULT Detach(HWND hostWindow);
	
	HRESULT EnumerateCommands(ifc_devicesupportedcommandenum **enumerator, DeviceCommandContext context);
	HRESULT SendCommand(const char *command, HWND hostWindow, ULONG_PTR param);
	HRESULT GetCommandFlags(const char *command, DeviceCommandFlags *flags);

	HRESULT GetActivity(ifc_deviceactivity **activity);

	HRESULT Advise(ifc_deviceevent *handler);
	HRESULT Unadvise(ifc_deviceevent *handler);

	HWND CreateView(HWND parentWindow);
	void SetNavigationItem(void *navigationItem); 

	HRESULT GetDropSupported(unsigned int dataType);
	HRESULT Drop(void *data, unsigned int dataType);

	HRESULT SetDisplayName(const wchar_t *displayName);
	
	HRESULT GetModel(wchar_t *buffer, size_t bufferSize); // - end-user-visible name for the end produt.
	HRESULT GetStatus(wchar_t *buffer, size_t bufferSize); // - free-form status message  (example: "Last synchronized 19:20 01/10/10").

public:
	DISPATCH_CODES
	{
		API_GETTYPE				= 10,
		API_GETCONNECTION		= 20,
		API_GETHIDDEN			= 30,
		API_GETTOTALSPACE		= 40,
		API_GETUSEDSPACE		= 50,
		API_GETATTACHED			= 60,
		API_ATTACH				= 70,
		API_DETACH				= 80,
		API_ENUMERATECOMMANDS	= 90,
		API_SENDCOMMAND			= 100,
		API_GETCOMMANDFLAGS		= 110,
		API_GETACTIVITY			= 120,
		API_ADVISE				= 130,
		API_UNADVISE			= 140,
		API_CREATEVIEW			= 150,
		API_SETNAVIGATIONITEM	= 160,
		API_GETDROPSUPPORTED	= 170,
		API_DROP				= 180,
		API_SETDISPLAYNAME		= 190,
		API_GETMODEL			= 200,
		API_GETSTATUS			= 210,
		API_GETDISPLAYTYPE		= 220,
	};
};


inline const char *ifc_device::GetType()
{
	return _call(API_GETTYPE, (const char*)NULL);
}

inline const char *ifc_device::GetDisplayType()
{
	return _call(API_GETDISPLAYTYPE, (const char*)NULL);
}

inline const char *ifc_device::GetConnection()
{
	return _call(API_GETCONNECTION, (const char*)NULL);
}

inline BOOL ifc_device::GetHidden()
{
	return _call(API_GETHIDDEN, (BOOL)FALSE);
}

inline HRESULT ifc_device::GetTotalSpace(uint64_t *size)
{
	return _call(API_GETTOTALSPACE, (HRESULT)E_NOTIMPL, size);
}

inline HRESULT ifc_device::GetUsedSpace(uint64_t *size)
{
	return _call(API_GETUSEDSPACE, (HRESULT)E_NOTIMPL, size);
}

inline BOOL ifc_device::GetAttached()
{
	return _call(API_GETATTACHED, (BOOL)FALSE);
}

inline HRESULT ifc_device::Attach(HWND hostWindow)
{
	return _call(API_ATTACH, (HRESULT)E_NOTIMPL, hostWindow);
}

inline HRESULT ifc_device::Detach(HWND hostWindow)
{
	return _call(API_DETACH, (HRESULT)E_NOTIMPL, hostWindow);
}

inline HRESULT ifc_device::EnumerateCommands(ifc_devicesupportedcommandenum **enumerator, DeviceCommandContext context)
{
	return _call(API_ENUMERATECOMMANDS, (HRESULT)E_NOTIMPL, enumerator, context);
}

inline HRESULT ifc_device::SendCommand(const char *command, HWND hostWindow, ULONG_PTR param)
{
	return _call(API_SENDCOMMAND, (HRESULT)E_NOTIMPL, command, hostWindow, param);
}

inline HRESULT ifc_device::GetCommandFlags(const char *command, DeviceCommandFlags *flags)
{
	return _call(API_GETCOMMANDFLAGS, (HRESULT)E_NOTIMPL, command, flags);
}

inline HRESULT ifc_device::GetActivity(ifc_deviceactivity **activity)
{
	return _call(API_GETACTIVITY, (HRESULT)E_NOTIMPL, activity);
}

inline HRESULT ifc_device::Advise(ifc_deviceevent *handler)
{
	return _call(API_ADVISE, (HRESULT)E_NOTIMPL, handler);
}

inline HRESULT ifc_device::Unadvise(ifc_deviceevent *handler)
{
	return _call(API_UNADVISE, (HRESULT)E_NOTIMPL, handler);
}

inline HWND ifc_device::CreateView(HWND parentWindow)
{
	return _call(API_CREATEVIEW, (HWND)NULL, parentWindow);
}

inline void ifc_device::SetNavigationItem(void *navigationItem)
{
	_voidcall(API_SETNAVIGATIONITEM, navigationItem);
}

inline HRESULT ifc_device::GetDropSupported(unsigned int dataType)
{
	return _call(API_GETDROPSUPPORTED, (HRESULT)E_NOTIMPL, dataType);
}

inline HRESULT ifc_device::Drop(void *data, unsigned int dataType)
{
	return _call(API_DROP, (HRESULT)E_NOTIMPL, data, dataType);
}

inline HRESULT ifc_device::SetDisplayName(const wchar_t *displayName)
{
	return _call(API_SETDISPLAYNAME, (HRESULT)E_NOTIMPL, displayName);
}

inline HRESULT ifc_device::GetModel(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETMODEL, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

inline HRESULT ifc_device::GetStatus(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETSTATUS, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_INTERFACE_HEADER
