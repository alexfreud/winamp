#include "main.h"
#include "navigationIcons.h"

#include <vector>

typedef struct IconCacheRecord
{
	size_t ref;
	int index;
	DeviceImage *image;
} IconCacheRecord;

typedef std::vector<IconCacheRecord> IconCache;

static IconCache iconCache;

static BOOL
NavigationIcons_GetSize(unsigned int *width, unsigned int *height)
{
	HWND libraryWindow;
	MLIMAGELISTIMAGESIZE imageSize;

	libraryWindow = Plugin_GetLibraryWindow();

	imageSize.hmlil =  MLNavCtrl_GetImageList(libraryWindow);
	if (NULL == imageSize.hmlil)
		return FALSE;
	
	if (FALSE == MLImageList_GetImageSize(libraryWindow, &imageSize))
		return FALSE;

	*width = imageSize.cx;
	*height = imageSize.cy;
	
	return TRUE;
}

static DeviceImage*
NavigationIcons_GetDeviceImage(ifc_device *device, unsigned int width, unsigned int height)
{
	wchar_t path[MAX_PATH * 2] = {0};

	if (NULL == device)
		return NULL;

	if (FAILED(device->GetIcon(path, ARRAYSIZE(path), width, height)))
		return NULL;

	return DeviceImageCache_GetImage(Plugin_GetImageCache(), 
					path, width, height, NULL, NULL);
}

static DeviceImage*
NavigationIcons_GetDeviceTypeImage(ifc_device *device, unsigned int width, unsigned int height)
{
	ifc_devicetype *type;
	DeviceImage *image;
	wchar_t path[MAX_PATH * 2] = {0};
	
	if (NULL == device || 
		NULL == WASABI_API_DEVICES ||
		S_OK != WASABI_API_DEVICES->TypeFind(device->GetName(), &type))
	{
		return NULL;
	}
	
	if (SUCCEEDED(type->GetIcon(path, ARRAYSIZE(path), width, height)))
	{

		image = DeviceImageCache_GetImage(Plugin_GetImageCache(), 
						path, width, height, NULL, NULL);
	}
	else
		image = NULL;

	type->Release();

	return image;
}

static DeviceImage*
NavigationIcons_GetDefaultImage(ifc_device *device, unsigned int width, unsigned int height)
{
	const wchar_t *path;

	path = Plugin_GetDefaultDeviceImage(width, height);
	if (NULL == path)
		return NULL;
	
	return DeviceImageCache_GetImage(Plugin_GetImageCache(), path, width, height, NULL, NULL);
}

static int
NavigationIcons_RegisterDeviceIcon(DeviceImage *image, int iconIndex)
{
	MLIMAGESOURCE imageSource;
	MLIMAGELISTITEM listItem;
	HWND libraryWindow;
	HBITMAP bitmap;
		
	if (NULL == image)
		return -1;

	libraryWindow = Plugin_GetLibraryWindow();
	if (NULL == libraryWindow)
		return -1;


	bitmap = DeviceImage_GetBitmap(image, 
				DeviceImage_ExactSize | DeviceImage_AlignHCenter | DeviceImage_AlignVCenter);

	if (NULL == bitmap)
		return -1;

	
	imageSource.cbSize = sizeof(imageSource);
	imageSource.lpszName = (LPCWSTR)bitmap;
	imageSource.type = SRC_TYPE_HBITMAP;
	imageSource.bpp = 32;
	imageSource.flags = 0;
	imageSource.hInst = NULL;

	listItem.cbSize = sizeof(listItem);
	listItem.hmlil = MLNavCtrl_GetImageList(libraryWindow);
	listItem.filterUID = MLIF_FILTER3_UID;
	listItem.pmlImgSource = &imageSource;
	listItem.mlilIndex = iconIndex;

	if (NULL == listItem.hmlil)
		return -1;

	if (listItem.mlilIndex >= 0)
	{
		if (FALSE == MLImageList_Replace(libraryWindow, &listItem))
			return -1;

		return listItem.mlilIndex;
	}

	return MLImageList_Add(libraryWindow, &listItem);
}


static IconCacheRecord *
NavigationIcons_FindCacheRecord(DeviceImage *image)
{
	size_t index;
	IconCacheRecord *record;

	if (NULL == image)
		return NULL;

	index = iconCache.size();
	while(index--)
	{
		record = &iconCache[index];
		if (record->image == image)
			return record;
	}

	return NULL;
}

static IconCacheRecord * 
NavigationIcons_FindAvailableCacheRecord()
{
	size_t index;
	IconCacheRecord *record;

	index = iconCache.size();
	while(index--)
	{
		record = &iconCache[index];
		if (0 == record->ref)
			return record;
	}

	return NULL;
}

static IconCacheRecord *
NavigationIcons_FindCacheRecordByIndex(int iconIndex)
{
	size_t index;
	IconCacheRecord *record;

	if (iconIndex < 0)
		return NULL;

	index = iconCache.size();
	while(index--)
	{
		record = &iconCache[index];
		if (record->index == iconIndex)
			return record;
	}

	return NULL;
}

static IconCacheRecord *
NavigationIcons_CreateCacheRecord(DeviceImage *image, int iconIndex)
{
	IconCacheRecord record;

	if (NULL == image || -1 == iconIndex)
		return NULL;

	record.ref = 1;
	record.index = iconIndex;
	record.image = image;

	DeviceImage_AddRef(image);

	iconCache.push_back(record);
	return &iconCache.back();
}

static HBITMAP
NavigationIcons_GetDeviceImageBitmap(DeviceImage *image)
{
	return DeviceImage_GetBitmap(image, 
								DeviceImage_ExactSize | 
								DeviceImage_AlignHCenter | 
								DeviceImage_AlignVCenter);
}

int
NavigationIcons_GetDeviceIconIndex(ifc_device *device)
{
	DeviceImage *image;
	unsigned int width, height;
	IconCacheRecord *record;
	int iconIndex;
	size_t attempt;

	if (FALSE == NavigationIcons_GetSize(&width, &height))
		return -1;

	for(attempt = 0; attempt < 3; attempt++)
	{
		switch(attempt)
		{
			case 0:
				image = NavigationIcons_GetDeviceImage(device, width, height);
				break;
			case 1:
				image = NavigationIcons_GetDeviceTypeImage(device, width, height);
				break;
			case 2:
				image = NavigationIcons_GetDefaultImage(device, width, height);
				break;
		}
	
		record = (NULL != image) ? 
					NavigationIcons_FindCacheRecord(image) : 
					NULL;

		if (NULL == record && 
			NULL == NavigationIcons_GetDeviceImageBitmap(image))
		{
				continue;
		}

		break;
	}

	if (NULL != record)
	{
		record->ref++;
		iconIndex = record->index;
	}
	else
	{
		record = NavigationIcons_FindAvailableCacheRecord();
		if (NULL != record)
		{
			iconIndex = NavigationIcons_RegisterDeviceIcon(image, record->index);
			if (-1 != iconIndex)
			{
				record->ref++;
				record->image = image;
			}
		}
		else
		{
			iconIndex = NavigationIcons_RegisterDeviceIcon(image, -1);
			if (-1 != iconIndex)
			{
				IconCacheRecord newRecord;
				newRecord.ref = 1;
				newRecord.image = image;
				newRecord.index = iconIndex;
				iconCache.push_back(newRecord);
			}
		}
	}

	if (-1 == iconIndex)
		DeviceImage_Release(image);

	return iconIndex;
}

BOOL
NavigationIcons_ReleaseIconIndex(int iconIndex)
{
	IconCacheRecord *record;

	record = NavigationIcons_FindCacheRecordByIndex(iconIndex);
	if (NULL == record)
		return FALSE;

	if (0 == record->ref)
		return FALSE;
	
	record->ref--;
	DeviceImage_Release(record->image);
	if (0 == record->ref)
		record->image = NULL;
	
	return TRUE;
}

void
NavigationIcons_ClearCache()
{
	size_t index;
	IconCacheRecord *record;

	index = iconCache.size();
	while(index--)
	{
		record = &iconCache[index];
		if (NULL != record->image)
		{
			while(record->ref--)
				DeviceImage_Release(record->image);
			record->image = NULL;
		}
		record->ref = 0;
	}
}