#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_devicesupportedcommand.h"


class DeviceSupportedCommand : public ifc_devicesupportedcommand
{
protected:
	DeviceSupportedCommand();
	~DeviceSupportedCommand();

public:
	static HRESULT CreateInstance(const char *name, 
								  DeviceSupportedCommand **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_devicesupportedcommand */
	const char *GetName(); 
	HRESULT GetFlags(DeviceCommandFlags *flags);
	
public:
	HRESULT SetFlags(DeviceCommandFlags mask, DeviceCommandFlags value);
	HRESULT Clone(DeviceSupportedCommand **instance);
	
protected:
	size_t ref;
	char *name;
	DeviceCommandFlags flags;
	

protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_HEADER