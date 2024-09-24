#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_ENUM_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_ENUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_devicesupportedcommandenum.h"
#include "./ifC_devicesupportedcommand.h"

class DeviceSupportedCommandEnum : public ifc_devicesupportedcommandenum
{

protected:
	DeviceSupportedCommandEnum();
	~DeviceSupportedCommandEnum();

public:
	static HRESULT CreateInstance(ifc_devicesupportedcommand **commands, 
								  size_t count, 
								  DeviceSupportedCommandEnum **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_devicesupportedcommandenum*/
	HRESULT Next(ifc_devicesupportedcommand **buffer, size_t bufferMax, size_t *count);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

protected:
	size_t ref;
	ifc_devicesupportedcommand **commands;
	size_t count;
	size_t cursor;
	
protected:
	RECVS_DISPATCH;
	
	

};

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_ENUM_HEADER