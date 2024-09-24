#include "avi_rle_decoder.h"
#include "../Winamp/wa_ipc.h"
#include <limits.h>
#include "rle.h"
#include <intsafe.h>

AVIRLE *AVIRLE::CreateDecoder(nsavi::video_format *stream_format)
{
	if (stream_format->bits_per_pixel == 4)
		return 0;

	size_t bytes_per_pixel = stream_format->bits_per_pixel / 8U;
	if (bytes_per_pixel > 4)
		return 0;

	size_t image_size=0;
	if (SizeTMult(stream_format->width, stream_format->height, &image_size) != S_OK || SizeTMult(image_size, bytes_per_pixel, &image_size) != S_OK)
		return 0;

	void *video_frame = (uint8_t *)malloc(image_size);
	if (!video_frame)
		return 0;

	AVIRLE *decoder = new AVIRLE(video_frame, stream_format, image_size);
	if (!decoder)
	{
		free(video_frame);
		return 0;	
	}
	
	return decoder;
}


AVIRLE::AVIRLE(void *video_frame, nsavi::video_format *stream_format, size_t video_frame_size) : stream_format(stream_format), video_frame((uint8_t *)video_frame), video_frame_size(video_frame_size)
{
	memset(palette, 0, sizeof(palette));
	memcpy(palette, (uint8_t *)stream_format + 44, 1024);
	video_outputted=false;
	palette_retrieved=false;
}

int AVIRLE::GetPalette(RGB32 **palette)
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

int AVIRLE::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
{
	if (stream_format)
	{
		*x = stream_format->width;
		*y = stream_format->height;
		*flip = 1;
		switch(stream_format->bits_per_pixel)
		{
		case 4:
			*color_format = '8BGR';
			break;
		case 8:
			*color_format = '8BGR';
			break;
		case 16:
			*color_format = '555R';
			break;
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

static bool CheckOverflow(size_t total_size, int current_position, int read_size)
{
	if (read_size > (int)total_size) // check separate to avoid overflow
		return true;
	if (((int)total_size - read_size) < current_position)
		return true;
	return false;
}

int AVIRLE::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
{
	if (stream_format)
	{
		uint32_t bytes_per_pixel = stream_format->bits_per_pixel / 8;
		const uint8_t * const rle = (const uint8_t *)inputBuffer;
		if (bytes_per_pixel == 2)
		{
			RLE16(rle, inputBufferBytes, (uint16_t *)video_frame, video_frame_size, stream_format->width);
		}
		else if (bytes_per_pixel == 1)
		{
			RLE8(rle, inputBufferBytes, (uint8_t *)video_frame, video_frame_size, stream_format->width);
		}
		else
		{

			int input = 0;
			int output = 0;

			int next_line = output + bytes_per_pixel*stream_format->width;
			while (input < (int)inputBufferBytes && output < (int)video_frame_size)
			{
				if (CheckOverflow(inputBufferBytes, input, 2)) // we always read at least two bytes
					break;

				uint8_t b0 = rle[input++];
				if (b0)
				{
					if (CheckOverflow(inputBufferBytes, input, bytes_per_pixel))
						break;

					if (CheckOverflow(video_frame_size, output, b0*bytes_per_pixel))
						break;

					uint8_t pixel[4];
					memcpy(pixel, &rle[input], bytes_per_pixel);
					input += bytes_per_pixel;
					while (b0--)
					{
						memcpy(&video_frame[output], &pixel, bytes_per_pixel);
						output+=bytes_per_pixel;
					}
				}
				else
				{
					uint8_t b1 = rle[input++];
					if (b1 == 0)
					{
						output = next_line;
						next_line = output + bytes_per_pixel*stream_format->width;
					}
					else if (b1 == 1)
					{
						break;
					}
					else if (b1 == 2)
					{
						if (CheckOverflow(inputBufferBytes, input, 2)) 
							break;

						uint8_t p1 = rle[input++];
						uint8_t p2 = rle[input++];
						output += bytes_per_pixel*p1;
						output += bytes_per_pixel*p2*stream_format->width;
						next_line += bytes_per_pixel*p2*stream_format->width;
					}
					else
					{
						if (CheckOverflow(inputBufferBytes, input, b1*bytes_per_pixel))
							break;

						if (CheckOverflow(video_frame_size, output, b1*bytes_per_pixel))
							break;

						memcpy(&video_frame[output], &rle[input], b1*bytes_per_pixel);
						input += b1*bytes_per_pixel;
						output += b1*bytes_per_pixel;
						if (bytes_per_pixel == 1 && (b1 & 1))
							input++;
					}
				}
			}

		}
		video_outputted=false;
		return AVI_SUCCESS;
	}

	return AVI_FAILURE;
}

void AVIRLE::Flush()
{

}

int AVIRLE::GetPicture(void **data, void **decoder_data)
{
	if (!video_outputted && video_frame)
	{
		*data = video_frame;
		*decoder_data=0;
		video_outputted=true;
		return AVI_SUCCESS;
	}
	
	return AVI_FAILURE;
}

void AVIRLE::Close()
{
	free(video_frame);
	delete this;
}

#define CBCLASS AVIRLE
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
CB(GET_PICTURE, GetPicture)
CB(GET_PALETTE, GetPalette)
END_DISPATCH;
#undef CBCLASS
