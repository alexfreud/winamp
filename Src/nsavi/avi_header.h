#pragma once
#include <bfc/platform/types.h>

namespace nsavi
{
#pragma pack(push, 1)
	static const uint32_t avi_header_flags_has_index = 0x10;
	static const uint32_t avi_header_flags_must_use_index = 0x20;
	static const uint32_t avi_header_flags_is_interleaved = 0x100;
	static const uint32_t avi_header_trust_ck_type = 0x800; // benski> i have no fucking clue
	static const uint32_t avi_header_flags_was_capture_file = 0x10000;
	static const uint32_t avi_header_flags_copyrighted = 0x20000;

	static const uint32_t stream_type_audio = 0x73647561;
	static const uint32_t stream_type_video = 0x73646976;

	const uint16_t audio_format_pcm = 1;
	const uint16_t audio_format_ms_adpcm = 2;
	const uint16_t audio_format_alaw = 6;
	const uint16_t audio_format_ulaw = 7;
	const uint16_t audio_format_ima_adpcm = 17;
	const uint16_t audio_format_truespeech = 34;
	const uint16_t audio_format_mp2 = 80;
	const uint16_t audio_format_mp3 = 85;
	const uint16_t audio_format_a52 = 8192;
	const uint16_t audio_format_aac = 255;
	const uint16_t audio_format_vorbis = 26447;
	const uint16_t audio_format_speex = 41225;
	const uint16_t audio_format_extensible = 65534; // aka WAVE_FORMAT_EXTENSIBLE
	const uint16_t audio_format_dts = 8193;

	static uint32_t video_format_rgb = 0;
	static uint32_t video_format_rle8 = 1;
	static uint32_t video_format_rle4 = 2;

	static const uint32_t idx1_flags_keyframe = 0x10;
	static const uint32_t idx1_flags_no_duration= 0x100;

	struct AVIH
	{
		uint32_t size_bytes; 
		uint32_t microseconds_per_frame;
		uint32_t max_bytes_per_second;
		uint32_t padding_granularity;
		uint32_t flags;
		uint32_t total_frames;
		uint32_t initial_frames;
		uint32_t streams;
		uint32_t suggested_buffer_size;
		uint32_t width;
		uint32_t height;
		uint8_t reserved[16];
	};

	struct STRH
	{
		uint32_t size_bytes;
		uint32_t stream_type;
		uint32_t fourcc;
		uint32_t flags;
		uint16_t priority;
		uint16_t language;
		uint32_t initial_frames;
		uint32_t scale;
		uint32_t rate;
		uint32_t start;
		uint32_t length;
		uint32_t suggested_buffer_size;
		uint32_t quality;
		uint32_t sample_size;
		int16_t left;
		int16_t top;
		int16_t right;
		int16_t bottom;
	};

	struct VIDEO_FIELD_DESC
	{
		uint32_t compressed_height;
		uint32_t compressed_width;
		uint32_t valid_height;
		uint32_t valid_width;
		uint32_t valid_x_offset;
		uint32_t valid_y_offset;
		uint32_t x_offset;
		uint32_t valid_y_start_line;
	};

	struct VPRP
	{
		uint32_t size_bytes;
		uint32_t video_format_token;
		uint32_t video_standard;
		uint32_t vertical_refresh_rate;
		uint32_t horizontal_total;
		uint32_t vertical_total;
		uint32_t aspect_ratio;
		uint32_t frame_width;
		uint32_t frame_height;
		uint32_t field_info_size;
		VIDEO_FIELD_DESC field_info[1];
	};

	struct STRF
	{
		uint32_t size_bytes;
	};

	struct STRD
	{
		uint32_t size_bytes;
	};

	struct STRN
	{
		uint32_t size_bytes;
	};

	struct DMLH
	{
		uint32_t size_bytes;
		uint32_t total_frames;
	};

	struct IDX1_INDEX
	{
		uint32_t chunk_id;
		uint32_t flags;
		uint32_t offset;
		uint32_t chunk_size;
	};

	struct IDX1
	{
		uint32_t index_count;
		IDX1_INDEX indices[1];
	};

	static const uint8_t indx_type_master = 0x0;
	static const uint8_t indx_type_chunk = 0x1;
	static const uint8_t indx_type_data = 0x80;
	static const uint8_t indx_subtype_field = 0x1;



	struct INDX
	{
		uint32_t size_bytes;
		uint16_t entry_size;
		uint8_t index_sub_type;
		uint8_t index_type;
		uint32_t number_of_entries;
		uint32_t chunk_id;
	};

	struct INDX_MASTER_ENTRY
	{
		uint64_t offset;
		uint32_t index_size;
		uint32_t duration;
	};

	struct AVISUPERINDEX 
	{
		INDX indx;
		uint32_t reserved[3];
		INDX_MASTER_ENTRY entries[1]; // actual size determined by indx.number_of_entries
	};

	struct INDX_CHUNK_ENTRY
	{
		uint32_t offset;
		uint32_t size ;// bit 31 is set if this is NOT a keyframe
	};

	struct AVISTDINDEX
	{
		INDX indx;
		uint64_t base_offset;
		uint32_t reserved;
		INDX_CHUNK_ENTRY entries[1]; // actual size determined by indx.number_of_entries
	};

	struct INDX_FIELD_ENTRY
	{
		uint32_t offset;
		uint32_t size; // size of all fields.  bit 31 set for NON-keyframes
		uint32_t offset_field2; // offset to second field
	};

	struct AVIFIELDINDEX
	{
		INDX indx;
		uint64_t base_offset;
		uint32_t reserved;
		INDX_FIELD_ENTRY entries[1]; // actual size determined by indx.number_of_entries
	};

	struct video_format
	{
		uint32_t size_bytes;
		uint32_t video_format_size_bytes; // redundant, I know
		int32_t width;
		int32_t height;
		uint16_t planes;
		uint16_t bits_per_pixel;
		uint32_t compression;
		uint32_t image_size;
		int32_t x_pixels_per_meter;
		int32_t y_pixels_per_meter;
		uint32_t color_used;
		uint32_t color_important;
	};


	struct audio_format
	{
		uint32_t size_bytes;
		uint16_t format;
		uint16_t channels;
		uint32_t sample_rate;
		uint32_t average_bytes_per_second;
		uint16_t block_align;
		uint16_t bits_per_sample;
		uint16_t extra_size_bytes;
	};

	struct mp3_format
	{
		audio_format format;
		uint16_t id;
		uint32_t flags;
		uint16_t block_size;
		uint16_t frames_per_block;
		uint16_t codec_delay;
	};

	struct STRL
	{
		STRH *stream_header;
		STRF *stream_format;
		STRD *stream_data;
		STRN *stream_name;
		INDX *stream_index;
		VPRP *video_properties;
	};
#pragma pack(pop)
}