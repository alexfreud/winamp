#pragma once
#include "..\..\General\gen_ml/ml_ipc_0313.h"
#include <vector>

class IconStore
{
public:
	IconStore();
	~IconStore();
	
	int GetPlaylistIcon();
	int GetVideoIcon();
	int GetDeviceIcon();
	int GetQueueIcon(int iconIndex = 0);
	int GetResourceIcon(HINSTANCE module, const wchar_t *name);
	void ReleaseResourceIcon(int iconIndex);

private:
	int RegisterResourceIcon(HINSTANCE module, const wchar_t *name);

private:
	typedef struct ResourceIcon
	{
		size_t ref;
		int index;
		wchar_t *name;
		HINSTANCE module;
	} ResourceIcon;

	int playlist_icon_index;
	int video_icon_index;
	int device_icon_index;
	int queue_icon_index[4];
	int active_queue_icon[4];

	std::vector<ResourceIcon> iconList;
};

extern IconStore icon_store;