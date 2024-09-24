#ifndef _NULLSOFT_WINAMP_ML_DEVICES_STATUS_BAR_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_STATUS_BAR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


#define STATUSBAR_WINDOW_CLASS		L"NullsoftDevicesStatusBar"

#define STATUS_ERROR		((unsigned int)-1)

HWND 
StatusBar_CreateWindow(unsigned long windowExStyle, 
					   const wchar_t *text, 
					   unsigned long windowStyle, 
					   int x, 
					   int y, 
					   int width, 
					   int height, 
					   HWND parentWindow, 
					   unsigned int controlId);

#define STATUSBAR_WM_FIRST					(WM_USER + 10)

#define STATUSBAR_WM_SET_BACK_BRUSH				(STATUSBAR_WM_FIRST + 0)
#define STATUSBAR_SET_BACK_BRUSH(/*HWND*/ _hwnd, /*HBRSUSH*/ _brush, /*BOOL*/_redraw)\
		((HBRUSH)SendMessageW((_hwnd), STATUSBAR_WM_SET_BACK_BRUSH, (WPARAM)(_redraw), (LPARAM)(_brush)))

#define STATUSBAR_WM_GET_BACK_BRUSH				(STATUSBAR_WM_FIRST + 1)
#define STATUSBAR_GET_BRUSH(/*HWND*/ _hwnd)\
		((HBRUSH)SendMessageW((_hwnd), STATUSBAR_WM_GET_BRUSH, 0, 0L))

#define STATUSBAR_WM_SET_BACK_COLOR			(STATUSBAR_WM_FIRST + 2)
#define STATUSBAR_SET_BACK_COLOR(/*HWND*/ _hwnd, /*COLORREF*/ _color, /*BOOL*/_redraw)\
		((COLORREF)SendMessageW((_hwnd), STATUSBAR_WM_SET_BACK_COLOR, (WPARAM)(_redraw), (LPARAM)(_color)))

#define STATUSBAR_WM_GET_BACK_COLOR			(STATUSBAR_WM_FIRST + 3)
#define STATUSBAR_GET_BACK_COLOR(/*HWND*/ _hwnd)\
		((COLORREF)SendMessageW((_hwnd), STATUSBAR_WM_GET_BACK_COLOR, 0, 0L))

#define STATUSBAR_WM_SET_TEXT_COLOR			(STATUSBAR_WM_FIRST + 4)
#define STATUSBAR_SET_TEXT_COLOR(/*HWND*/ _hwnd, /*COLORREF*/ _color, /*BOOL*/_redraw)\
		((COLORREF)SendMessageW((_hwnd), STATUSBAR_WM_SET_TEXT_COLOR, (WPARAM)(_redraw), (LPARAM)(_color)))

#define STATUSBAR_WM_GET_TEXT_COLOR			(STATUSBAR_WM_FIRST + 5)
#define STATUSBAR_GET_TEXT_COLOR(/*HWND*/ _hwnd)\
		((COLORREF)SendMessageW((_hwnd), STATUSBAR_WM_GET_TEXT_COLOR, 0, 0L))

#define STATUSBAR_WM_GET_IDEAL_HEIGHT		(STATUSBAR_WM_FIRST + 6)
#define STATUSBAR_GET_IDEAL_HEIGHT(/*HWND*/ _hwnd)\
		((int)SendMessageW((_hwnd), STATUSBAR_WM_GET_IDEAL_HEIGHT, 0, 0L))

#define STATUSBAR_WM_ADD_STATUS				(STATUSBAR_WM_FIRST + 7)
#define STATUSBAR_ADD_STATUS(/*HWND*/ _hwnd, /*const wchar_t* */ _statusText)\
		((unsigned int)SendMessageW((_hwnd), STATUSBAR_WM_ADD_STATUS, 0, (LPARAM)(_statusText)))

#define STATUSBAR_WM_REMOVE_STATUS			(STATUSBAR_WM_FIRST + 8)
#define STATUSBAR_REMOVE_STATUS(/*HWND*/ _hwnd, /*unsigned int*/ _statusId)\
		((BOOL)SendMessageW((_hwnd), STATUSBAR_WM_REMOVE_STATUS, (WPARAM)(_statusId), 0L))

#define STATUSBAR_WM_SET_STATUS_TEXT		(STATUSBAR_WM_FIRST + 9)
#define STATUSBAR_SET_STATUS_TEXT(/*HWND*/ _hwnd, /*unsigned int*/ _statusId, /*const wchar_t* */ _statusText)\
		((BOOL)SendMessageW((_hwnd), STATUSBAR_WM_SET_STATUS_TEXT, (WPARAM)(_statusId), (LPARAM)(_statusText)))

#define STATUSBAR_WM_SET_STATUS_RTEXT		(STATUSBAR_WM_FIRST + 10)
#define STATUSBAR_SET_STATUS_RTEXT(/*HWND*/ _hwnd, /*unsigned int*/ _statusId, /*const wchar_t* */ _statusText)\
		((BOOL)SendMessageW((_hwnd), STATUSBAR_WM_SET_STATUS_RTEXT, (WPARAM)(_statusId), (LPARAM)(_statusText)))

#define STATUS_MOVE_TOP		0
#define STATUS_MOVE_BOTTOM	1
#define STATUS_MOVE_UP		2
#define STATUS_MOVE_DOWN	3

#define STATUSBAR_WM_MOVE_STATUS			(STATUSBAR_WM_FIRST + 11)
#define STATUSBAR_MOVE_STATUS(/*HWND*/ _hwnd, /*unsigned int*/ _statusId, /*unsigned int */ _statusMove)\
		((BOOL)SendMessageW((_hwnd), STATUSBAR_WM_MOVE_STATUS, (WPARAM)(_statusId), (LPARAM)(_statusMove)))

#endif //_NULLSOFT_WINAMP_ML_DEVICES_STATUS_BAR_HEADER