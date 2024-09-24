#ifndef __RESIZE_H
#define __RESIZE_H
#ifdef _WIN32
LRESULT CALLBACK resizeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

class resizeClass
{
#ifdef _WIN32
	friend LRESULT CALLBACK resizeWndProc(HWND wnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // heh, i really need this one ;)
#endif
private:
	void drawFrame(void);
	void setResizeCursor(int action);
	int orientation(int a, int action);
	OSWINDOWHANDLE hWnd;
	OSWINDOWHANDLE resizedWindow;
	ifc_window *resizedWindowR;
	int resizeWay;
	RECT curRect;
	RECT originalRect;
	int cX, cY;
	int minWinWidth, minWinHeight;
	int maxWinWidth, maxWinHeight;
	int sugWinWidth, sugWinHeight;
	int screenHeight, screenWidth;
	bool cancelit;
	RECT snapAdjust;

#ifdef WIN32
	HBRUSH oldB, brush;
	HPEN oldP, pen;
#endif
	HDC dc;
	int mix;

public:
	resizeClass(ifc_window *wnd, int minx, int miny, int maxx, int maxy, int sugx, int sugy);
	~resizeClass();
	int resizeWindow(ifc_window *wnd, int way);
#ifdef _WIN32
	LRESULT wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
	RECT getRect(void);
};

#endif

