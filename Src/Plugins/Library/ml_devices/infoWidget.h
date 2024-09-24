#ifndef _NULLSOFT_WINAMP_ML_DEVICES_INFO_WIDGET_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_INFO_WIDGET_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define WIDGET_TYPE_UNKNOWN				0
#define WIDGET_TYPE_WELCOME				1
#define WIDGET_TYPE_SERVICE_ERROR		2
#define WIDGET_TYPE_VIEW_ERROR			3

HWND InfoWidget_CreateWindow(unsigned int type,
							 const wchar_t *title,
							 const wchar_t *text,
							 const wchar_t *imagePath,
							 HWND parentWindow, 
							 int x, 
							 int y, 
							 int width, 
							 int height, 
							 BOOL border,
							 unsigned int controlId);

#endif //_NULLSOFT_WINAMP_ML_DEVICES_INFO_WIDGET_HEADER