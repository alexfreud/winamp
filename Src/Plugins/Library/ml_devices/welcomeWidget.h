#ifndef _NULLSOFT_WINAMP_ML_DEVICES_WELCOME_WIDGET_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_WELCOME_WIDGET_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>



HWND WelcomeWidget_CreateWindow(HWND parentWindow, 
							  int x, 
							  int y, 
							  int width, 
							  int height,
							  BOOL border,
							  unsigned int controlId);

#endif //_NULLSOFT_WINAMP_ML_DEVICES_WELCOME_WIDGET_HEADER