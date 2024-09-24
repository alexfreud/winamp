#ifndef WINAMP_HTTP_GRAB_TEXT_HEADER
#define WINAMP_HTTP_GRAB_TEXT_HEADER

#include <windows.h>

#define HTTPGRAB_USEWINDOWTEXT		0x0000
#define HTTPGRAB_USESTATUSTEXT		0x0001

HWND BeginGrabHTTPText(HWND hwndFwd, UINT flags, HWND *phwndTarget); // returns hwnd that you need to supply as parent of http funciton
void EndGrabHTTPText(HWND hwndHost); 


#endif //WINAMP_HTTP_GRAB_TEXT_HEADER