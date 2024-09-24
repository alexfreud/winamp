#ifndef NULLSOFT_AUTH_LOGIN_NOTIFIER_HEADER
#define NULLSOFT_AUTH_LOGIN_NOTIFIER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class LoginGuiObject;

HWND LoginNotifier_CreateWindow(UINT styleEx, UINT style, INT x, INT y, INT cx, INT cy, HWND hParent, INT controlId);


#define NLNTYPE_INFORMATION	0
#define NLNTYPE_WARNING		1
#define NLNTYPE_ERROR		2
#define NLNTYPE_QUESTION	3

#define NLNM_FIRST				(WM_USER + 10)

#define NLNM_NOTIFY				(NLNM_FIRST + 0) // wParam = (WPARAM)(INT)notificationType, lParam = (LPARAM)(LPCWSTR)notificationText; Return - TRUE on success;	notificationText can be MAKEINTRESOURCE(authError)
#define LoginNotifier_Notify(/*HWND*/ __hwnd, /*INT*/ __notificationType, /*LPCWSTR*/ __notificationText)\
		(SNDMSG((__hwnd), NLNM_NOTIFY, (WPARAM)(__notificationType), (LPARAM)(__notificationText)))

#define NLNM_GETIDEALHEIGHT		(NLNM_FIRST + 1) // wParam - not used, lParam - not used; Return - ideal height
#define LoginNotifier_GetIdealHeight(/*HWND*/ __hwnd)\
		((INT)SNDMSG((__hwnd), NLNM_GETIDEALHEIGHT, 0, 0L))

#define NLNM_SETBKCOLOR			(NLNM_FIRST + 2) // wParam - not used, lParam = (LPARAM)(COLORREF)rgb; Return ignored
#define LoginNotifier_SetBkColor(/*HWND*/ __hwnd, /*COLORREF*/ __rgb)\
		(SNDMSG((__hwnd), NLNM_SETBKCOLOR, 0, (LPARAM)(__rgb)))

#define NLNM_GETBKCOLOR			(NLNM_FIRST + 3) // wParam - not used, lParam - not used; Return back color
#define LoginNotifier_GetBkColor(/*HWND*/ __hwnd)\
		((COLORREF)SNDMSG((__hwnd), NLNM_GETBKCOLOR, 0, 0L))

#define NLNM_SETTEXTCOLOR			(NLNM_FIRST + 4) // wParam - not used, lParam = (LPARAM)(COLORREF)rgb; Return ignored
#define LoginNotifier_SetTextColor(/*HWND*/ __hwnd, /*COLORREF*/ __rgb)\
		(SNDMSG((__hwnd), NLNM_SETTEXTCOLOR, 0, (LPARAM)(__rgb)))

#define NLNM_GETTEXTCOLOR			(NLNM_FIRST + 5) // wParam - not used, lParam - not used; Return back color
#define LoginNotifier_GetTextColor(/*HWND*/ __hwnd)\
		((COLORREF)SNDMSG((__hwnd), NLNM_GETTEXTCOLOR, 0, 0L))

#define NLNM_PLAYBEEP			(NLNM_FIRST + 6) // wParam - not used, lParam - not used; Return ignored
#define LoginNotifier_PlayBeep(/*HWND*/ __hwnd)\
		(SNDMSG((__hwnd), NLNM_PLAYBEEP, 0, 0L))

#define NLNM_GETIDEALSIZE		(NLNM_FIRST + 7) // wParam - not used, lParam - (LPARAM)(SIZE*)sizeOut; Return TRUE on success
#define LoginNotifier_GetIdealSize(/*HWND*/ __hwnd, /*SIZE* */ __sizeOut)\
		((BOOL)SNDMSG((__hwnd), NLNM_GETIDEALSIZE, 0, (LPARAM)(__sizeOut)))

#endif //NULLSOFT_AUTH_LOGIN_NOTIFIER_HEADER