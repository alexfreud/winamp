#ifndef _NULLSOFT_WINAMP_ML_DEVICES_DEVICE_COMMANDS_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_DEVICE_COMMANDS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

BOOL
DeviceCommands_Register();

BOOL
DeviceCommand_GetSupported(ifc_device *device, 
						   const char *name, 
						   DeviceCommandContext context, 
						   ifc_devicesupportedcommand **commandOut);

BOOL
DeviceCommand_GetEnabled(ifc_device *device, 
						 const char *name, 
						 DeviceCommandContext context);

#endif //_NULLSOFT_WINAMP_ML_DEVICES_DEVICE_COMMANDS_HEADER