#ifndef NULLSOFT_WINAMP_WINAMPATTRIBUTES_H
#define NULLSOFT_WINAMP_WINAMPATTRIBUTES_H

#include "attributes.h"

// internet
extern _bool config_proxy80;

// video
extern _bool config_video_overlays;
extern _bool config_video_yv12;
extern _bool config_video_vsync2;
extern _bool config_video_ddraw;
extern _bool config_video_gdiplus;
extern _mutable_bool config_video_autoopen;
extern _mutable_bool config_video_autoclose;
extern _mutable_bool config_video_auto_fs;

// audio
extern _unsigned config_audio_bits;
extern _bool config_audio_mono;
extern _bool config_audio_surround;
extern _bool config_audio_dither;

// playback
extern _int config_playback_thread_priority;

// EQ
enum { EQ_FREQUENCIES_WINAMP = 0, EQ_FREQUENCIES_ISO = 1, };
extern _int config_eq_frequencies;
enum { EQ_TYPE_4FRONT = 0, EQ_TYPE_CONSTANT_Q = 1, };
extern _int config_eq_type;
extern _bool config_eq_limiter;

// replay gain
extern _bool config_replaygain;
enum { RG_MODE_GAIN=0, RG_MODE_GAIN_NOCLIP=1, RG_MODE_NORMALIZE=2, RG_MODE_NOCLIP=3};
extern _unsigned config_replaygain_mode;
enum { RG_SOURCE_TRACK=0, RG_SOURCE_ALBUM=1};
extern _unsigned config_replaygain_source;
extern _bool config_replaygain_preferred_only;
extern _float config_replaygain_non_rg_gain;
extern _float config_replaygain_preamp;

// accessibility
extern _bool config_accessibility_modalbeep;
extern _bool config_accessibility_modalflash;

#endif