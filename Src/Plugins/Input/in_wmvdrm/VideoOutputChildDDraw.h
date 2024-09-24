#ifndef NULLSOFT_VIDEOOUTPUTCHILDDDRAWH
#define NULLSOFT_VIDEOOUTPUTCHILDDDRAWH
#include "../Winamp/VideoOutputChild.h"

class VideoOutputChildDDraw : public VideoRenderer
{
public:
	VideoOutputChildDDraw() : m_mon_x(0), m_mon_y(0), foundGUID(false), parent(0), adjuster(0) {}
	VideoAspectAdjuster *adjuster;
	void update_monitor_coords();
	int m_mon_x, m_mon_y;
	bool foundGUID;
	GUID m_devguid;
	HWND parent;
};

#endif