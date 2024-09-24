#ifndef NULLSOFT_CONFIGH
#define NULLSOFT_CONFIGH
#include "dsound.h"
#ifdef WM_DEFINE_CONFIG
#define DEFVAL(x) =x
#define CFGEXTERN
#else
#define DEFVAL(x)
#define CFGEXTERN extern
#endif

#define CFG(type, name, defval) CFGEXTERN type config_##name DEFVAL(defval); CFGEXTERN type default_##name DEFVAL(defval);

CFG(bool, lowmemory, true); 
CFG(bool, clock, true); 

CFG(bool, video_dedicated_thread, true); 
CFG(bool, video_early, false); 
CFG(int, video_early_pad, 500); 
CFG(bool, video_outoforder, true); 
CFG(bool, video_catchup, true);
CFG(int, video_jitter, 5);
CFG(int, video_drop_threshold, 15);
CFG(size_t, video_cache_frames,  16);
CFG(bool, video_notifylate, true);
CFG(bool, video_framedropoffset, false);
//CFG(bool, video_flip, false);

CFG(bool, audio_outoforder, false);
CFG(bool, audio_dedicated_thread, true);
CFG(int, audio_early_pad, 0);
CFG(bool, audio_early, false);
CFG(size_t, audio_cache_frames, 12);
CFG(DWORD, audio_num_channels, DSSPEAKER_5POINT1);

CFG(bool, no_silent, false);
CFG(bool, untrusted_ok, false);

CFG(bool, http_metadata, false);
CFG(size_t, buffer_time, 5000);

CFG(int, col1, -1);
CFG(int, col2, -1);

extern bool config_no_video;

CFG(bool, extra_asx_extensions, false);
void ReadConfig(), WriteConfig(), DefaultConfig();

#endif