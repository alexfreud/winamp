#include "draw.h"
#include "WADrawDC.h"

WADrawDC::WADrawDC(HWND _hwnd)
{
	hdc = draw_GetWindowDC(_hwnd);
	hwnd=_hwnd;
}

WADrawDC::WADrawDC(HDC _hdc, HWND _hwnd)
{
	if (!_hdc)
	{
		hdc = draw_GetWindowDC(_hwnd);
		hwnd=_hwnd;
	}
	else
	{
		hdc=_hdc;
		hwnd=0; // set to 0 so we know not to call draw_ReleaseDC
	}
}

WADrawDC::~WADrawDC()
{
	if (hwnd)
		draw_ReleaseDC(hwnd,hdc);
}

WADrawDC::operator HDC()
{
	return hdc;
}