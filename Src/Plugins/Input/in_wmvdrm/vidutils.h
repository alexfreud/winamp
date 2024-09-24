#ifndef NULLSOFT_IN_WMVDRM_VID_UTILS_H
#define NULLSOFT_IN_WMVDRM_VID_UTILS_H

#include "../Agave/Config/ifc_configgroup.h"

class VideoConfig
{
public:
	VideoConfig();
	bool yv12();
	bool overlays();
	bool vsync();
	bool ddraw();

private:
	void GetGroup();

	ifc_configgroup *group;
	ifc_configitem *itemYV12, *itemOverlay, *itemVsync, *itemDDraw;
};

extern VideoConfig config_video;

#endif