#include "main.h"
#include "images.h"
#include "resource.h"
#include <strsafe.h>

static int small_images[] = { IDB_GENERIC_16, IDB_EVO_16, IDB_INCREDIBLE_16, IDB_NEXUSONE_16, IDB_DROID_16 };
static int large_images[] = { IDB_GENERIC_160, IDB_EVO_160, IDB_INCREDIBLE_160, IDB_NEXUSONE_160, IDB_DROID_160 };
int GetImageIndex(const wchar_t *manufacturer, const wchar_t *model)
{
	if (!wcscmp(manufacturer, L"HTC"))
	{
		if (!wcscmp(model, L"PC36100")) // evo
		{
			return 1;
		}
		else if (!wcscmp(model, L"ADR6300")) // incredible
		{
			return 2;
		}
		else if (!wcscmp(model, L"Nexus One"))
		{
			return 3;
		}
	}
	else if (!wcscmp(manufacturer, L"motorola"))
	{
		if (!wcscmp(model, L"DROID2"))
		{
			return 4;
		}
	}

	return 0;
}

void GetImagePath(int image_index, int width, int height, wchar_t *path, size_t path_cch)
{
	if (image_index < 0)
	{
		path[0]=0;
		return;
	}

	if (width <= 16 && height <= 16)
	{
		if (image_index >= sizeof(small_images)/sizeof(small_images[0]))
		{
			path[0]=0;
			return;
		}
		int resource = small_images[image_index];
		FormatResProtocol(MAKEINTRESOURCE(resource), L"PNG", path, path_cch);
	}
	else
	{
		if (image_index >= sizeof(large_images)/sizeof(large_images[0]))
		{
			path[0]=0;
			return;
		}
		int resource = large_images[image_index];
		FormatResProtocol(MAKEINTRESOURCE(resource), L"PNG", path, path_cch);
	}
}

int GetSmallImageID(int image_index)
{
	if (image_index < 0 || image_index >= sizeof(small_images)/sizeof(small_images[0]))
		return IDB_GENERIC_16;

	return small_images[image_index];
}