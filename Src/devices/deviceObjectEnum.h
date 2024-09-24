#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_ENUMERATOR_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_ENUMERATOR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_deviceobject.h"
#include "./ifc_deviceobjectenum.h"

class DeviceObjectEnum : public ifc_deviceobjectenum
{
protected:
	DeviceObjectEnum();
	~DeviceObjectEnum();

public:
	static HRESULT CreateInstance(ifc_deviceobject **objects, 
								 size_t count, 
								 DeviceObjectEnum **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);
	
	/* ifc_deviceobjectenum */
	HRESULT Next(ifc_deviceobject **objects, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

protected:
	size_t ref;
	ifc_deviceobject **buffer;
	size_t size;
	size_t cursor;

protected:
	RECVS_DISPATCH;
};

#endif // _NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_ENUMERATOR_HEADER