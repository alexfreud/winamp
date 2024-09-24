#include "Tracks.h"
#include "read.h"
#include "global_elements.h"

// returns bytes read.  0 means EOF
static uint64_t ReadTracksVideo(nsmkv::MKVReader *reader, uint64_t size, nsmkv::Video &video)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node(reader, &node);

		if (bytes_read == 0)
			return 0;

		// benski> checking bytes_read and node.size separately prevents possible integer overflow attack
		if (bytes_read > size)
			return 0;
		total_bytes_read+=bytes_read;
		size-=bytes_read;

		if (node.size > size)
			return 0;
		total_bytes_read+=node.size;
		size-=node.size;

		switch(node.id)
		{
		case mkv_video_pixelwidth:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Pixel Width: %I64u\n", val);
				video.pixel_width_found=true;
#endif
				video.pixel_width=val;
			}
			break;
		case mkv_video_pixelheight:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Pixel Height: %I64u\n", val);
				video.pixel_height_found=true;
#endif
				video.pixel_height=val;
			}
			break;
		case mkv_video_flaginterlaced:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Flag Interlaced: 0x%I64x\n", val);
				video.flag_interlaced_found=true;
#endif
				video.flag_interlaced = !!val;
			}
			break;
		case mkv_video_displaywidth:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Display Width: %I64u\n", val);
				video.display_width_found = true;
#endif
				video.display_width = val;
			}
			break;
		case mkv_video_displayheight:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Display Height: %I64u\n", val);
				video.display_height_found = true;
#endif
				video.display_height = val;
			}
			break;
		case mkv_video_pixelcropleft:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Pixel Crop Left: %I64u\n", val);
				video.pixel_crop_left_found = true;
#endif
				video.pixel_crop_left = val;
			}
			break;
		case mkv_video_pixelcroptop:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Pixel Crop Top: %I64u\n", val);
				video.pixel_crop_top_found = true;
#endif
				video.pixel_crop_top = val;
			}
			break;
		case mkv_video_pixelcropbottom:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Pixel Crop Bottom: %I64u\n", val);
				video.pixel_crop_bottom_found = true;
#endif
				video.pixel_crop_bottom = val;
			}
			break;
		case mkv_video_pixelcropright:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Pixel Crop Right: %I64u\n", val);
				video.pixel_crop_right_found = true;
#endif
				video.pixel_crop_right = val;
			}
			break;
		default:
			nsmkv::ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}

// returns bytes read.  0 means EOF
static uint64_t ReadTracksAudio(nsmkv::MKVReader *reader, uint64_t size, nsmkv::Audio &audio)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node(reader, &node);

		if (bytes_read == 0)
			return 0;

		// benski> checking bytes_read and node.size separately prevents possible integer overflow attack
		if (bytes_read > size)
			return 0;
		total_bytes_read+=bytes_read;
		size-=bytes_read;

		if (node.size > size)
			return 0;
		total_bytes_read+=node.size;
		size-=node.size;

		switch(node.id)
		{
		case mkv_audio_samplingfrequency:
			{
				double val;
				if (read_float(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Sampling Frequency: %g\n", val);
				audio.sampling_frequency_found = true;
#endif
				audio.sampling_frequency = (uint64_t)val;
			}
			break;
		case mkv_audio_channels:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Channels: %I64u\n", val);
				audio.channels_found = true;
#endif
				audio.channels = val;
			}
			break;
		case mkv_audio_output_samplingfrequency:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("      Output Sampling Frequency: %I64u\n", val);
				audio.output_sampling_frequency_found = true;
#endif
				audio.output_sampling_frequency = val;
			}
			break;
		default:
			nsmkv::ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}


static uint64_t ReadTrackEntry(nsmkv::MKVReader *reader, uint64_t size, nsmkv::TrackEntry &track_entry)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node(reader, &node);

		if (bytes_read == 0)
			return 0;

		// benski> checking bytes_read and node.size separately prevents possible integer overflow attack
		if (bytes_read > size)
			return 0;
		total_bytes_read+=bytes_read;
		size-=bytes_read;

		if (node.size > size)
			return 0;
		total_bytes_read+=node.size;
		size-=node.size;

		switch(node.id)
		{
		case mkv_tracks_tracknumber:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;
#ifdef WA_VALIDATE
				printf("    Track Number: %I64u\n", val);
				track_entry.track_number_found = true;
#endif
				track_entry.track_number = val;
			}
			break;
		case mkv_tracks_trackuid:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Track UID: %I64u\n", val);
				track_entry.track_uid_found = true;
#endif
				track_entry.track_uid = val;
			}
			break;
		case mkv_tracks_tracktype:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Track Type: 0x%I64x\n", val);
				track_entry.track_type_found = true;
#endif
				track_entry.track_type = val;
			}
			break;
		case mkv_tracks_flagenabled:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Flag Enabled: 0x%I64x\n", val);
				track_entry.flag_enabled_found = true;
#endif
				track_entry.flag_enabled = !!val;
			}
			break;
		case mkv_tracks_flagdefault:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Flag Default: 0x%I64x\n", val);
				track_entry.flag_default_found = true;
#endif
				track_entry.flag_default = !!val;
			}
			break;
		case mkv_tracks_flagforced:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Flag Forced: 0x%I64x\n", val);
				track_entry.flag_forced_found = true;
#endif
				track_entry.flag_forced = !!val;
			}
			break;
		case mkv_tracks_flaglacing:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Flag Lacing: 0x%I64x\n", val);
				track_entry.flag_lacing_found = true;
#endif
				track_entry.flag_lacing = !!val;
			}
			break;
		case mkv_tracks_mincache:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Min Cache: %I64u\n", val);
				track_entry.min_cache_found = true;
#endif
				track_entry.min_cache = !!val;
			}
			break;
		case mkv_tracks_tracktimecodescale:
			{
				double val;
				if (read_float(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Track Time Code Scale: %g\n", val);
				track_entry.track_timecode_scale_found = true;
#endif
				track_entry.track_timecode_scale = val;
			}
			break;
		case mkv_tracks_maxblockadditionid:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Max Block Addition ID: %I64u\n", val);
				track_entry.max_block_additional_id_found=true;
#endif
				track_entry.max_block_additional_id=val;
			}
			break;
		case mkv_tracks_codecid:
			{
				char *utf8=0;
				if (read_utf8(reader, node.size, &utf8) == 0)
					return 0;

#ifdef WA_VALIDATE
				if (utf8)
					printf("    Codec ID: %s\n", utf8);
				track_entry.codec_id_found = true;
#endif
				track_entry.Own(track_entry.codec_id, utf8);
			}
			break;
		case mkv_tracks_codecdecodeall:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Codec Decode All: %I64u\n", val);
				track_entry.decode_all_found = true;
#endif
				track_entry.decode_all = !!val;
			}
			break;
		case mkv_tracks_defaultduration:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Default Duration: %I64u\n", val);
				track_entry.default_duration_found = true;
#endif
				track_entry.default_duration = val;
			}
			break;
		case mkv_tracks_codecprivate:
			{
#ifdef WA_VALIDATE
				printf("    Codec Private: binary size %I64u\n", node.size);
#endif

				void *codec_private = malloc((size_t)node.size);
				if (!codec_private)
					return 0;
				size_t bytes_read;
				reader->Read(codec_private, (size_t)node.size, &bytes_read);
				if (bytes_read != node.size)
				{
					free(codec_private);
					return 0;
				}
				track_entry.OwnCodecPrivate(codec_private, (size_t)node.size);
#ifdef WA_VALIDATE
				track_entry.codec_private_found = true;
#endif
			}
			break;
		case mkv_tracks_language:
			{
				char *utf8=0;
				if (read_utf8(reader, node.size, &utf8) == 0)
					return 0;

#ifdef WA_VALIDATE
				if (utf8)
					printf("    Codec Language: %s\n", utf8);
				track_entry.language_found = true;
#endif
				track_entry.Own(track_entry.language, utf8);
			}
			break;
		case mkv_tracks_video:
			{
#ifdef WA_VALIDATE
				printf("    Video Settings\n");
#endif
				if (ReadTracksVideo(reader, node.size, track_entry.video) == 0)
					return 0;
			}
			break;
		case mkv_tracks_audio:
			{
#ifdef WA_VALIDATE
				printf("    Audio Settings\n");
#endif
				if (ReadTracksAudio(reader, node.size, track_entry.audio) == 0)
					return 0;
			}
			break;
		case mkv_tracks_name:
			{
				char *utf8=0;
				if (read_utf8(reader, node.size, &utf8) == 0)
					return 0;

#ifdef WA_VALIDATE
				if (utf8)
					printf("    Track Name: %s\n", utf8);
				track_entry.name_found = true;
#endif
				track_entry.Own(track_entry.name, utf8);
			}
			break;
		case mkv_tracks_maxcache:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Max Cache: %I64u\n", val);
				track_entry.max_cache_found = true;
#endif
				track_entry.max_cache = val;
			}
			break;
		default:
			nsmkv::ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}


// returns bytes read.  0 means EOF
uint64_t nsmkv::ReadTracks(nsmkv::MKVReader *reader, uint64_t size, nsmkv::Tracks &tracks)
{
	uint64_t total_bytes_read=0;
	while (size)
	{
		ebml_node node;
		uint64_t bytes_read = read_ebml_node(reader, &node);

		if (bytes_read == 0)
			return 0;

		// benski> checking bytes_read and node.size separately prevents possible integer overflow attack
		if (bytes_read > size)
			return 0;
		total_bytes_read+=bytes_read;
		size-=bytes_read;

		if (node.size > size)
			return 0;
		total_bytes_read+=node.size;
		size-=node.size;

		switch(node.id)
		{
		case mkv_tracks_trackentry:
			{
#ifdef WA_VALIDATE
				printf("  Track Entry\n");
#endif
				TrackEntry *track_entry = new TrackEntry;
				if (ReadTrackEntry(reader, node.size, *track_entry) == 0)
				{
					delete track_entry;
					return 0;
				}
				tracks.tracks.push_back(track_entry);
			}
			break;
		default:
			ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}

const nsmkv::TrackEntry *nsmkv::Tracks::EnumTrack(size_t i) const
{
	if (tracks.size() > i)
	{
		return tracks[i];
	}
	return 0;
}