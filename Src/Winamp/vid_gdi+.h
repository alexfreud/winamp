#ifndef _VID_GDIPLUS_H_
#define _VID_GDIPLUS_H_

#include <windows.h>
#include <gdiplus.h>
#include "VideoOutputChild.h"

using namespace Gdiplus;

class GDIPVideoOutput : public VideoRenderer {
public:
	GDIPVideoOutput();
	virtual ~GDIPVideoOutput();
	virtual int create(HWND parent, VideoAspectAdjuster *_adjuster, int w, int h, unsigned int type, int flipit, double aspectratio); //return 1 if ok
	virtual int needChange(); //return 1 if need to renegociate video output
	virtual int onPaint(HWND hwnd) { return 0; } //return 1 if override
	virtual void displayFrame(const char *buf, int size, int time);
	virtual void close(); // hides any output of the video
	virtual void timerCallback();
	virtual void drawSubtitle(SubsItem *item) { subs=item; }
	virtual void resetSubtitle() { subs=NULL; }
	virtual void setVFlip(int on) { flip=on; }	
	virtual void Refresh();

protected:
	bool FillFrame(Bitmap *frame, void *buf);

	unsigned int type, w, h, flip, winw, winh;
	Graphics * graphics; // on screen canvas
	Graphics * graphicsback; // off screen canvas
	Bitmap * frame;
	HWND parent;
	int needschange;
	VideoAspectAdjuster *adjuster;
	RECT lastrect;
	HDC backdc;
	void SetupGraphics();
	ULONG_PTR gdiplusToken;
	SubsItem *subs;
};

extern GDIPVideoOutput gdiplusVideo;

#endif // _VID_GDIPLUS_H_
