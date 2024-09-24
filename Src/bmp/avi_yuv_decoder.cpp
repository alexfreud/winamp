#include "avi_yuv_decoder.h"
#include "../Winamp/wa_ipc.h"
#include <limits.h>
#include <bfc/error.h>
#include <intsafe.h>

int BMP_GetMallocSize(int32_t height, int32_t width, int32_t bits_per_pixel, size_t *out_frame_bytes);

AVIYUV *AVIYUV::CreateDecoder(nsavi::video_format *stream_format)
{
	AVIYUV *decoder = new AVIYUV( stream_format);
	if (!decoder)
	{
		return 0;	
	}

	if (decoder->Initialize() != NErr_Success)
	{
		delete decoder;
		return 0;
	}
	return decoder;
}


AVIYUV::AVIYUV(nsavi::video_format *stream_format) : stream_format(stream_format)
{
	video_frame=0;
	video_frame_size_bytes=0;
	o=false;
}

AVIYUV::~AVIYUV()
{
	free(video_frame);
}


int AVIYUV::Initialize()
{
	size_t frame_bytes;
	int ret = BMP_GetMallocSize(stream_format->height, stream_format->width, 16, &frame_bytes);
	if (ret != NErr_Success)
		return ret;

	video_frame=malloc(frame_bytes);
	if (!video_frame)
		return NErr_OutOfMemory;

	video_frame_size_bytes = frame_bytes;
	return NErr_Success;
}

int AVIYUV::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
{
	if (stream_format)
	{
		*x = stream_format->width;
		*y = stream_format->height;
		//*flip = 1;
		*color_format = stream_format->compression;
		return AVI_SUCCESS;
	}
		
	return AVI_FAILURE;
}

int AVIYUV::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
{
	if (stream_format)
	{
		if (video_frame_size_bytes < inputBufferBytes)
			return AVI_FAILURE;
		memcpy(video_frame, inputBuffer, inputBufferBytes);
		//video_frame = inputBuffer; // heh
		o=true;
		return AVI_SUCCESS;
	}

	return AVI_FAILURE;
}

void AVIYUV::Flush()
{

}

int AVIYUV::GetPicture(void **data, void **decoder_data)
{
	if (o && video_frame)
	{
		*data =(void *) video_frame;
		*decoder_data=0;
		//video_frame=0;
		o=false;
		//video_outputted=true;
		return AVI_SUCCESS;
	}
	
	return AVI_FAILURE;
}

void AVIYUV::Close()
{
	delete this;
}


#define CBCLASS AVIYUV
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
CB(GET_PICTURE, GetPicture)
END_DISPATCH;
#undef CBCLASS
