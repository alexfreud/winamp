#include <precomp.h>
#ifdef WIN32
#include <windows.h>
#endif

#include <bfc/assert.h>
#include <api/wndmgr/resize.h>
#include <api/wnd/wndtrack.h>
#include <api/wnd/basewnd.h>
#include <api/wndmgr/layout.h>

#define tag L"wa3_resizerclass"

extern HINSTANCE hInstance;


//----------------------------------------------------------------------
resizeClass::resizeClass(ifc_window *wnd, int minx, int miny, int maxx, int maxy, int sugx, int sugy)
{
	screenWidth = Wasabi::Std::getScreenWidth();
	screenHeight = Wasabi::Std::getScreenHeight();

#if defined (_WIN32) || defined(_WIN64)
	WNDCLASSW wc;
	if (!GetClassInfoW(hInstance, tag, &wc))
	{
		MEMSET(&wc, 0, sizeof(wc));
		wc.lpfnWndProc = resizeWndProc;
		wc.hInstance = hInstance;					// hInstance of DLL
		wc.lpszClassName = tag;			// our window class name
		wc.style = 0;

		int _r = RegisterClassW(&wc);
		ASSERTPR(_r, "cannot create resizer wndclass");
	}

	hWnd = CreateWindowExW(0, tag, L"", 0, 0, 0, 1, 1, NULL, NULL, hInstance, NULL);

	ASSERT(hWnd);

	SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)this);

	if (minx > maxx && maxx != -1) minx = maxx;
	if (miny > maxy && maxy != -1) miny = maxy;
#endif

	minWinWidth = minx;
	minWinHeight = miny;
	maxWinWidth = maxx;
	maxWinHeight = maxy;
	sugWinWidth = sugx;
	sugWinHeight = sugy;

	if (wnd->getInterface(layoutGuid))
	{
		static_cast<Layout *>(wnd)->getSnapAdjust(&snapAdjust);
		if (minWinWidth != -1) minWinWidth -= snapAdjust.left + snapAdjust.right;
		if (minWinHeight != -1) minWinHeight -= snapAdjust.bottom + snapAdjust.top;
		if (maxWinWidth != -1) maxWinWidth -= snapAdjust.left + snapAdjust.right;
		if (maxWinHeight != -1) maxWinHeight -= snapAdjust.bottom + snapAdjust.top;
		if (sugWinWidth != -1) sugWinWidth -= snapAdjust.left + snapAdjust.right;
		if (sugWinHeight != -1) sugWinHeight -= snapAdjust.bottom + snapAdjust.top;
	}

	dc = NULL;
#ifdef WIN32
	oldB = NULL;
	brush = NULL;
	oldP = NULL;
	pen = NULL;
#endif
}


//----------------------------------------------------------------------
resizeClass::~resizeClass()
{
#ifdef WIN32
	if (dc)
	{
		SelectObject(dc, oldB);
		SelectObject(dc, oldP);
		SetROP2(dc, mix);
		DeleteObject(pen);
		DeleteObject(brush);
		ReleaseDC(NULL, dc);
	}

	if (IsWindow(hWnd))
		DestroyWindow(hWnd);
	//BU win98 sucks  UnregisterClass(tag, Main::gethInstance());
#else
	if (dc)
	{
		XFreeGC(Linux::getDisplay(), dc->gc);
		FREE(dc);
	}
#endif
}

//----------------------------------------------------------------------
BOOL CALLBACK invalidateChildren(HWND hwnd, LPARAM lParam)
{
	InvalidateRect(hwnd, NULL, FALSE);
	return TRUE;
}

//----------------------------------------------------------------------
BOOL CALLBACK invalidateAll(HWND hwnd, LPARAM lParam)
{
	EnumChildWindows(hwnd, invalidateChildren, 0);
	InvalidateRect(hwnd, NULL, FALSE);
	return TRUE;
}

//----------------------------------------------------------------------
int resizeClass::resizeWindow(ifc_window *wnd, int way)
{

	//  paintHookStart();

	wnd->getWindowRect(&originalRect);

	snapAdjust.left = 0;
	snapAdjust.top = 0;
	snapAdjust.right = 0;
	snapAdjust.bottom = 0;

	if (wnd->getInterface(layoutGuid))
	{
		static_cast<Layout *>(wnd)->getSnapAdjust(&snapAdjust);
		if (wnd->getRenderRatio() != 1.0)
		{
			double rr = wnd->getRenderRatio();
			snapAdjust.left = (int)((double)(snapAdjust.left) * rr);
			snapAdjust.top = (int)((double)(snapAdjust.top) * rr);
			snapAdjust.right = (int)((double)(snapAdjust.right) * rr);
			snapAdjust.bottom = (int)((double)(snapAdjust.bottom) * rr);
		}
		originalRect.left += snapAdjust.left;
		originalRect.top += snapAdjust.top;
		originalRect.right -= snapAdjust.right;
		originalRect.bottom -= snapAdjust.bottom;
	}

	curRect = originalRect;
	resizeWay = way;
	resizedWindow = wnd->gethWnd();
	resizedWindowR = wnd;

	POINT pt;
	Wasabi::Std::getMousePos(&pt);
	cX = pt.x;
	cY = pt.y;

#ifdef WIN32
	SetCapture(hWnd);
#endif
	SetTimer(hWnd, 1, 100, NULL); // this timer will make sure that we never get interlock

	drawFrame();

	MSG msg;
	cancelit = 1;
	while (GetCapture() == hWnd && GetMessage( &msg, hWnd, 0, 0 ))
	{
		TranslateMessage( &msg );
#ifdef LINUX
		if ( msg.message == WM_LBUTTONUP || msg.message == WM_MOUSEMOVE )
			wndProc( msg.hwnd, msg.message, msg.wParam, msg.lParam );
		else
#endif
			DispatchMessage( &msg );
	}

	drawFrame();

	//  paintHookStop();

	if (GetCapture() == hWnd) ReleaseCapture();
	KillTimer(hWnd, 1);

	if (!cancelit)
	{
		curRect.left -= snapAdjust.left;
		curRect.top -= snapAdjust.top;
		curRect.right += snapAdjust.right;
		curRect.bottom += snapAdjust.bottom;
	}
	else
	{
		curRect = originalRect;
		// evil, but less evil than InvalidateRect(NULL, NULL, FALSE);
		EnumWindows(invalidateAll, 0);
	}

	return !cancelit;
}


//----------------------------------------------------------------------
RECT resizeClass::getRect(void)
{
	return curRect;
}

//#define nifty

//----------------------------------------------------------------------
void resizeClass::drawFrame(void)
{
	RECT outRect;

#ifdef WIN32
	if (!dc)
	{
		dc = GetDC(NULL);
		brush = CreateSolidBrush(0xFFFFFF);
		pen = CreatePen(PS_SOLID, 0, 0xFFFFFF);
		oldB = (HBRUSH)SelectObject(dc, brush);
		oldP = (HPEN)SelectObject(dc, pen);
		mix = SetROP2(dc, R2_XORPEN);
	}

	outRect = curRect;

#ifndef nifty

	DrawFocusRect(dc, &outRect);

#else

	RECT inRect;

	inRect = outRect;
	inRect.left += 10;
	inRect.right -= 10;
	inRect.top += 24;
	inRect.bottom -= 10;

	MoveToEx(dc, inRect.left, inRect.top, NULL);
	LineTo(dc, inRect.right, inRect.top);
	LineTo(dc, inRect.right, inRect.bottom);
	LineTo(dc, inRect.left, inRect.bottom);
	LineTo(dc, inRect.left, inRect.top);
	MoveToEx(dc, outRect.left, 0, NULL);
	LineTo(dc, outRect.left, screenHeight);
	MoveToEx(dc, outRect.right, 0, NULL);
	LineTo(dc, outRect.right, screenHeight);
	MoveToEx(dc, 0, outRect.top, NULL);
	LineTo(dc, screenWidth, outRect.top);
	MoveToEx(dc, 0, outRect.bottom, NULL);
	LineTo(dc, screenWidth, outRect.bottom);

#endif
#endif//WIN32
#ifdef LINUX
	outRect = curRect;

	if ( ! dc )
	{
		dc = (HDC)MALLOC( sizeof( hdc_typ ) );
		XGCValues gcv;
		gcv.function = GXxor;
		gcv.subwindow_mode = IncludeInferiors;
		gcv.foreground = 0xffffff;
		gcv.line_style = LineOnOffDash;
		gcv.dashes = 1;
		dc->gc = XCreateGC( Linux::getDisplay(), Linux::RootWin(), GCFunction | GCForeground | GCSubwindowMode, &gcv );
	}

	XDrawRectangle( Linux::getDisplay(), Linux::RootWin(), dc->gc,
	                outRect.left, outRect.top, outRect.right - outRect.left,
	                outRect.bottom - outRect.top );
#endif 
	//PORTME
}

LRESULT resizeClass::wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONUP:
		cancelit = 0;
		ReleaseCapture();
		return 0;

	case WM_MOUSEMOVE:
		{
			POINT pt;
			int iX, iY;
			Wasabi::Std::getMousePos(&pt);
			iX = pt.x - cX;
			iY = pt.y - cY;
			drawFrame();

			if (resizeWay & TOP)
				curRect.top = originalRect.top + iY;
			if (resizeWay & BOTTOM)
				curRect.bottom = originalRect.bottom + iY;
			if (resizeWay & LEFT)
				curRect.left = originalRect.left + iX;
			if (resizeWay & RIGHT)
				curRect.right = originalRect.right + iX;

			if (abs((curRect.right - curRect.left) - sugWinWidth) < 10)
				if (resizeWay & RIGHT)
					curRect.right = curRect.left + sugWinWidth;
				else if (resizeWay & LEFT)
					curRect.left = curRect.right - sugWinWidth;

			if (abs((curRect.bottom - curRect.top) - sugWinHeight) < 10)
				if (resizeWay & BOTTOM)
					curRect.bottom = curRect.top + sugWinHeight;
				else if (resizeWay & TOP)
					curRect.top = curRect.bottom - sugWinHeight;

			curRect.left -= snapAdjust.left;
			curRect.top -= snapAdjust.top;
			curRect.right += snapAdjust.right;
			curRect.bottom += snapAdjust.bottom;

			windowTracker->autoDock(resizedWindowR, &curRect, resizeWay);

			curRect.left += snapAdjust.left;
			curRect.top += snapAdjust.top;
			curRect.right -= snapAdjust.right;
			curRect.bottom -= snapAdjust.bottom;

			if ((curRect.right - curRect.left) < minWinWidth)
				if (resizeWay & RIGHT)
					curRect.right = curRect.left + minWinWidth;
				else if (resizeWay & LEFT)
					curRect.left = curRect.right - minWinWidth;

			if ((curRect.bottom - curRect.top) < minWinHeight)
				if (resizeWay & BOTTOM)
					curRect.bottom = curRect.top + minWinHeight;
				else if (resizeWay & TOP)
					curRect.top = curRect.bottom - minWinHeight;

			if (maxWinWidth != -1 && (curRect.right - curRect.left) > maxWinWidth)
				if (resizeWay & RIGHT)
					curRect.right = curRect.left + maxWinWidth;
				else if (resizeWay & LEFT)
					curRect.left = curRect.right - maxWinWidth;

			if (maxWinHeight != -1 && (curRect.bottom - curRect.top) > maxWinHeight)
				if (resizeWay & BOTTOM)
					curRect.bottom = curRect.top + maxWinHeight;
				else if (resizeWay & TOP)
					curRect.top = curRect.bottom - maxWinHeight;

			drawFrame();
		}
		return 0;
	}
#ifdef WIN32
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
#else
	return 0;
#endif
}

// Window Procedure
//
LRESULT CALLBACK resizeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef WIN32
	resizeClass *gThis = (resizeClass *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (!gThis)
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	else
		return gThis->wndProc(hwnd, uMsg, wParam, lParam);
#else
	return 0;
#endif
}

void resizeClass::setResizeCursor(int action)
{}


