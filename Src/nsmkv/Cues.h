#pragma once
#include <bfc/platform/types.h>
#include "mkv_reader.h"
#include <map>

const uint32_t mkv_segment_cues = 0xc53bb6b;
const uint32_t mkv_cues_cuepoint = 0x3b;
const uint32_t mkv_cuepoint_cuetime=0x33;
const uint32_t mkv_cuepoint_cuetrackpositions = 0x37;
const uint32_t mkv_cuetrackpositions_cuetrack= 0x77;
const uint32_t mkv_cuetrackpositions_cueclusterposition=0x71;

namespace nsmkv
{

	class CueTrackPosition
	{
	public:
		CueTrackPosition()
#ifdef WA_VALIDATE
		:
		track_found(false),
		cluster_position_found(false)
//		block_number_found(false)
#endif
		{
			track=0;
			cluster_position=0;
			block_number=0;
		}
		uint64_t track;
		uint64_t cluster_position;
		uint64_t block_number;

#ifdef WA_VALIDATE
		bool track_found;
		bool cluster_position_found;
//		bool block_number_found;
#endif
	};

	class CuePoint
	{
	public:
		CuePoint()
#ifdef WA_VALIDATE
			:
			cue_time_found(false)
#endif
		{
			cue_time = 0;
		}
		CueTrackPosition *GetPosition(uint64_t track);
		uint64_t cue_time;
		typedef std::map<uint64_t, CueTrackPosition*> CueTrackPositions; // keyed on track number
		CueTrackPositions cue_track_positions;
#ifdef WA_VALIDATE
		bool cue_time_found;
#endif
	};

	class Cues
	{
	public:
	enum
	{
		SEEK_WHATEVER=0,
		SEEK_FORWARD=1,
		SEEK_BACKWARD=2,
	};
		CuePoint *GetCuePoint(uint64_t when, uint64_t current_time=0, int seek_direction=SEEK_WHATEVER);
		typedef std::map<uint64_t, CuePoint*> CuePoints; // use Map on cue_time to ensure order
		CuePoints cue_points;
	};
	uint64_t ReadCues(MKVReader *reader, uint64_t size, nsmkv::Cues &cues);
}
