#include "Cues.h"
#include "read.h"
#include "global_elements.h"

nsmkv::CuePoint *nsmkv::Cues::GetCuePoint(uint64_t when, uint64_t current_time, int seek_direction)
{
	CuePoint *last=0;
	CuePoints::iterator itr;
	for (itr=cue_points.begin(); itr!=cue_points.end();itr++)
	{
		CuePoint *cue_point = itr->second;
		if (cue_point->cue_time == when)
		{
			if (seek_direction != SEEK_FORWARD || current_time < cue_point->cue_time)
				return cue_point;
		}
		else if (cue_point->cue_time > when)
		{
			if (last)
			{
				if (seek_direction != SEEK_FORWARD || current_time < last->cue_time)
					return last;
			}
			else
				return cue_point;
		}

		last = cue_point;
	}
	return last; // will be 0 if we don't have any cue points, which is what we want.
}

nsmkv::CueTrackPosition *nsmkv::CuePoint::GetPosition(uint64_t track)
{
	CueTrackPositions::iterator found = cue_track_positions.find(track);
	if (found != cue_track_positions.end())
		return found->second;
	else
		return 0;
}

// returns bytes read.  0 means EOF
static uint64_t ReadCueTrackPositions(nsmkv::MKVReader *reader, uint64_t size, nsmkv::CueTrackPosition &track_position)
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
		case mkv_cuetrackpositions_cuetrack:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

				track_position.track = val;
#ifdef WA_VALIDATE
				printf("      Cue Track: %I64u\n", val);
				track_position.track_found = true;
#endif
			}
			break;
		case mkv_cuetrackpositions_cueclusterposition:
			{			
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

				track_position.cluster_position = val;
#ifdef WA_VALIDATE
				printf("      Cue Cluster Position: %I64u\n", val);
				track_position.cluster_position_found = true;
#endif
			}
			break;
		default:
			nsmkv::ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}

// returns bytes read.  0 means EOF
static uint64_t ReadCuePoint(nsmkv::MKVReader *reader, uint64_t size, nsmkv::CuePoint &cue_point)
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
		case mkv_cuepoint_cuetime:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

				cue_point.cue_time = val;

#ifdef WA_VALIDATE
				printf("    Cue Time: %I64u\n", val);
				cue_point.cue_time_found = true;
#endif
			}
			break;
		case mkv_cuepoint_cuetrackpositions:
			{
#ifdef WA_VALIDATE
				printf("    Cue Track Positions\n");
#endif
				nsmkv::CueTrackPosition *track_position = new nsmkv::CueTrackPosition;
				if (ReadCueTrackPositions(reader, node.size, *track_position) == 0)
				{
					delete track_position;
					return 0;
				}
				cue_point.cue_track_positions[track_position->track] = track_position;
			}
			break;
		default:
			nsmkv::ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}

// returns bytes read.  0 means EOF
uint64_t nsmkv::ReadCues(nsmkv::MKVReader *reader, uint64_t size, nsmkv::Cues &cues)
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
		case mkv_cues_cuepoint:
			{
#ifdef WA_VALIDATE
				printf("  Cue Point\n");
#endif
				CuePoint *cue_point = new CuePoint;
				if (ReadCuePoint(reader, node.size, *cue_point) == 0)
				{
					delete cue_point;
					return 0;
				}
				cues.cue_points[cue_point->cue_time] = cue_point;				
			}
			break;
		default:
			ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}

