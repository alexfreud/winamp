/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "VideoConfigGroup.h"
#include "WinampAttributes.h"


ifc_configitem *VideoConfigGroup::GetItem(const wchar_t *name)
{
	if (!wcscmp(name, L"overlay"))
		return &config_video_overlays;
	else if (!wcscmp(name, L"YV12"))
		return &config_video_yv12;
	else if (!wcscmp(name, L"vsync"))
		return &config_video_vsync2;
	else if (!wcscmp(name, L"ddraw"))
		return &config_video_ddraw;
	else if (!wcscmp(name, L"gdiplus"))
		return &config_video_gdiplus;
	else if (!wcscmp(name, L"autoopen"))
		return &config_video_autoopen;
	else if (!wcscmp(name, L"autoclose"))
		return &config_video_autoclose;
	else if (!wcscmp(name, L"auto_fs"))
		return &config_video_auto_fs;

	return 0;
}


#define CBCLASS VideoConfigGroup
START_DISPATCH;
CB(IFC_CONFIGGROUP_GETITEM, GetItem)
CB(IFC_CONFIGGROUP_GETGUID, GetGUID)
END_DISPATCH;
#undef CBCLASS