#ifndef _NULLSOFT_WINAMP_ML_DEVICES_WIDGET_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_WIDGET_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./widgetStyle.h"

#define WIDGET_WINDOW_CLASS		L"NullsoftDevicesWidget"


typedef BOOL (*WidgetInitCallback)(HWND /*hwnd*/, void** /*self_out*/, void* /*param*/);
typedef void (*WidgetDestroyCallback)(void* /*self*/, HWND /*hwnd*/);
typedef void (*WidgetLayoutCallback)(void* /*self*/, HWND /*hwnd*/, WidgetStyle* /*style*/, const RECT* /*clientRect*/, SIZE* /*viewSize*/, BOOL /*redraw*/);
typedef BOOL (*WidgetPaintCallback)(void* /*self*/, HWND /*hwnd*/, WidgetStyle* /*style*/, HDC /*hdc*/, const RECT* /*paintRect*/, BOOL /*erase*/);
typedef BOOL (*WidgetMessageCallback)(void* /*self*/, HWND /*hwnd*/, unsigned int /*uMsg*/, WPARAM  /*wParam*/, LPARAM /*lParam*/, LRESULT* /*result*/);
typedef void (*WidgetStyleCallback)(void* /*self*/, HWND /*hwnd*/, WidgetStyle* /*style*/);
typedef BOOL (*WidgetMouseCallback)(void* /*self*/, HWND /*hwnd*/, unsigned int /*vKeys*/, const POINT* /*cursor*/);
typedef INT  (*WidgetInputCallback)(void* /*self*/, HWND /*hwnd*/, unsigned int /*vKey*/, MSG* /*message*/);
typedef BOOL (*WidgetKeyCallback)(void* /*self*/, HWND /*hwnd*/, unsigned int /*vKey*/, unsigned int /*flags*/);
typedef void (*WidgetFocusCallback)(void* /*self*/, HWND /*hwnd*/, HWND /*focusHwnd*/, BOOL /*focusReceived*/);
typedef BOOL (*WidgetMenuCallback)(void* /*self*/, HWND /*hwnd*/, HWND /*targetWindow*/, const POINT* /*cursor*/);
typedef void (*WidgetZoomCallback)(void* /*self*/, HWND /*hwnd*/, NMTRBTHUMBPOSCHANGING* /*sliderInfo*/);
typedef void (*WidgetScrollCallback)(void* /*self*/, HWND /*hwnd*/, int* /*dx*/, int* /*dy*/);
typedef BOOL (*WidgetNotifyCallback)(void* /*self*/, HWND /*hwnd*/, NMHDR* /*notification*/, LRESULT* /*result*/);
typedef BOOL (*WidgetHelpCallback)(void* /*self*/, HWND /*hwnd*/, wchar_t* /*buffer*/, size_t /*bufferMax*/);

typedef struct WidgetInterface
{
	WidgetInitCallback init;
	WidgetDestroyCallback destroy;
	WidgetLayoutCallback layout;
	WidgetPaintCallback paint;
	WidgetStyleCallback styleColorChanged;
	WidgetStyleCallback styleFontChanged;
	WidgetMouseCallback mouseMove;
	WidgetMouseCallback leftButtonDown;
	WidgetMouseCallback leftButtonUp;
	WidgetMouseCallback leftButtonDblClk;
	WidgetMouseCallback rightButtonDown;
	WidgetMouseCallback rightButtonUp;
	WidgetKeyCallback keyDown;
	WidgetKeyCallback keyUp;
	WidgetKeyCallback character;
	WidgetInputCallback inputRequest;
	WidgetFocusCallback focusChanged;
	WidgetMenuCallback contextMenu;
	WidgetZoomCallback zoomChanging;
	WidgetScrollCallback scrollBefore;
	WidgetScrollCallback scroll;
	WidgetNotifyCallback notify;
	WidgetHelpCallback help;
	WidgetMessageCallback messageProc;
} WidgetInterface;

HWND 
Widget_CreateWindow(unsigned int type,
					const WidgetInterface *callbacks,
					const wchar_t *text,
					unsigned long windowExStyle,
					unsigned long windowStyle, 
					int x, 
					int y, 
					int width, 
					int height, 
					HWND parentWindow,
					unsigned int controlId,
					void *param);


#define WIDGET_WM_FIRST					(WM_USER + 10)

#define WIDGET_WM_GET_TYPE				(WIDGET_WM_FIRST + 0)
#define WIDGET_GET_TYPE(/*HWND*/ _hwnd)\
		((unsigned int)SendMessageW((_hwnd), WIDGET_WM_GET_TYPE, 0, 0L))

#define WIDGET_WM_GET_SELF				(WIDGET_WM_FIRST + 1)
#define WIDGET_GET_SELF(/*HWND*/ _hwnd, _type)\
		((_type*)SendMessageW((_hwnd), WIDGET_WM_GET_SELF, 0, 0L))

#define WIDGET_WM_SET_STYLE				(WIDGET_WM_FIRST + 2)
#define WIDGET_SET_STYLE(/*HWND*/ _hwnd, /*WidgetStyle* */ _style)\
		((BOOL)SendMessageW((_hwnd), WIDGET_WM_SET_STYLE, 0, (LPARAM)(_style)))

#define WIDGET_WM_GET_STYLE				(WIDGET_WM_FIRST + 3)
#define WIDGET_GET_STYLE(/*HWND*/ _hwnd)\
		((WidgetStyle*)SendMessageW((_hwnd), WIDGET_WM_GET_STYLE, 0, 0L))

#define WIDGET_WM_STYLE_COLOR_CHANGED	(WIDGET_WM_FIRST + 4)
#define WIDGET_STYLE_COLOR_CHANGED(/*HWND*/ _hwnd)\
		(SendMessageW((_hwnd), WIDGET_WM_STYLE_COLOR_CHANGED, 0, 0L))

#define WIDGET_WM_STYLE_FONT_CHANGED	(WIDGET_WM_FIRST + 5)
#define WIDGET_STYLE_FONT_CHANGED(/*HWND*/ _hwnd)\
		(SendMessageW((_hwnd), WIDGET_WM_STYLE_FONT_CHANGED, 0, 0L))

#define WIDGET_WM_FREEZE				(WIDGET_WM_FIRST + 6)
#define WIDGET_FREEZE(/*HWND*/ _hwnd)\
		(SendMessageW((_hwnd), WIDGET_WM_FREEZE, TRUE, 0L))
#define WIDGET_THAW(/*HWND*/ _hwnd)\
		(SendMessageW((_hwnd), WIDGET_WM_FREEZE, FALSE, 0L))

#define WIDGET_WM_SET_SCROLL_POS		(WIDGET_WM_FIRST + 7) // just offsets scroll positions wihtout scrolling view. result = (LRESULT)MAKELPARAM(actualDx,actualDy).
#define WIDGET_SET_SCROLL_POS(/*HWND*/ _hwnd, /*int*/ _dx, /*int*/ _dy, /*BOOL*/ _redraw)\
		(SendMessageW((_hwnd), WIDGET_WM_SET_SCROLL_POS, (WPARAM)(_redraw), MAKELPARAM((_dx), (_dy))))

#define WIDGET_WM_SCROLL				(WIDGET_WM_FIRST + 8)
#define WIDGET_SCROLL(/*HWND*/ _hwnd, /*int*/ _dx, /*int*/ _dy, /*BOOL*/ _redraw)\
		((BOOL)SendMessageW((_hwnd), WIDGET_WM_SCROLL, (WPARAM)(_redraw), MAKELPARAM((_dx), (_dy))))

#define WIDGET_WM_ZOOM_SLIDER_POS_CHANGING	(WIDGET_WM_FIRST + 9)
#define WIDGET_ZOOM_SLIDER_POS_CHANGING(/*HWND*/ _hwnd, /*NMTRBTHUMBPOSCHANGING* */ _sliderInfo)\
		((BOOL)SendMessageW((_hwnd), WIDGET_WM_ZOOM_SLIDER_POS_CHANGING, 0, (LPARAM)(_sliderInfo)))

#define WIDGET_WM_ENABLE_CHILDREN_SCROLL	(WIDGET_WM_FIRST + 10)
#define WIDGET_ENABLE_CHILDREN_SCROLL(/*HWND*/ _hwnd, /*BOOL*/ _enable)\
		((BOOL)SendMessageW((_hwnd), WIDGET_WM_ENABLE_CHILDREN_SCROLL, 0, (LPARAM)(_enable)))

#define WIDGET_WM_GET_CHILDREN_SCROLL_ENABLED	(WIDGET_WM_FIRST + 11)
#define WIDGET_GET_CHILDREN_SCROLL_ENABLED(/*HWND*/ _hwnd)\
		((BOOL)SendMessageW((_hwnd), WIDGET_WM_GET_CHILDREN_SCROLL_ENABLED, 0, 0L))

#define WIDGET_WM_GET_HELP_URL				(WIDGET_WM_FIRST + 12)
#define WIDGET_GET_HELP_URL(/*HWND*/ _hwnd, /* wchar_t* */ _buffer, /*size_t*/ _bufferMax)\
		((BOOL)SendMessageW((_hwnd), WIDGET_WM_GET_HELP_URL, (WPARAM)(_bufferMax), (LPARAM)(_buffer)))

#endif //_NULLSOFT_WINAMP_ML_DEVICES_WIDGET_HEADER