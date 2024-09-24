#ifndef NULLOSFT_DROPBOX_PLUGIN_COMMANDLINK_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_COMMANDLINK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define NWC_COMMANDLINKA		"NullsoftCommandLink"
#define NWC_COMMANDLINKW		L"NullsoftCommandLink"

#ifdef UNICODE
#define NWC_COMMANDLINK		NWC_COMMANDLINKW
#else
#define NWC_COMMANDLINK		NWC_COMMANDLINKA
#endif // !UNICODE

// styles
#define CLS_HOTTRACK		0x0001
#define CLS_TRACKVISITED	0x0002
#define CLS_DEFAULTCOLORS	0x0008
#define CLS_ALWAYSUNDERLINE	0x0010
#define CLS_HIGHLIGHTCOLOR  0x0020 // enables CLM_SETHIGHLIGHTCOLOR/CLM_GETHIGHLIGHTCOLOR

// states
#define CLIS_VISITED		0x0001
#define CLIS_HOT			0x0002
#define CLIS_PRESSED		0x0004
#define CLIS_FOCUSED		0x0008
#define CLIS_HIDEFOCUS		0x0010

#define CommandLink_CreateWindow(__windowStyleEx, __linkText, __windowStyle, __x, __y, __cx, __cy, __parentWindow, __controlId)\
	CreateWindowEx((__windowStyleEx), NWC_COMMANDLINK, (__linkText), (__windowStyle),\
		(__x), (__y), (__cx), (__cy), (__parentWindow), (HMENU)(INT_PTR)(__controlId), NULL, 0)
						

#define CLM_FIRST			(WM_USER + 1)

#define CLM_GETIDEALHEIGHT	(CLM_FIRST + 0)
#define CommandLink_GetIdealHeight(/*HNWD*/__hwnd)\
	((INT)SendMessageW((__hwnd), CLM_GETIDEALHEIGHT, 0, 0L))

#define CLM_GETIDEALSIZE		(CLM_FIRST + 1)
#define CommandLink_GetIdealSize(/*HNWD*/ __hwnd, /*LPSIZE*/ __sizeOut)\
	((BOOL)SendMessageW((__hwnd), CLM_GETIDEALSIZE, 0, (LPARAM)(__sizeOut)))

#define CLM_RESETVISITED		(CLM_FIRST + 2)
#define CommandLink_ResetVisited(/*HNWD*/__hwnd)\
	(SendMessageW((__hwnd), CLM_RESETVISITED, 0, 0L))

#define CLM_GETMARGINS		(CLM_FIRST + 3)  //lParam = (RECT*)*margins; Return TRUE on success
#define CommandLink_GetMargins(/*HNWD*/ __hwnd, /*LPRECT*/ __rectOut)\
	((BOOL)SendMessageW((__hwnd), CLM_GETMARGINS, 0, (LPARAM)(__rectOut)))

#define CLM_SETMARGINS		(CLM_FIRST + 4)  //lParam = (RECT*)*margins; Return TRUE on success
#define CommandLink_SetMargins(/*HNWD*/ __hwnd, /*LPCRECT*/ __rectIn)\
	((BOOL)SendMessageW((__hwnd), CLM_SETMARGINS, 0, (LPARAM)(__rectIn)))

#define CLM_SETBACKCOLOR		(CLM_FIRST + 5)  //lParam = (LPARAM)rgb; Return TRUE on success
#define CommandLink_SetBackColor(/*HNWD*/ __hwnd, /*COLORREF*/ __rgb)\
	((BOOL)SendMessageW((__hwnd), CLM_SETBACKCOLOR, 0, (LPARAM)(__rgb)))

#define CLM_GETBACKCOLOR		(CLM_FIRST + 6)  //Return COLORREF
#define CommandLink_GetBackColor(/*HNWD*/ __hwnd)\
	((COLORREF)SendMessageW((__hwnd), CLM_GETBACKCOLOR, 0, 0L))

#define CLM_SETTEXTCOLOR		(CLM_FIRST + 7)  //lParam = (LPARAM)rgb; Return TRUE on success
#define CommandLink_SetTextColor(/*HNWD*/ __hwnd, /*COLORREF*/ __rgb)\
	((BOOL)SendMessageW((__hwnd), CLM_SETTEXTCOLOR, 0, (LPARAM)(__rgb)))

#define CLM_GETTEXTCOLOR		(CLM_FIRST + 8)  //Return COLORREF
#define CommandLink_GetTextColor(/*HNWD*/ __hwnd)\
	((COLORREF)SendMessageW((__hwnd), CLM_GETTEXTCOLOR, 0, 0L))

#define CLM_SETVISITEDCOLOR		(CLM_FIRST + 9)  //lParam = (LPARAM)rgb; Return TRUE on success
#define CommandLink_SetVisitedColor(/*HNWD*/ __hwnd, /*COLORREF*/ __rgb)\
	((BOOL)SendMessageW((__hwnd), CLM_SETVISITEDCOLOR, 0, (LPARAM)(__rgb)))

#define CLM_GETVISITEDCOLOR		(CLM_FIRST + 10)  //Return COLORREF
#define CommandLink_GetVisitedColor(/*HNWD*/ __hwnd)\
	((COLORREF)SendMessageW((__hwnd), CLM_GETVISITEDCOLOR, 0, 0L))

#define CLM_SETHIGHLIGHTCOLOR	(CLM_FIRST + 11)  //lParam = (LPARAM)rgb; Return TRUE on success
#define CommandLink_SetHighlightColor(/*HNWD*/ __hwnd, /*COLORREF*/ __rgb)\
	((BOOL)SendMessageW((__hwnd), CLM_SETHIGHLIGHTCOLOR, 0, (LPARAM)(__rgb)))

#define CLM_GETHIGHLIGHTCOLOR	(CLM_FIRST + 12)  //Return COLORREF
#define CommandLink_GetHighlightColor(/*HNWD*/ __hwnd)\
	((COLORREF)SendMessageW((__hwnd), CLM_GETHIGHLIGHTCOLOR, 0, 0L))



// notifications
// NM_CLICK
// NM_SETFOCUS
// NM_KILLFOCUS



// internal call ( do not use)
EXTERN_C BOOL CommandLink_RegisterClass(HINSTANCE hInstance);



#endif //NULLOSFT_DROPBOX_PLUGIN_COMMANDLINK_HEADER