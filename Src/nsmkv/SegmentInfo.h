#pragma once
#include "mkv_date.h"
#include "mkv_reader.h"
#include <bfc/platform/guid.h>
/*
Time Scale: 986946
Muxing App: libebml v0.7.7 + libmatroska v0.8.1
Writing App: mkvmerge v2.0.2 ('You're My Flame') built on Sep 20 2007 09:35:09
Duration: 60257
Date UTC: Sun Nov 18 20:23:18 2007
Segment UID: binary size 16
*/

const uint32_t mkv_segment_segmentinfo = 0x549a966;
const uint32_t mkv_segmentinfo_timecodescale = 0xad7b1;
const uint32_t mkv_segmentinfo_muxingapp=0xd80;
const uint32_t mkv_segmentinfo_writingapp=0x1741;
const uint32_t mkv_segmentinfo_duration=0x489;
const uint32_t mkv_segmentinfo_dateutc=0x461;
const uint32_t mkv_segmentinfo_segmentuid=0x33a4;
const uint32_t mkv_segmentinfo_nextuid=0x1eb923;
const uint32_t mkv_segmentinfo_prevuid=0x1cb923;
const uint32_t mkv_segmentinfo_nextfilename=0x1e83bb;
const uint32_t mkv_segmentinfo_prevfilename=0x1c83ab;
const uint32_t mkv_segmentinfo_title=0x3ba9;

namespace nsmkv
{
	class SegmentInfo
	{
	public:
		SegmentInfo() :
		  time_code_scale(1000000),
			muxing_app(0),
			writing_app(0),
			duration(0),
			production_date(0),
			segment_uid(INVALID_GUID),
			next_uid(INVALID_GUID),
			prev_uid(INVALID_GUID),
			next_filename(0),
			prev_filename(0),
			title(0)
#ifdef WA_VALIDATE
			,
			time_code_scale_found(false),
			muxing_app_found(false),
			writing_app_found(false),
			duration_found(false),
			production_date_found(false),
			segment_uid_found(false),
			next_uid_found(false),
			prev_uid_found(false),
			next_filename_found(false),
			prev_filename_found(false),
			title_found(false)
#endif

		{
		}
		~SegmentInfo()
		{
			free(muxing_app);
			free(writing_app);
			free(title);
		}
		void Own(char *&field, char *value)
		{
			if (field)
				free(field);
			field = value;
		}

		int GetDurationMilliseconds() const;
		uint64_t ConvertMillisecondsToTime(int milliseconds) const;
		uint64_t time_code_scale;
		char *muxing_app;
		char *writing_app;
		char *title;
		double duration;
		mkv_date_t production_date;
		GUID segment_uid;
		GUID prev_uid;
		GUID next_uid;
		char *prev_filename;
		char *next_filename;

#ifdef WA_VALIDATE
		bool segment_uid_found;
		bool prev_uid_found;
		bool next_uid_found;
		bool prev_filename_found;
		bool next_filename_found;
		bool time_code_scale_found;
		bool duration_found;
		bool muxing_app_found;
		bool writing_app_found;
		bool production_date_found;
		bool title_found;


#endif

	};
	
	uint64_t ReadSegmentInfo(nsmkv::MKVReader *reader, uint64_t size, nsmkv::SegmentInfo &segment_info);
}