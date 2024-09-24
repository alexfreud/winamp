#ifndef _VIDEO_DDRAW_H
#define _VIDEO_DDRAW_H

#include <ddraw.h>
#include "VideoOutputChildDDraw.h"

class SubsItem;

class DDrawVideoOutput : public VideoOutputChildDDraw
{
public:
	DDrawVideoOutput();
	virtual ~DDrawVideoOutput();
	int create(HWND parent, VideoAspectAdjuster *_adjuster, int w, int h, unsigned int ptype, int flipit, double aspectratio);
	int needChange() { return needchange; }
	int onPaint(HWND hwnd);
	void displayFrame(const char *buf, int size, int time);
	void timerCallback();
	void setPalette(RGBQUAD *pal) { m_palette = pal; }
	void drawSubtitle(SubsItem *item);
	void resetSubtitle();
	void setVFlip(int on) { flip = on; }
	void Refresh();
	void close();
private:
	bool LockSurface(DDSURFACEDESC *dd);
	bool Paint(HWND hwnd);

	int width, height, flip;
	int needchange;
	unsigned int type;
	LPDIRECTDRAW	lpDD;
	LPDIRECTDRAWSURFACE lpddsOverlay, lpddsPrimary, lpddsSTTemp;
	int sttmp_w, sttmp_h;
	DDCAPS capsDrv;
	unsigned int uDestSizeAlign, uSrcSizeAlign;
	DWORD dwUpdateFlags;
	RECT rs, rd;
	RECT lastresizerect;
	bool initing;
	
	LPDIRECTDRAWCLIPPER lpddsClipper;
	DDPIXELFORMAT m_ddpf;
	int m_depth;
	RGBQUAD *m_palette;
	HFONT subFont;
	RECT subRect;
	SubsItem *m_lastsubtitle;
	int m_sub_needremeasure;
	RECT winRect;
	int m_fontsize;
	RECT m_monRect;
};

extern DDrawVideoOutput ddrawVideo;
#endif
