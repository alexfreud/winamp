#ifndef WINAMP_POSTSETUP_HEADER
#define WINAMP_POSTSETUP_HEADER

#include <windows.h>

HWND CreateStatusWnd(HWND hwndParent, INT x, INT y, INT cx, INT cy);
void SetStatusText();
BOOL StartWinamp(BOOL bWaitShow, HWND *phwndWA, LPCSTR pszParam);

#endif //WINAMP_POSTSETUP_HEADER