#pragma once
#include <windows.h>

class WADrawDC
{
public:
	WADrawDC(HWND _hwnd);
	WADrawDC(HDC _hdc, HWND _hwnd);
	~WADrawDC();
	operator HDC();
private:
	HDC hdc;
	HWND hwnd;
};