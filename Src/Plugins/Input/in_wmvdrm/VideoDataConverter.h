#ifndef NULLSOFT_VIDEODATACONVERTERH
#define NULLSOFT_VIDEODATACONVERTERH

#include "OutputStream.h"
class VideoDataConverter
{
public:
	virtual void *Convert(void *videoData)
	{
		return videoData;
	}
};



VideoDataConverter *MakeConverter(VideoOutputStream *stream);

#endif
