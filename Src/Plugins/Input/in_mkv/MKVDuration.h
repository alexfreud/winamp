#pragma once
#include "../nsmkv/header.h"
#include "../nsmkv/segmentinfo.h"
// parses only enough information to determine the file duration

class MKVDuration
{
public:
	MKVDuration();
	bool Open(const wchar_t *filename);
	int GetLengthMilliseconds();
	const char *GetTitle();
	int GetBitrate();
private:
	uint64_t ReadSegment(nsmkv::MKVReader *reader, uint64_t size);
	bool segment_info_found;
	nsmkv::Header header;
	nsmkv::SegmentInfo segment_info;
	uint64_t content_length;
};