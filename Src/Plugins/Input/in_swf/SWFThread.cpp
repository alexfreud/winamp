#include "main.h"
#include "api.h"
#include "SWFContainer.h"

SWFContainer *activeContainer=0;
WNDPROC oldVidProc=0;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_SIZE:
		activeContainer->SyncSizeToWindow(hWnd);	
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	}
	return CallWindowProc(oldVidProc, hWnd, message, wParam, lParam);
}

