#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_STORE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_STORE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#include "./ifc_devicesupportedcommandstore.h"
#include "./deviceSupportedCommand.h"
#include "./ifc_devicesupportedcommandenum.h"
#include <vector>

class DeviceSupportedCommandStore : public ifc_devicesupportedcommandstore
{

protected:
	DeviceSupportedCommandStore();
	~DeviceSupportedCommandStore();

public:
	static HRESULT CreateInstance(DeviceSupportedCommandStore **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_devicesupportedcommandstore*/
	HRESULT Add(const char *name, DeviceCommandFlags flags);
	HRESULT Remove(const char *name);
	HRESULT RemoveAll();

	HRESULT GetFlags(const char *name, DeviceCommandFlags *flagsOut);
	HRESULT SetFlags(const char *name, DeviceCommandFlags mask, DeviceCommandFlags value);
	
	HRESULT Get(const char *name, ifc_devicesupportedcommand **command);
	HRESULT GetActive(ifc_devicesupportedcommand **command);
	
	HRESULT Enumerate(ifc_devicesupportedcommandenum **enumerator);

	HRESULT Clone(ifc_devicesupportedcommandstore **instance, BOOL fullCopy);
	
public:
	void Lock();
	void Unlock();

protected:
	static int SearchCallback(const void *key, const void *element);
	static int SortCallback(const void *element1, const void *element2);
	DeviceSupportedCommand *Find(const char *name, size_t *indexOut);

protected:
	typedef std::vector<DeviceSupportedCommand*> CommandList;

protected:
	size_t ref;
	CommandList commandList;
	CRITICAL_SECTION lock;

protected:
	RECVS_DISPATCH;
	
	

};

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_SUPPORTED_COMMAND_STORE_HEADER