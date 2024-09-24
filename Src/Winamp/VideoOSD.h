#ifndef NULLSOFT_VIDEOOSDH
#define NULLSOFT_VIDEOOSDH

/* Video On Screen Display */
#include <windows.h>

#define NUM_WIDGETS 11

class IVideoOSD
{
public:
	IVideoOSD();
	~IVideoOSD();
	void SetParent(HWND _parent) 
	{
		parent=_parent;
	}
	bool Showing(); 
	bool Ready();
	void Show();
	void Hide();
	void Draw();
	int GetBarHeight();
	bool Mouse(int x, int y, WPARAM wParam, bool moving); // return true if we need to close because of this

	int ctrlrects_ready;

	virtual bool MouseDown(int x, int y, WPARAM wParam) {return false;};
	virtual bool MouseMove(int x, int y, WPARAM wParam) {return false;};
	virtual bool MouseUp(int x, int y, WPARAM wParam) {return false;};
	
protected:
	HWND parent;
private:
	bool CloseHitTest(int x, int y);
	void HitTest(int x, int y, int dragging);
	int osdLastClickItem;

	int osdMemBMW; // width of memory bitmap
	int osdMemBMH; // height of memory bitmap
	int osdLastMouseX;  // for WM_MOUSEMOVE thresholding, so osd isn't spastic
	int osdLastMouseY;  // for WM_MOUSEMOVE thresholding, so osd isn't spastic
	RECT ctrlrect[NUM_WIDGETS]; // relative to [i.e. (0,0) is] upper left corner of the black strip @ the bottom
	RECT ctrlrect_all;          // relative to [i.e. (0,0) is] upper left corner of the black strip @ the bottom

	bool show_osd;
	
	static void CALLBACK TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	int ignore_mousemove_count;
	int last_close_height, last_close_width;
};

#endif