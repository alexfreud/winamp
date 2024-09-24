#include "main.h"
#include "VideoDataConverter.h"
#include <cassert>

class YV12Converter : public VideoDataConverter
{
public:
	YV12Converter(int w, int h)
			: width(w), height(h)
	{
		yv12.y.rowBytes = width;
		yv12.v.rowBytes = width / 2;
		yv12.u.rowBytes = width / 2;
		vOffset = width*height;
		uOffset = width*height/4;
	}

	void *Convert(void *videoData)
	{
		yv12.y.baseAddr = (unsigned char*)videoData;
		yv12.v.baseAddr = yv12.y.baseAddr + vOffset;
		yv12.u.baseAddr = yv12.v.baseAddr + uOffset;

		return (void *)&yv12;
	}

	int width, height;
	int vOffset, uOffset;
	YV12_PLANES yv12;
};

VideoDataConverter *MakeConverter(VideoOutputStream *stream)
{
	switch (stream->FourCC())
	{
	case '21VY':
		return new YV12Converter(stream->DestinationWidth(), stream->DestinationHeight());
	default:
		return new VideoDataConverter;
	}
}