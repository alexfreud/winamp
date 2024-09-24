#ifndef NULLOSFT_MEDIALIBRARY_REFLECTED_MESSAGES_HEADER
#define NULLOSFT_MEDIALIBRARY_REFLECTED_MESSAGES_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

// reflected messages will pass this as lParam
typedef struct _REFLECTPARAM
{
	LRESULT			result;			// return result here. If refleting window is dialog it is responsible to set result using SetWindowlongPtr.
	LPARAM			lParam;			// orginal lParam
	HWND				hwndFrom;		// reflecting window
}REFLECTPARAM, *PREFLECTPARAM;



// reflected messages
// you need to return TRUE if you procesed a message otherwise FALSE
#define REFLECT_BASE				(WM_APP + 0x3000)

#define WM_SUPPORTREFLECT			(REFLECT_BASE + 0x0000) // wParam = (WPARM)(UINT)testMessageCode. Return TRUE if you suport message reflecting

#define REFLECTED_DRAWITEM			(REFLECT_BASE + WM_DRAWITEM)
#define REFLECTED_CTLCOLORBTN		(REFLECT_BASE + WM_CTLCOLORBTN)
#define REFLECTED_CTLCOLOREDIT		(REFLECT_BASE + WM_CTLCOLOREDIT)
#define REFLECTED_CTLCOLORLISTBOX	(REFLECT_BASE + WM_CTLCOLORLISTBOX)
#define REFLECTED_CTLCOLORSCROLLBAR	(REFLECT_BASE + WM_CTLCOLORSCROLLBAR)
#define REFLECTED_CTLCOLORSTATIC	(REFLECT_BASE + WM_CTLCOLORSTATIC)
#define REFLECTED_NOTIFY				(REFLECT_BASE + WM_NOTIFY)
#define REFLECTED_COMMAND			(REFLECT_BASE + WM_COMMAND)
#define REFLECTED_MEASUREITEM		(REFLECT_BASE + WM_MEASUREITEM)


#ifdef __cplusplus
#define REFLECTMESSAGE(hwnd, uMsg, wParam, lParam) (BOOL)::SendMessage((hwnd), (REFLECT_BASE + (uMsg)), (wParam), (lParam))
#else 
#define REFLECTMESSAGE(hwnd, uMsg, wParam, lParam) (BOOL)SendMessage((hwnd), (REFLECT_BASE + (uMsg)), (wParam), (lParam))
#endif

BOOL CanReflect(UINT uMsg);
BOOL ReflectMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bDialog, LRESULT *pResult);

HRESULT InstallReflector(HWND hwnd); // this is installs simple window hook that allows reflection code to run.
									// returns , S_OK - hook installed, S_FALSE in case hook already installed, E_XXX - something bad 
BOOL RemoveReflector(HWND hwnd);		// returns TRUE if window was reflecting 

#endif // NULLOSFT_MEDIALIBRARY_REFLECTED_MESSAGES_HEADER
