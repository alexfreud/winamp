/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "PlaybackConfigGroup.h"
#include "../Agave/Config/ifc_configitem.h"

#include "WinampAttributes.h"

ifc_configitem *PlaybackConfigGroup::GetItem(const wchar_t *name)
{
	if (!wcscmp(name, L"bits"))
		return &config_audio_bits;
	else if (!wcscmp(name, L"mono"))
		return &config_audio_mono;
	else if (!wcscmp(name, L"surround"))
		return &config_audio_surround;
	else if (!wcscmp(name, L"dither"))
		return &config_audio_dither;
	else if (!wcscmp(name, L"replaygain"))
		return &config_replaygain;
	else if (!wcscmp(name, L"replaygain_mode"))
		return &config_replaygain_mode;
	else if (!wcscmp(name, L"replaygain_source"))
		return &config_replaygain_source;
	else if (!wcscmp(name, L"replaygain_preferred_only"))
		return &config_replaygain_preferred_only;
	else if (!wcscmp(name, L"non_replaygain"))
		return &config_replaygain_non_rg_gain;
	else if (!wcscmp(name, L"replaygain_preamp"))
		return &config_replaygain_preamp;
	else if (!wcscmp(name, L"priority"))
	return &config_playback_thread_priority;

	return 0;
}


#define CBCLASS PlaybackConfigGroup
START_DISPATCH;
CB(IFC_CONFIGGROUP_GETITEM, GetItem)
CB(IFC_CONFIGGROUP_GETGUID, GetGUID)
END_DISPATCH;
#undef CBCLASS
