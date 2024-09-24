#include <precomp.h>
#include "oswnd.h"

int OSWnd::onInit()
{
	OSWND_PARENT::onInit();
	onSetVisible(isVisible());
	return 1;
}

void OSWnd::onSetVisible(int show)
{
#ifdef WIN32
	ShowWindow(getOSHandle(), show ? SW_NORMAL : SW_HIDE);
#endif
}

int OSWnd::onResize()
{
	OSWND_PARENT::onResize();
#ifdef WIN32
	if (getOSHandle())
	{
		RECT r;
		getClientRect(&r);
		SetWindowPos(getOSHandle(), NULL, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
	}
#endif
	return 1;
}


