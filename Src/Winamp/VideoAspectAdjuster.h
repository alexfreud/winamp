#ifndef NULLSOFT_VIDEOASPECTADJUSTERH
#define NULLSOFT_VIDEOASPECTADJUSTERH

#include <windows.h>

class VideoAspectAdjuster
{
public:
	virtual void adjustAspect(RECT &rd) = 0;
	virtual void DrawLogo(HDC out, RECT *canvas_size)=0;
};
#endif