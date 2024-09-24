#pragma once
#include <bfc/platform/types.h>
#include "mkv_reader.h"
#include <vector>

const uint32_t mkv_segment_tracks = 0x654ae6b;
const uint32_t mkv_tracks_trackentry = 0x2e;
const uint32_t mkv_tracks_tracknumber=0x57;
const uint32_t mkv_tracks_trackuid=0x33c5;
const uint32_t mkv_tracks_tracktype=0x3;
const uint32_t mkv_tracks_flagenabled=0x39;
const uint32_t mkv_tracks_flagdefault=0x8;
const uint32_t mkv_tracks_flagforced=0x15aa;
const uint32_t mkv_tracks_flaglacing=0x1c;
const uint32_t mkv_tracks_mincache=0x2de7;
const uint32_t mkv_tracks_tracktimecodescale=0x3314f;
const uint32_t mkv_tracks_maxblockadditionid=0x15ee;
const uint32_t mkv_tracks_codecid=0x6;
const uint32_t mkv_tracks_codecdecodeall=0x2A;
const uint32_t mkv_tracks_defaultduration=0x3e383;
const uint32_t mkv_tracks_codecprivate=0x23a2;
const uint32_t mkv_tracks_language=0x2b59c;
const uint32_t mkv_tracks_name=0x136e;
const uint32_t mkv_tracks_maxcache=0x2df8;

// Track - video settings
const uint32_t mkv_tracks_video=0x60;
const uint32_t mkv_video_pixelwidth = 0x30;
const uint32_t mkv_video_pixelheight = 0x3a;
const uint32_t mkv_video_flaginterlaced = 0x1a;
const uint32_t mkv_video_displaywidth = 0x14b0;
const uint32_t mkv_video_displayheight = 0x14ba;
const uint32_t mkv_video_pixelcropbottom = 0x14aa;
const uint32_t mkv_video_pixelcroptop = 0x14bb;
const uint32_t mkv_video_pixelcropleft = 0x14cc;
const uint32_t mkv_video_pixelcropright = 0x14dd;


// Track - audio settings;
const uint32_t mkv_tracks_audio=0x61;
const uint32_t mkv_audio_samplingfrequency=0x35;
const uint32_t mkv_audio_channels = 0x1f;
const uint32_t mkv_audio_output_samplingfrequency=0x38b5;

// Track Types
enum
{
	mkv_track_type_video = 0x01,
	mkv_track_type_audio = 0x02,
	mkv_track_type_complex = 0x03, // i.e., combined video and audio
	mkv_track_type_logo = 0x10,
	mkv_track_type_subtitle = 0x11,
	mkv_track_type_buttons = 0x12,
	mkv_track_type_control = 0x20,
};
/* TODO: benski>

*/
namespace nsmkv
{
#pragma pack(push, 8)
	struct VideoData
	{
		size_t struct_size;
		uint64_t pixel_width;
		uint64_t pixel_height;
		uint64_t pixel_crop_bottom;
		uint64_t pixel_crop_top;
		uint64_t pixel_crop_left;
		uint64_t pixel_crop_right;
		uint64_t display_width;
		uint64_t display_height;
		uint64_t display_unit;
		bool flag_interlaced;
	};
#pragma pack(pop)
	class Video : public VideoData
	{
	public:
		Video()
#ifdef WA_VALIDATE
			:
		pixel_width_found(false),
		pixel_height_found(false),
		pixel_crop_bottom_found(false),
		pixel_crop_top_found(false),
		pixel_crop_left_found(false),
		pixel_crop_right_found(false),
		display_width_found(false),
		display_height_found(false),
//		display_unit_found(false),
		flag_interlaced_found(false)
#endif
		{
			struct_size = sizeof(VideoData);
			pixel_width=0;
			pixel_height=0;
			pixel_crop_bottom=0;
			pixel_crop_top=0;
			pixel_crop_left=0;
			pixel_crop_right=0;
			display_width=0;
			display_height=0;
			display_unit=0;
			flag_interlaced=false;
		}
#ifdef WA_VALIDATE
		bool pixel_width_found;
		bool pixel_height_found;
		bool pixel_crop_bottom_found;
		bool pixel_crop_top_found;
		bool pixel_crop_left_found;
		bool pixel_crop_right_found;
		bool display_width_found;
		bool display_height_found;
//		bool display_unit_found;
		bool flag_interlaced_found;
#endif
	};
#pragma pack(push, 8)
	struct AudioData
	{
		size_t struct_size;
		uint64_t sampling_frequency;
		uint64_t channels;
		uint64_t bit_depth;
		uint64_t output_sampling_frequency;
	};
#pragma pack(pop)
	class Audio : public AudioData
	{
	public:
		Audio()
#ifdef WA_VALIDATE
			:
			sampling_frequency_found(false),
			channels_found(false),
//			bit_depth_found(false),
			output_sampling_frequency_found(false)
#endif
		{
			struct_size = sizeof(AudioData);
			sampling_frequency=8000;
			channels=1;
			bit_depth=0;
			output_sampling_frequency=0;
		}
#ifdef WA_VALIDATE
		bool sampling_frequency_found;
		bool channels_found;
//		bool bit_depth_found;
		bool output_sampling_frequency_found;
#endif
	};
#pragma pack(push, 8)
	struct TrackEntryData
	{
		size_t struct_size;
		uint64_t track_number;
		uint64_t track_uid;
		uint64_t track_type;
		bool flag_enabled;
		bool flag_default;
		bool flag_forced;
		bool flag_lacing;
		bool decode_all;
		uint64_t min_cache;
		uint64_t max_cache;
		uint64_t default_duration;
		double track_timecode_scale;
		uint64_t max_block_additional_id;
		char *name;
		char *language;
		char *codec_id;
		void *codec_private;
		size_t codec_private_len;
		char *codec_name;
		uint64_t attachment_link;
	};
#pragma pack(pop)
	class TrackEntry : public TrackEntryData
	{
	public:
		TrackEntry()
#ifdef WA_VALIDATE
			:
			track_number_found(false),
			track_uid_found(false),
			track_type_found(false),
			flag_enabled_found(false),
			flag_default_found(false),
			flag_forced_found(false),
			flag_lacing_found(false),
			min_cache_found(false),
			max_cache_found(false),
			default_duration_found(false),
			track_timecode_scale_found(false),
			max_block_additional_id_found(false),
			decode_all_found(false),
			name_found(false),
			language_found(false),
			codec_id_found(false),
			codec_private_found(false),
			codec_name_found(false)
//			attachment_link_found(false)
#endif
		{
			struct_size = sizeof(TrackEntryData);
			track_number = 0;
			track_uid = 0;
			track_type = 0;
			flag_enabled = true;
			flag_default = true;
			flag_forced = false;
			flag_lacing = false;
			min_cache = 0;
			max_cache = 0;
			default_duration = 0;
			track_timecode_scale = 0;
			max_block_additional_id = 0;
			decode_all = true;
			name = 0;
			language = 0;
			codec_id = 0;
			codec_private = 0;
			codec_private_len = 0;
			codec_name = 0;
			attachment_link = 0;
		}
		~TrackEntry()
		{
			free(name);
			free(language);
			free(codec_id);
			free(codec_private);
			free(codec_name);
		}
		void Own(char *&field, char *value)
		{
			if (field)
				free(field);
			field = value;
		}
		void OwnCodecPrivate(void *_codec_private, size_t _codec_private_len)
		{
			free(codec_private);
			codec_private=_codec_private;
			codec_private_len = _codec_private_len;
		}

		Video video;
		Audio audio;
#ifdef WA_VALIDATE
		bool track_number_found;
		bool track_uid_found;
		bool track_type_found;
		bool flag_enabled_found;
		bool flag_default_found;
		bool flag_forced_found;
		bool flag_lacing_found;
		bool min_cache_found;
		bool max_cache_found;
		bool default_duration_found;
		bool track_timecode_scale_found;
		bool max_block_additional_id_found;
		bool decode_all_found;
		bool name_found;
		bool language_found;
		bool codec_id_found;
		bool codec_private_found;
		bool codec_name_found;
//		bool attachment_link_found;
#endif
	};
	class Tracks
	{
	public:
		~Tracks()
		{
			//tracks.deleteAll();
			for (auto obj : tracks)
			{
				delete obj;
			}
			tracks.clear();
		}
		const nsmkv::TrackEntry *EnumTrack(size_t i) const;
		typedef std::vector<nsmkv::TrackEntry*> TrackEntryList;
		TrackEntryList tracks;
	};
	uint64_t ReadTracks(nsmkv::MKVReader *reader, uint64_t size, nsmkv::Tracks &tracks);
}