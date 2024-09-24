#define WM_DEFINE_CONFIG 1
#include "config.h"
#include "loadini.h"
#include "main.h"
#include "../nu/Config.h"

bool config_no_video = false;
extern Nullsoft::Utility::Config wmConfig;
#pragma warning(disable:4800)
#define READ(type, name) config_##name = (type)wmConfig.cfg_int(TEXT("config_") TEXT(#name), default_##name)
#define WRITE(type, name) wmConfig.cfg_int(TEXT("config_") TEXT(#name), default_##name) = (int)config_##name
#define DEFAULT(name) config_##name = default_##name

void ReadConfig()
{
	READ(bool, lowmemory);
	READ(bool, clock);

	READ(bool, video_dedicated_thread);
	READ(bool, video_early);
	READ(int, video_early_pad);
	READ(bool, video_outoforder);
	READ(bool, video_catchup);
	READ(int, video_jitter);
	READ(int, video_drop_threshold);
	READ(size_t, video_cache_frames);
	READ(bool, video_notifylate);
	READ(bool, video_framedropoffset);

	READ(bool, audio_outoforder);
	READ(bool, audio_dedicated_thread);
	READ(int, audio_early_pad);
	READ(bool, audio_early);
	READ(size_t, audio_cache_frames);
	READ(size_t, audio_num_channels);

//	READ(bool, no_silent);
//	READ(bool, untrusted_ok);

	READ(bool, http_metadata);
	READ(size_t, buffer_time);

	READ(bool, extra_asx_extensions);

	READ(int, col1);
	READ(int, col2);
}

void WriteConfig()
{
	WRITE(bool, lowmemory);

	WRITE(bool, clock);

	WRITE(bool, video_dedicated_thread);
	WRITE(bool, video_early);
	WRITE(int, video_early_pad);
	WRITE(bool, video_outoforder);
	WRITE(bool, video_catchup);
	WRITE(int, video_jitter);
	WRITE(int, video_drop_threshold);
	WRITE(size_t, video_cache_frames);
	WRITE(bool, video_notifylate);
	WRITE(bool, video_framedropoffset);

	WRITE(bool, audio_outoforder);
	WRITE(bool, audio_dedicated_thread);
	WRITE(int, audio_early_pad);
	WRITE(bool, audio_early);
	WRITE(size_t, audio_cache_frames);
	WRITE(size_t, audio_num_channels);

//	WRITE(bool, no_silent);
//	WRITE(bool, untrusted_ok);

	WRITE(bool, http_metadata);
	WRITE(size_t, buffer_time);

	WRITE(bool, extra_asx_extensions);

	WRITE(int, col1);
	WRITE(int, col2);
}

void DefaultConfig()
{
	DEFAULT(http_metadata);
//	DEFAULT(no_silent);
//	DEFAULT(untrusted_ok);
	DEFAULT(buffer_time);
	DEFAULT(audio_num_channels);

	DEFAULT(audio_outoforder);
	DEFAULT(audio_dedicated_thread);
	DEFAULT(audio_early_pad);
	DEFAULT(audio_early);
	DEFAULT(audio_cache_frames);

	DEFAULT(lowmemory);

	DEFAULT(clock);

	DEFAULT(video_dedicated_thread);
	DEFAULT(video_early);
	DEFAULT(video_early_pad);
	DEFAULT(video_outoforder);
	DEFAULT(video_catchup);
	DEFAULT(video_jitter);
	DEFAULT(video_drop_threshold);
	DEFAULT(video_cache_frames);
	DEFAULT(video_notifylate);
	DEFAULT(video_framedropoffset);

	DEFAULT(extra_asx_extensions);

	DEFAULT(col1);
	DEFAULT(col2);
}