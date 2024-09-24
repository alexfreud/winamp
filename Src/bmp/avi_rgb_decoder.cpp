#include "avi_rgb_decoder.h"
#include "../Winamp/wa_ipc.h"
#include <bfc/error.h>
#include <limits.h>
#include <intsafe.h>

int BMP_GetMallocSize(int32_t height, int32_t width, int32_t bits_per_pixel, size_t *out_frame_bytes)
{
	if (height < 0 || width < 0)
	{
		return NErr_Error;
	}

	uint64_t frame_size = (uint64_t)height * (uint64_t)width;
	if (frame_size > SIZE_MAX)
		return NErr_IntegerOverflow;

	uint64_t frame_bytes = frame_size * (uint64_t)bits_per_pixel;
	if (frame_bytes > SIZE_MAX || frame_bytes < frame_size)
		return NErr_IntegerOverflow;

	*out_frame_bytes = (size_t)(frame_bytes / 8);
	return NErr_Success;
}

AVIRGB *AVIRGB::CreateDecoder(nsavi::video_format *stream_format)
{

	AVIRGB *decoder = new AVIRGB(stream_format);
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


AVIRGB::AVIRGB(nsavi::video_format *stream_format) : stream_format(stream_format)
{
	palette_retrieved=false;
	video_frame=0;
	video_frame_size_bytes=0;
	
	
	if (stream_format->size_bytes == 1064)
	{
		memset(palette, 0, sizeof(palette));
		memcpy(palette, (uint8_t *)stream_format + 44, 1024);
	}
	o=false;
}

AVIRGB::~AVIRGB()
{
	free(video_frame);
}

int AVIRGB::Initialize()
{
	size_t frame_bytes;
	int ret = BMP_GetMallocSize(stream_format->height, stream_format->width, stream_format->bits_per_pixel, &frame_bytes);
	if (ret != NErr_Success)
		return ret;

	video_frame=malloc(frame_bytes);
	if (!video_frame)
		return NErr_OutOfMemory;

	video_frame_size_bytes = frame_bytes;

	return NErr_Success;
}

int AVIRGB::GetPalette(RGB32 **palette)
{
	if (!palette_retrieved)
	{
		*palette = (RGB32 *)(this->palette);
		palette_retrieved=true;
		return AVI_SUCCESS;
	}
	else
	{
		return AVI_FAILURE;
	}

}

int AVIRGB::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
{
	if (stream_format)
	{
		*x = stream_format->width;
		*y = stream_format->height;
		*flip = 1;
		switch(stream_format->bits_per_pixel)
		{
		case 8:
			*color_format = '8BGR';
			break;
			// TODO:
		//case 16:
			//*color_format = '8GBR';
		case 24:
			*color_format = '42GR';
			break;
			case 32:
			*color_format = '23GR';
			break;
			default:
				return AVI_FAILURE;
		}
		return AVI_SUCCESS;
	}
		
	return AVI_FAILURE;
}

int AVIRGB::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
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

void AVIRGB::Flush()
{

}

int AVIRGB::GetPicture(void **data, void **decoder_data)
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

void AVIRGB::Close()
{
	delete this;
}

#define CBCLASS AVIRGB
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
CB(GET_PICTURE, GetPicture)
CB(GET_PALETTE, GetPalette)
END_DISPATCH;
#undef CBCLASS
