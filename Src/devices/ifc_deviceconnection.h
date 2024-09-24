#ifndef _NULLSOFT_WINAMP_DEVICES_DEVICE_CONNECTION_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DEVICES_DEVICE_CONNECTION_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {0CF89CC5-AD4E-4c81-AF74-AB2FDB6F56CE}
static const GUID IFC_DeviceConnection = 
{ 0xcf89cc5, 0xad4e, 0x4c81, { 0xaf, 0x74, 0xab, 0x2f, 0xdb, 0x6f, 0x56, 0xce } };

#include "ifc_deviceobject.h"

class __declspec(novtable) ifc_deviceconnection : public ifc_deviceobject
{
protected:
	ifc_deviceconnection() {}
	~ifc_deviceconnection() {}

};

#endif //_NULLSOFT_WINAMP_DEVICES_DEVICE_CONNECTION_INTERFACE_HEADER