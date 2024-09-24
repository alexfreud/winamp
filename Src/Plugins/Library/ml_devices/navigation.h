#ifndef _NULLSOFT_WINAMP_ML_DEVICES_NAVIGATION_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_NAVIGATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

BOOL 
Navigation_Initialize(void);

void 
Navigation_Uninitialize(void);

BOOL 
Navigation_SelectDevice(const char *name);

BOOL 
Navigation_EditDeviceTitle(const char *name);

BOOL 
Navigation_ProcessMessage(INT msg, 
							   INT_PTR param1, 
							   INT_PTR param2, 
							   INT_PTR param3, 
							   INT_PTR *result);

#endif //_NULLSOFT_WINAMP_ML_DEVICES_NAVIGATION_HEADER
