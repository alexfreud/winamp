#pragma once
#include "../nsmkv/header.h"
#include "../nsmkv/segmentinfo.h"
#include "../nsmkv/tracks.h"
// parses only enough information to use in GetExtendedFileInfo

class MKVInfo
{
public:
	MKVInfo();
	bool Open(const wchar_t *filename);
	int GetLengthMilliseconds();
	const char *GetTitle();
	int GetBitrate();
	bool GetHeight(int &height);
	bool GetWidth(int &width);
	const nsmkv::Tracks *GetTracks();
private:
	uint64_t ReadSegment(nsmkv::MKVReader *reader, uint64_t size);
	bool segment_info_found;
	bool tracks_found;
	nsmkv::Header header;
	nsmkv::SegmentInfo segment_info;
	nsmkv::Tracks tracks;
	uint64_t content_length;
};