#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_ACTIVITY_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_ACTIVITY_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {6FE2E838-6C56-4b14-8CE0-FA9B19113DA5}
static const GUID IFC_DeviceActivity = 
{ 0x6fe2e838, 0x6c56, 0x4b14, { 0x8c, 0xe0, 0xfa, 0x9b, 0x19, 0x11, 0x3d, 0xa5 } };


#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_deviceactivity: public Dispatchable
{
protected:
	ifc_deviceactivity() {}
	~ifc_deviceactivity() {}

public:
	BOOL GetActive();
	BOOL GetCancelable();
	HRESULT GetProgress(unsigned int *percentCompleted);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferMax);
	HRESULT GetStatus(wchar_t *buffer, size_t bufferMax);
	HRESULT Cancel(HWND hostWindow);

public:
	DISPATCH_CODES
	{
		API_GETACTIVE = 10,
		API_GETCANCELABLE = 20,
		API_GETPROGRESS = 30,
		API_GETDISPLAYNAME = 40,
		API_GETSTATUS = 50,
		API_CANCEL = 60,
	};
};

inline BOOL ifc_deviceactivity::GetActive()
{
	return _call(API_GETACTIVE, (BOOL)FALSE);
}

inline BOOL ifc_deviceactivity::GetCancelable()
{
	return _call(API_GETCANCELABLE, (BOOL)FALSE);
}

inline HRESULT ifc_deviceactivity::GetProgress(unsigned int *percentCompleted)
{
	return _call(API_GETPROGRESS, (HRESULT)E_NOTIMPL, percentCompleted);
}

inline HRESULT ifc_deviceactivity::GetDisplayName(wchar_t *buffer, size_t bufferMax)
{
	return _call(API_GETDISPLAYNAME, (HRESULT)E_NOTIMPL, buffer, bufferMax);
}

inline HRESULT ifc_deviceactivity::GetStatus(wchar_t *buffer, size_t bufferMax)
{
	return _call(API_GETSTATUS, (HRESULT)E_NOTIMPL, buffer, bufferMax);
}

inline HRESULT ifc_deviceactivity::Cancel(HWND hostWindow)
{
	return _call(API_CANCEL, (HRESULT)E_NOTIMPL, hostWindow);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_ACTIVITY_INTERFACE_HEADER