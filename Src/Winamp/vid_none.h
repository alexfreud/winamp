#ifndef _VIDEO_NONE_H
#define _VIDEO_NONE_H

#include "wa_ipc.h"
#include "VideoOutputChild.h"

class NoneVideoOutput : public VideoRenderer
{
public:
	int create(HWND parent, VideoAspectAdjuster *_adjuster, int w, int h, unsigned int type, int flipit, double aspectratio)
	{
		if (type == VIDEO_MAKETYPE('N', 'O', 'N', 'E')) return 1;
		return 0;
	}
	void close() { }
	int needChange() { return 0; }
	void displayFrame(const char *buf, int size, int time) { }
	void Refresh() {}
};
extern NoneVideoOutput noneVideo;
#endif
