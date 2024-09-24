#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_ENUMERATOR_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_ENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {21135710-8161-46c8-83C5-134CC1E832DF}
static const GUID IFC_DeviceObjectEnum = 
{ 0x21135710, 0x8161, 0x46c8, { 0x83, 0xc5, 0x13, 0x4c, 0xc1, 0xe8, 0x32, 0xdf } };


#include <bfc/dispatch.h>

class ifc_deviceobject;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_deviceobjectenum : public Dispatchable
{
protected:
	ifc_deviceobjectenum() {}
	~ifc_deviceobjectenum() {}

public:
	HRESULT Next(ifc_deviceobject **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

public:
	DISPATCH_CODES
	{
		API_NEXT = 10,
		API_RESET = 20,
		API_SKIP = 30,
		API_GETCOUNT = 40,

	};
};

inline HRESULT ifc_deviceobjectenum::Next(ifc_deviceobject **buffer, size_t bufferMax, size_t *fetched)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, buffer, bufferMax, fetched);
}

inline HRESULT ifc_deviceobjectenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_deviceobjectenum::Skip(size_t count)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, count);
}

inline HRESULT ifc_deviceobjectenum::GetCount(size_t *count)
{
	return _call(API_GETCOUNT, (HRESULT)E_NOTIMPL, count);
}

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_OBJECT_ENUMERATOR_INTERFACE_HEADER
