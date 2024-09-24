#ifndef NULLSOFT_FLVVIDEOHEADER_H
#define NULLSOFT_FLVVIDEOHEADER_H

namespace FLV
{
	const int VIDEO_FORMAT_JPEG = 1;
	const int VIDEO_FORMAT_SORENSON = 2; // H.263
	const int VIDEO_FORMAT_SCREEN = 3;
	const int VIDEO_FORMAT_VP6 = 4;
	const int VIDEO_FORMAT_VP62 = 5;
	const int VIDEO_FORMAT_SCREEN_V2 = 6;
	const int VIDEO_FORMAT_AVC = 7; // MPEG-4 Part 10

	const int VIDEO_FRAMETYPE_KEYFRAME = 1;
	const int VIDEO_FRAMETYPE_IFRAME = 2;
	const int VIDEO_FRAMETYPE_IFRAME_DISPOSABLE = 3;
	const int VIDEO_FRAMETYPE_GENERATED = 4;
	const int VIDEO_FRAMETYPE_INFO = 5;
};


class FLVVideoHeader
{
public:
	bool Read(unsigned __int8 *data, size_t size); // size must be >=1, returns "true" if this was a valid header

	// attributes, consider these read-only
	int format;
	int frameType;
};

#endif