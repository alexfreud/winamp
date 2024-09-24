#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_TYPE_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_TYPE_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {C2AB1F06-1D88-4d21-A8F4-1B36A1929281}
static const GUID IFC_DeviceType = 
{ 0xc2ab1f06, 0x1d88, 0x4d21, { 0xa8, 0xf4, 0x1b, 0x36, 0xa1, 0x92, 0x92, 0x81 } };


#include "ifc_deviceobject.h"

class __declspec(novtable) ifc_devicetype : public ifc_deviceobject
{
protected:
	ifc_devicetype() {}
	~ifc_devicetype() {}

};

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_TYPE_INTERFACE_HEADER