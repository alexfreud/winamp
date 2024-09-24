#ifndef NULLSOFT_VIDEOOUTPUTCHILDH
#define NULLSOFT_VIDEOOUTPUTCHILDH


class SubsItem;
#include <windows.h>
#include "VideoAspectAdjuster.h"

/*
 VideoRenderer is the base classes for the various video rendering classes
*/

class VideoRenderer 
{
public:
	virtual ~VideoRenderer() 
	{
	}
	virtual int create(HWND parent, VideoAspectAdjuster *_adjuster, int w, int h, unsigned int type, int flipit, double aspectratio) = 0; //return 1 if ok
	virtual int needChange() = 0; //return 1 if need to renegociate video output
	virtual int onPaint(HWND hwnd) { return 0; } //return 1 if override
	virtual void displayFrame(const char *buf, int size, int time) = 0;
	virtual void close()= 0; // hides any output of the video
	virtual void timerCallback() { }
	virtual void setPalette(RGBQUAD *pal) { }
	virtual void drawSubtitle(SubsItem *item) { }
	virtual void resetSubtitle() { }
	virtual void setVFlip(int on) { }	
	virtual void Refresh()=0;
	
};



#endif
