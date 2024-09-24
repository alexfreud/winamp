#ifndef _NULLSOFT_WINAMP_ML_DEVICES_EMBEDED_EDITOR_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_EMBEDED_EDITOR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


typedef void (CALLBACK *EmbeddedEditorFinishCb)(HWND /*editorWindow*/,  BOOL /*canceled*/, 
												const wchar_t * /*text*/, void * /*user*/);


BOOL
EmbeddedEditor_Attach(HWND hwnd, 
					  EmbeddedEditorFinishCb callback, 
					  void *user);

BOOL
EmbeddedEditor_AdjustWindowRectEx(RECT *rect, 
								  unsigned long styleEx, 
								  unsigned long style);


#define EMBEDDEDEDITOR_WM_FIRST					(WM_USER + 10)

#define EMBEDDEDEDITOR_WM_SET_TEXT_COLOR		(EMBEDDEDEDITOR_WM_FIRST + 0)
#define EMBEDDEDEDITOR_SET_TEXT_COLOR(/*HWND*/ _hwnd, /*COLORREF*/ _color)\
		((COLORREF)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_SET_TEXT_COLOR, 0, (LPARAM)(_color)))

#define EMBEDDEDEDITOR_WM_GET_TEXT_COLOR		(EMBEDDEDEDITOR_WM_FIRST + 1)
#define EMBEDDEDEDITOR_GET_TEXT_COLOR(/*HWND*/ _hwnd)\
		((COLORREF)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_GET_TEXT_COLOR, 0, 0L))

#define EMBEDDEDEDITOR_WM_SET_BACK_COLOR		(EMBEDDEDEDITOR_WM_FIRST + 2)
#define EMBEDDEDEDITOR_SET_BACK_COLOR(/*HWND*/ _hwnd, /*COLORREF*/ _color)\
		((COLORREF)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_SET_BACK_COLOR, 0, (LPARAM)(_color)))

#define EMBEDDEDEDITOR_WM_GET_BACK_COLOR		(EMBEDDEDEDITOR_WM_FIRST + 3)
#define EMBEDDEDEDITOR_GET_BACK_COLOR(/*HWND*/ _hwnd)\
		((COLORREF)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_GET_BACK_COLOR, 0, 0L))

#define EMBEDDEDEDITOR_WM_SET_BORDER_COLOR			(EMBEDDEDEDITOR_WM_FIRST + 4)
#define EMBEDDEDEDITOR_SET_BORDER_COLOR(/*HWND*/ _hwnd, /*COLORREF*/ _color)\
		((COLORREF)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_SET_BORDER_COLOR, 0, (LPARAM)(_color)))

#define EMBEDDEDEDITOR_WM_GET_BORDER_COLOR		(EMBEDDEDEDITOR_WM_FIRST + 5)
#define EMBEDDEDEDITOR_GET_BORDER_COLOR(/*HWND*/ _hwnd)\
		((COLORREF)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_GET_BORDER_COLOR, 0, 0L))

#define EMBEDDEDEDITOR_WM_SET_USER_DATA			(EMBEDDEDEDITOR_WM_FIRST + 6)
#define EMBEDDEDEDITOR_SET_USER_DATA(/*HWND*/ _hwnd, /*void* */ _user)\
		((void*)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_SET_USER_DATA, 0, (LPARAM)(_user)))

#define EMBEDDEDEDITOR_WM_GET_USER_DATA			(EMBEDDEDEDITOR_WM_FIRST + 7)
#define EMBEDDEDEDITOR_GET_USER_DATA(/*HWND*/ _hwnd)\
		((void*)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_GET_USER_DATA, 0, 0L))

#define EMBEDDEDEDITOR_WM_SET_ANCHOR_POINT		(EMBEDDEDEDITOR_WM_FIRST + 8)
#define EMBEDDEDEDITOR_SET_ANCHOR_POINT(/*HWND*/ _hwnd, /*long*/ _x, /*long*/ _y)\
		((BOOL)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_SET_ANCHOR_POINT, (WPARAM)(_x), (LPARAM)(_y)))

#define EMBEDDEDEDITOR_WM_GET_ANCHOR_POINT		(EMBEDDEDEDITOR_WM_FIRST + 9)
#define EMBEDDEDEDITOR_GET_ANCHOR_POINT(/*HWND*/ _hwnd, /*long* */ _x, /*long* */ _y)\
		((BOOL)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_GET_ANCHOR_POINT, (WPARAM)(_x), (LPARAM)(_y)))

#define EMBEDDEDEDITOR_WM_SET_MAX_SIZE			(EMBEDDEDEDITOR_WM_FIRST + 10)
#define EMBEDDEDEDITOR_SET_MAX_SIZE(/*HWND*/ _hwnd, /*long*/ _width, /*long*/ _height)\
		((BOOL)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_SET_MAX_SIZE, (WPARAM)(_width), (LPARAM)(_height)))

#define EMBEDDEDEDITOR_WM_GET_MAX_SIZE			(EMBEDDEDEDITOR_WM_FIRST + 11)
#define EMBEDDEDEDITOR_GET_MAX_SIZE(/*HWND*/ _hwnd, /*long* */ _width, /*long* */ _height)\
		((BOOL)SendMessageW((_hwnd), EMBEDDEDEDITOR_GET_MAX_SIZE, (WPARAM)(_width), (LPARAM)(_height)))

#define EMBEDDEDEDITOR_WM_END_EDITING			(EMBEDDEDEDITOR_WM_FIRST + 12)
#define EMBEDDEDEDITOR_END_EDITING(/*HWND*/ _hwnd, /*BOOL*/ _cancel)\
		((BOOL)SendMessageW((_hwnd), EMBEDDEDEDITOR_WM_END_EDITING, (WPARAM)(_cancel), 0L))
#endif //_NULLSOFT_WINAMP_ML_DEVICES_EMBEDED_EDITOR_HEADER