#include "IconStore.h"
#include "resource1.h"
#include "..\..\General\gen_ml/ml.h"

extern winampMediaLibraryPlugin plugin;
IconStore icon_store;

static int IconStore_RegisterBitmap(HMLIMGLST imageList, int index, HINSTANCE module, const wchar_t *iconName)
{
	MLIMAGESOURCE imageSource;
	MLIMAGELISTITEM listItem;

	imageSource.cbSize = sizeof(imageSource);
	imageSource.hInst = module;
	imageSource.lpszName = iconName;
	imageSource.type = SRC_TYPE_PNG;
	imageSource.bpp = 32;
	imageSource.flags = ISF_FORCE_BPP;

	if (NULL == module && FALSE == IS_INTRESOURCE(iconName))
		imageSource.flags |= ISF_LOADFROMFILE;

	listItem.cbSize = sizeof(listItem);
	listItem.hmlil = imageList;
	listItem.filterUID = MLIF_FILTER3_UID;
	listItem.pmlImgSource = &imageSource;
	listItem.mlilIndex = index;

	if (listItem.mlilIndex >= 0)
	{
		if (FALSE == MLImageList_Replace(plugin.hwndLibraryParent, &listItem))
			return -1;
		return listItem.mlilIndex;
	}

	return MLImageList_Add(plugin.hwndLibraryParent, &listItem);
}

static BOOL IconStore_IsResourceNameEqual(const wchar_t *name1, const wchar_t *name2)
{
	if (FALSE != IS_INTRESOURCE(name1) || FALSE != IS_INTRESOURCE(name2))
	{
		return (name1 == name2);
	}

	return (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 
										NORM_IGNORECASE, name1, -1, name2, -1));
}

static void IconStore_FreeResourceName(wchar_t *name)
{
	if (FALSE == IS_INTRESOURCE(name))
		free(name);
}

IconStore::IconStore()
{
	playlist_icon_index = -1;
	video_icon_index = -1;
	device_icon_index = -1;
	for (int i = 0; i < 4; i++)
	{
		active_queue_icon[i] = queue_icon_index[i] = 0;
	}
}

IconStore::~IconStore()
{
	size_t index;
	
	index = iconList.size();
	while(index--)
	{
		IconStore_FreeResourceName(iconList[index].name);
	}
}

int IconStore::GetPlaylistIcon()
{
	if (-1 == playlist_icon_index)
	{
		playlist_icon_index = IconStore_RegisterBitmap(MLNavCtrl_GetImageList(plugin.hwndLibraryParent), -1,
													   plugin.hDllInstance, MAKEINTRESOURCE(IDR_PLAYLIST_ICON));
	}

	return playlist_icon_index;
}

int IconStore::GetVideoIcon()
{
	if (-1 == video_icon_index)
	{
		MLIMAGELISTTAG imageTag;

		imageTag.hmlil =  MLNavCtrl_GetImageList(plugin.hwndLibraryParent);
		imageTag.nTag = 102; // video node image tag registered by ml_local

		if (FALSE != MLImageList_GetIndexFromTag(plugin.hwndLibraryParent, &imageTag))
			video_icon_index = imageTag.mlilIndex;
		else
			video_icon_index = IconStore_RegisterBitmap(imageTag.hmlil, -1, 
									plugin.hDllInstance, MAKEINTRESOURCE(IDR_VIDEO_ICON));
	}

	return video_icon_index;
}

int IconStore::GetDeviceIcon()
{
	if (-1 == device_icon_index)
	{
		HMLIMGLST imageList;

		imageList  = MLNavCtrl_GetImageList(plugin.hwndLibraryParent);
		if (NULL != imageList)
		{
			device_icon_index = IconStore_RegisterBitmap(imageList, -1,
									plugin.hDllInstance, MAKEINTRESOURCE(IDR_DEVICE_ICON));
		}
	}

	return device_icon_index;
}

int IconStore::GetQueueIcon(int iconIndex)
{
	if (!queue_icon_index[iconIndex])
	{
		HMLIMGLST imageList;

		imageList = MLNavCtrl_GetImageList(plugin.hwndLibraryParent);
		if (NULL != imageList)
		{
			queue_icon_index[iconIndex] = IconStore_RegisterBitmap(imageList, -1,
												plugin.hDllInstance, MAKEINTRESOURCE(IDB_XFER_QUEUE_16 + iconIndex));
		}
	}

	return queue_icon_index[iconIndex];
}

void IconStore::ReleaseResourceIcon(int iconIndex)
{
	if (-1 == iconIndex)
		return;

	size_t index = iconList.size();
	while(index--)
	{
		ResourceIcon *icon = &iconList[index];
		if (icon->index == iconIndex)
		{
			if (0 != icon->ref)
			{
				icon->ref--;
				if (0 == icon->ref)
				{
					IconStore_FreeResourceName(icon->name);
					icon->name = NULL;
					icon->module = NULL;
				}
			}
			break;
		}
	}
}

int IconStore::GetResourceIcon(HINSTANCE module, const wchar_t *name)
{
	ResourceIcon *icon;
	size_t index;

	if (module == plugin.hDllInstance &&
		FALSE != IS_INTRESOURCE(name) &&
		name == MAKEINTRESOURCE(IDR_DEVICE_ICON))
	{
		return GetDeviceIcon();
	}

	index = iconList.size();
	while(index--)
	{
		icon = &iconList[index];
		if (icon->module == module &&
			FALSE != IconStore_IsResourceNameEqual(icon->name, name))
		{
			icon->ref++;
			return icon->index;
		}
	}

	return RegisterResourceIcon(module, name);
}

int IconStore::RegisterResourceIcon(HINSTANCE module, const wchar_t *name)
{
	ResourceIcon *icon, iconData;
	HMLIMGLST imageList;
	size_t index;

	imageList = MLNavCtrl_GetImageList(plugin.hwndLibraryParent);
	if (NULL == imageList)
		return -1;

	index = iconList.size();
	while(index--)
	{
		icon = &iconList[index];
		if (0 == icon->ref)
		{
			break;
		}
	}
	if ((size_t)-1 == index)
	{
		icon = &iconData;
		icon->index = -1;
	}

	if (FALSE != IS_INTRESOURCE(name))
		icon->name = (wchar_t*)name;
	else
	{
		icon->name = _wcsdup(name);
		if (NULL == icon->name)
			return -1;
	}

	icon->ref = 1;
	icon->module = module;
	icon->index = IconStore_RegisterBitmap(imageList, icon->index, icon->module, icon->name);

	if (-1 != icon->index && &iconData == icon)
		iconList.push_back(iconData);

	return icon->index;
}