#ifndef NULLOSFT_MEDIALIBRARY_COMMANDBAR_CONTROL_HEADER
#define NULLOSFT_MEDIALIBRARY_COMMANDBAR_CONTROL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CBM_FIRST
#define CBM_FIRST				(WM_APP + 100)
#endif

#define CBM_GETBESTHEIGHT		(CBM_FIRST + 1)
#define CBM_GETOWNER				(CBM_FIRST + 2)
#define CBM_SETOWNER				(CBM_FIRST + 3)
#define CBM_GETDATA				(CBM_FIRST + 4)
#define CBM_SETDATA				(CBM_FIRST + 5)

#define CommandBar_GetBestHeight(/*HWND*/ __hwndCB)\
		((INT)(INT_PTR)SENDMSG((__hwndCB), CBM_GETBESTHEIGHT, 0, 0L))

#define CommandBar_GetOwner(/*HWND*/ __hwndCB)\
	((HWND)SENDMSG((__hwndCB), CBM_GETOWNER, 0, 0L))

#define CommandBar_SetOwner(/*HWND*/ __hwndCB, /*HWND*/ __hwndNewOwner)\
	((BOOL)SENDMSG((__hwndCB), CBM_SETOWNER, 0, (LPARAM)(__hwndNewOwner)))

#define CommandBar_GetData(/*HWND*/ __hwndCB)\
	((HWND)SENDMSG((__hwndCB), CBM_GETDATA, 0, 0L))

#define CommandBar_SetData(/*HWND*/ __hwndCB, /*ULONG_PTR*/ __userData)\
	((BOOL)SENDMSG((__hwndCB), CBM_SETDATA, 0, (LPARAM)(__userData)))




#ifdef __cplusplus
}
#endif



#endif // NULLOSFT_MEDIALIBRARY_COMMANDBAR_CONTROL_HEADER