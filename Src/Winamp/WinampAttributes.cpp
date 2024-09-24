#include "main.h"
#include "api.h"
#include "WinampAttributes.h"
#include "InternetConfigGroup.h"
#include "VideoConfigGroup.h"
#include "PlaybackConfigGroup.h"
#include "EQConfigGroup.h"
#include "DeveloperConfigGroup.h"
#include "AccessibilityConfigGroup.h"
#include "../config/config.h"
extern Config config;

/* --- Video --- */
VideoConfigGroup videoConfigGroup;
_bool config_video_overlays(true);
_bool config_video_yv12(true);
_bool config_video_vsync2(true);
_bool config_video_ddraw(true);
_bool config_video_gdiplus(false);
_mutable_bool config_video_autoopen(true);
_mutable_bool config_video_autoclose(true);
_mutable_bool config_video_auto_fs(false);

/* --- Internet --- */
InternetConfigGroup internetConfigGroup;
_bool config_proxy80(false);

/* --- Playback --- */
PlaybackConfigGroup playbackConfigGroup;
_unsigned config_audio_bits(16);
_bool config_audio_mono(false);
_bool config_audio_surround(true);
_bool config_audio_dither(true);
_bool config_replaygain(false);
_unsigned config_replaygain_mode(RG_MODE_GAIN_NOCLIP);
_unsigned config_replaygain_source(RG_SOURCE_TRACK);
_bool config_replaygain_preferred_only(false);
_float config_replaygain_non_rg_gain(-6);
_float config_replaygain_preamp(0);
_int config_playback_thread_priority(THREAD_PRIORITY_HIGHEST);

/* --- EQ --- */
EQConfigGroup eqConfigGroup;
_int config_eq_frequencies(EQ_FREQUENCIES_WINAMP);
_int config_eq_type(EQ_TYPE_4FRONT);
_bool config_eq_limiter(true);

/* --- Developer --- */
DeveloperConfigGroup developerConfigGroup;


/* --- Accessibility --- */
AccessibilityConfigGroup accessibilityConfigGroup;
_bool config_accessibility_modalbeep(false);
_bool config_accessibility_modalflash(true);
	
void RegisterConfigGroups()
{
	config.RegisterGroup(&internetConfigGroup);
	config.RegisterGroup(&videoConfigGroup);
	config.RegisterGroup(&playbackConfigGroup);
	config.RegisterGroup(&eqConfigGroup);
	config.RegisterGroup(&developerConfigGroup);
	config.RegisterGroup(&accessibilityConfigGroup);
}

