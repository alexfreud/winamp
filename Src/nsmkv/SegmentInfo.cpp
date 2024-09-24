#include "SegmentInfo.h"
#include "read.h"
#include "global_elements.h"
#include <time.h>
#include <TCHAR.h>
#include <sys/stat.h> 

int FileExists(const char * filename);

// returns bytes read.  0 means EOF
uint64_t nsmkv::ReadSegmentInfo(nsmkv::MKVReader *reader, uint64_t size, nsmkv::SegmentInfo &segment_info)
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
		case mkv_segmentinfo_timecodescale:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Time Code Scale: %I64u\n", val);
				segment_info.time_code_scale_found = true;
#endif
				segment_info.time_code_scale = val;
			}
			break;
		case mkv_segmentinfo_muxingapp:
			{
				char *utf8 = 0;
				if (node.size && read_utf8(reader, node.size, &utf8) == 0)
					return 0;
				if (utf8)
#ifdef WA_VALIDATE
					printf("    Muxing App: %s\n", utf8);
				segment_info.muxing_app_found = true;
#endif
				segment_info.Own(segment_info.muxing_app, utf8);
			}
			break;
		case mkv_segmentinfo_writingapp:
			{
				char *utf8 = 0;
				if (node.size && read_utf8(reader, node.size, &utf8) == 0)
					return 0;
				if (utf8)
#ifdef WA_VALIDATE
					printf("    Writing App: %s\n", utf8);
				segment_info.writing_app_found = true;
#endif
				segment_info.Own(segment_info.writing_app, utf8);
			}
			break;
		case mkv_segmentinfo_duration:
			{
				double val;
				if (read_float(reader, node.size, &val) == 0)
					return 0;

#ifdef WA_VALIDATE
				printf("    Duration: %g\n", val);
				segment_info.duration_found = true;
#endif
				segment_info.duration = val;
			}
			break;
		case mkv_segmentinfo_dateutc:
			{
				uint64_t val;
				if (read_unsigned(reader, node.size, &val) == 0)
					return 0;

				segment_info.production_date = val;
				// value is in nanoseconds, relative to jan 01, 2001. ugh

				__time64_t val_time = mkv_date_as_time_t(val);
#ifdef WA_VALIDATE
				printf("    Date UTC: %s", _ctime64(&val_time));
				segment_info.production_date_found = true;
#endif
			}
			break;
		case mkv_segmentinfo_segmentuid:
			{
#ifdef WA_VALIDATE
				printf("    Segment UID: binary size %I64u\n", node.size);
				segment_info.segment_uid_found = true;
#endif
				if (node.size == 16)
				{
					size_t bytes_read;
					reader->Read(&segment_info.segment_uid, (size_t)node.size, &bytes_read);
					if (bytes_read != node.size)
						return 0;
				}
				else // bad size, let's just skip it
				{
					reader->Skip(node.size);
				}
			}
			break;
		case mkv_segmentinfo_prevuid:
			{
#ifdef WA_VALIDATE
				printf("    Previous UID: binary size %I64u\n", node.size);
				segment_info.prev_uid_found = true;
#endif
				if (node.size == 16)
				{
					size_t bytes_read;
					reader->Read(&segment_info.prev_uid, (size_t)node.size, &bytes_read);
					if (bytes_read != node.size)
						return 0;
				}
				else // bad size, let's just skip it
				{
					reader->Skip(node.size);
				}
			}
			break;
		case mkv_segmentinfo_nextuid:
			{
#ifdef WA_VALIDATE
				printf("    Next Segment UID: binary size %I64u\n", node.size);
				segment_info.next_uid_found = true;
#endif
				if (node.size == 16)
				{
					size_t bytes_read;
					reader->Read(&segment_info.next_uid, (size_t)node.size, &bytes_read);
					if (bytes_read != node.size)
						return 0;
				}
				else // bad size, let's just skip it
				{
					reader->Skip(node.size);
				}
			}
			break;
		case mkv_segmentinfo_title:
			{
				char *utf8 = 0;
				if (read_utf8(reader, node.size, &utf8) == 0)
					return 0;
				if (utf8)
#ifdef WA_VALIDATE
					printf("    Title: %s\n", utf8);
				segment_info.title_found = true;
#endif
				segment_info.Own(segment_info.title, utf8);
			}
			break;
		case mkv_segmentinfo_prevfilename:
			{
				char *utf8 = 0;
				if (read_utf8(reader, node.size, &utf8) == 0)
					return 0;
				if (utf8)
#ifdef WA_VALIDATE
					printf("    Previous Filename: %s\n", utf8);
				segment_info.prev_filename_found = true;
				if (FileExists(segment_info.prev_filename) != 0)
				{
					printf("****Specified previous filename not found");
				}
#endif
				segment_info.Own(segment_info.prev_filename,utf8);
			}
			break;
		case mkv_segmentinfo_nextfilename:
			{
				char *utf8 = 0;
				if (read_utf8(reader, node.size, &utf8) == 0)
					return 0;
				if (utf8)
#ifdef WA_VALIDATE
					printf("    Next Filename: %s\n", utf8);
				segment_info.next_filename_found = true;
				if (FileExists(segment_info.next_filename) != 0)
				{
					printf("****Specified next filename not found");
				}
#endif
				segment_info.Own(segment_info.next_filename, utf8);
			}
			break;

		default:
			nsmkv::ReadGlobal(reader, node.id, node.size);
		}
	}
	return total_bytes_read;
}

int nsmkv::SegmentInfo::GetDurationMilliseconds() const
{
	double nanoseconds = (double)time_code_scale * duration;
	double microseconds = nanoseconds / 1000.0;
	double milliseconds = microseconds / 1000.0;
	return (int)milliseconds;
}

uint64_t nsmkv::SegmentInfo::ConvertMillisecondsToTime(int milliseconds) const
{
	double time_code = (double)milliseconds * 1000000.0 / (double)time_code_scale;
	return (uint64_t)time_code;
}

int FileExists(const char * filename) { 
	int iStat; 
	struct _stat64 fileInfo; 

	// get the file attributes 
	return (iStat = _stat64(filename,&fileInfo)); 
}
