#ifndef __OSWND_H
#define __OSWND_H

#include <api/wnd/wndclass/guiobjwnd.h>

#define OSWND_PARENT GuiObjectWnd

class OSWnd : public OSWND_PARENT
{
public:
	virtual int onInit();
	virtual void onSetVisible(int show);
	virtual int onResize();
	virtual int handleRatio() { return 0; }

	virtual HWND getOSHandle() = 0;
};

#endif
