#include "MKVInfo.h"
#include "../nsmkv/global_elements.h"
#include "../nsmkv/read.h"
#include "../nsmkv/segment.h"
#include "../nsmkv/tracks.h"
#include "../nsmkv/file_mkv_reader.h"

MKVInfo::MKVInfo()
{
	segment_info_found = false;
	content_length = 0;
	tracks_found = false;
}

bool MKVInfo::Open(const wchar_t *filename)
{
	FILE *f = _wfopen(filename, L"rb");
	if (!f)
		return false;

	MKVReaderFILE reader(f);

	content_length = reader.GetContentLength();

	ebml_node node;
	while (!segment_info_found)
	{
		if (read_ebml_node(&reader, &node) == 0)
			break;

		switch(node.id)
		{
		case mkv_header:
			if (nsmkv::ReadHeader(&reader, node.size, header) == 0)
			{
				return false;
			}
			break;
		case mkv_segment:
			if (ReadSegment(&reader, node.size) == 0)
			{
				return false;
			}
			break;
		default:
			nsmkv::SkipNode(&reader, node.id, node.size);

		}
	}

	return segment_info_found;
}

uint64_t MKVInfo::ReadSegment(nsmkv::MKVReader *reader, uint64_t size)
{
	uint64_t total_bytes_read=0;
	while (size && (!segment_info_found || !tracks_found))
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
		case mkv_segment_segmentinfo:
			{
				if (ReadSegmentInfo(reader, node.size, segment_info) == 0)
					return 0;
				segment_info_found=true;
			}
			break;
		case mkv_segment_tracks:
			{
				if (ReadTracks(reader, node.size, tracks) == 0)
					return 0;
				tracks_found=true;
			}
			break;

		default:
			if (nsmkv::SkipNode(reader, node.id, node.size) == 0)
				return 0;
		}
	}
	return total_bytes_read;
}

int MKVInfo::GetLengthMilliseconds()
{
	if (!segment_info_found)
		return -1000;
	else
		return segment_info.GetDurationMilliseconds();
}

const char *MKVInfo::GetTitle()
{
	return segment_info.title;
}

int MKVInfo::GetBitrate()
{
	if (segment_info_found)
	{
		int time_ms = segment_info.GetDurationMilliseconds();
		if (time_ms)
			return (int) (8ULL * content_length / (uint64_t)time_ms);
	}
	return 0;
}

bool MKVInfo::GetHeight(int &height)
{
	if (tracks_found)
	{
		size_t i=0;
		const nsmkv::TrackEntry *track=0;
		while (track = tracks.EnumTrack(i++))
		{
			if (track->track_type == mkv_track_type_video)
			{
				height = (int)track->video.pixel_height;
				return true;
			}
		}
	}
	return false;
}


bool MKVInfo::GetWidth(int &width)
{
	if (tracks_found)
	{
		size_t i=0;
		const nsmkv::TrackEntry *track=0;
		while (track = tracks.EnumTrack(i++))
		{
			if (track->track_type == mkv_track_type_video)
			{
				width = (int)track->video.pixel_width;
				return true;
			}
		}
	}
	return false;
}

const nsmkv::Tracks *MKVInfo::GetTracks()
{
	if (tracks_found)
		return &tracks;
	else
		return 0;
}