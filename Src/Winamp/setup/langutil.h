#ifndef WINAMP_SETUP_LANGUTIL_HEADER
#define WINAMP_SETUP_LANGUTIL_HEADER

#include <windows.h>
#include "./loadimage.h"

INT_PTR WADialogBoxParam(LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HWND WACreateDialogParam(LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
HBITMAP WALoadImage2(LPCWSTR pszSectionName, LPCWSTR lpImageName, BOOL bPremult);


#define WADialogBox(lpTemplate, hWndParent, lpDialogFunc) \
WADialogBoxParam(lpTemplate, hWndParent, lpDialogFunc, 0L)

#define WACreateDialog(lpName, hWndParent, lpDialogFunc) \
WACreateDialogParam(lpName, hWndParent, lpDialogFunc, 0L)

#endif //WINAMP_SETUP_LANGUTIL_HEADER