#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_STORE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_STORE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_deviceobject.h"
#include "./deviceObjectEnum.h"
#include <vector>

class DeviceObjectStore;

typedef void (__cdecl *DeviceObjectCallback)(DeviceObjectStore * /*store*/, 
											ifc_deviceobject * /*object*/, 
											void * /*user*/);

typedef ifc_deviceobject *(_cdecl *DeviceObjectCreator)(const char * /*name*/, 
															  void * /*user*/);

class DeviceObjectStore
{
public:
	DeviceObjectStore(DeviceObjectCallback addCallback, 
					  DeviceObjectCallback removeCallback, 
					  void *callbackData);

	~DeviceObjectStore();

public:
	void Lock();
	void Unlock();
	CRITICAL_SECTION *GetLock();

	HRESULT Add(ifc_deviceobject *object);
	size_t AddRange(ifc_deviceobject **objects, size_t count);
	size_t AddIndirect(const char **names, size_t count, DeviceObjectCreator callback, void *user);

	HRESULT Remove(const char *name);
	void RemoveAll();
	HRESULT Find(const char *name, ifc_deviceobject **object);
	HRESULT Enumerate(DeviceObjectEnum **enumerator);

private:
	std::vector<ifc_deviceobject*> list;
	CRITICAL_SECTION lock;
	DeviceObjectCallback addCallback;
	DeviceObjectCallback removeCallback;
	void *callbackData;
	
	

};

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_STORE_HEADER