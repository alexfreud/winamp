#ifndef _NULLSOFT_WINAMP_ML_DEVICES_WDGET_HOST_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_WIDGET_HOST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

typedef HWND (*WidgetCreateProc)(HWND hostWindow, void *user);

HWND 
WidgetHost_Create(unsigned int windowStyle, 
				  int x, 
				  int y, 
				  int width, 
				  int height, 
				  HWND parentWindow, 
				  WidgetCreateProc createProc, 
				  void *createParam);


#endif //_NULLSOFT_WINAMP_ML_DEVICES_WIDGET_HOST_HEADER
