#ifndef _VIDEO_OVERLAY_H
#define _VIDEO_OVERLAY_H

#include <ddraw.h>
#include <multimon.h>
#include "VideoOutputChildDDraw.h"

class SubsItem;

class OverlayVideoOutput : public VideoOutputChildDDraw {

public:
	OverlayVideoOutput();
	virtual ~OverlayVideoOutput();
	int create(HWND parent,VideoAspectAdjuster *_adjuster, int w, int h, unsigned int type, int flipit, double aspectratio); //return 1 if ok
	int needChange() { return needchange; }
	int onPaint(HWND hwnd);
	void displayFrame(const char *buf, int size, int time);  
	void timerCallback();
	void close();

	void drawSubtitle(SubsItem *item);
	void resetSubtitle();
	void setVFlip(int on) { flip=on; }
	void Refresh() { InvalidateRect(parent, NULL, TRUE); }
private:
	bool LockSurface(DDSURFACEDESC *dd);
	int width, height, flip;
	int m_closed;
	int needchange;
	unsigned int type;
	LPDIRECTDRAW		   lpDD;
	LPDIRECTDRAWSURFACE lpddsOverlay, lpddsPrimary;
	LPDIRECTDRAWSURFACE lpBackBuffer;

	DDCAPS          capsDrv;
	unsigned int    uDestSizeAlign, uSrcSizeAlign;
	DWORD           dwUpdateFlags;
	DDOVERLAYFX     ovfx;
	RECT rs,rd;
	RECT m_oldrd;
	RECT winRect;

	int overlay_color;
	bool initing;
	int yuy2_output, uyvy_output;

	void getRects(RECT *drs, RECT *drd, int fixmultimon=1) const;

	HFONT subFont;
	RECT subRect;
	SubsItem *curSubtitle;
	int m_fontsize;
};
extern OverlayVideoOutput overlayVideo;
#endif