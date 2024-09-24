#ifndef NULLSOFT_MPEG4VIDEO_H
#define NULLSOFT_MPEG4VIDEO_H
#include "../external_dependencies/libmp4v2/mp4.h"
#include <bfc/dispatch.h>
#include <api/service/services.h>

enum
{
    MP4_VIDEO_SUCCESS = 0,
    MP4_VIDEO_FAILURE = 1,
		MP4_VIDEO_OUTPUT_FORMAT_CHANGED = -1, // succeeded, but call GetOutputFormat again!
		MP4_VIDEO_AGAIN = -2,
};

class MP4VideoDecoder : public Dispatchable
{
protected:
	MP4VideoDecoder() {}
	~MP4VideoDecoder() {}

public:
	static FOURCC getServiceType() { return WaSvc::MP4VIDEODECODER; } 
	int Open(MP4FileHandle mp4_file, MP4TrackId mp4_track);
	int GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp);
	void Flush();
	void Close();
	int CanHandleCodec(const char *codecName); // return 0 for no, anything else for yes
	int GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp);
	void FreePicture(void *data, void *decoder_data);
	void HurryUp(int state);

	DISPATCH_CODES
	{
    MPEG4_VIDEO_OPEN = 11,
    MPEG4_VIDEO_GETOUTPUTFORMAT = 21,
    MPEG4_VIDEO_DECODE = 31,
    MPEG4_VIDEO_FLUSH = 40,
    MPEG4_VIDEO_CLOSE = 50,
    MPEG4_VIDEO_HANDLES_CODEC = 60,
		MPEG4_VIDEO_GET_PICTURE = 70,
		MPEG4_VIDEO_FREE_PICTURE = 80,
		MPEG4_VIDEO_HURRY_UP = 90,
	};

};

inline int MP4VideoDecoder::Open(MP4FileHandle mp4_file, MP4TrackId mp4_track)
{
	return _call(MPEG4_VIDEO_OPEN, (int)MP4_VIDEO_FAILURE, mp4_file, mp4_track);
}

inline int MP4VideoDecoder::GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio)
{
	return _call(MPEG4_VIDEO_GETOUTPUTFORMAT, (int)MP4_VIDEO_FAILURE, x, y, color_format, aspect_ratio);
}

inline int MP4VideoDecoder::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp)
{
	return _call(MPEG4_VIDEO_DECODE, (int)MP4_VIDEO_FAILURE, inputBuffer, inputBufferBytes, timestamp);
}

inline void MP4VideoDecoder::Flush()
{
	_voidcall(MPEG4_VIDEO_FLUSH);
}

inline void MP4VideoDecoder::Close()
{
	_voidcall(MPEG4_VIDEO_CLOSE);
}

inline int MP4VideoDecoder::CanHandleCodec(const char *codecName)
{
	return _call(MPEG4_VIDEO_HANDLES_CODEC, (int)0, codecName);
}

inline int MP4VideoDecoder::GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp)
{
	return _call(MPEG4_VIDEO_GET_PICTURE, (int)MP4_VIDEO_FAILURE, data, decoder_data, timestamp);
}

inline void MP4VideoDecoder::FreePicture(void *data, void *decoder_data)
{
		_voidcall(MPEG4_VIDEO_FREE_PICTURE, data, decoder_data);
}

inline void MP4VideoDecoder::HurryUp(int state)
{
		_voidcall(MPEG4_VIDEO_HURRY_UP, state);
}
#endif