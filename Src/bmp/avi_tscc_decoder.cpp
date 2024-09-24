#include "avi_tscc_decoder.h"
#include "avi_rle_decoder.h"
#include "avi_yuv_decoder.h"
#include "../Winamp/wa_ipc.h"
#include "rle.h"
#include <limits.h>
#include <intsafe.h>

AVITSCC *AVITSCC::CreateDecoder(nsavi::video_format *stream_format)
{
	size_t bytes_per_pixel = stream_format->bits_per_pixel / 8U;
	if (bytes_per_pixel > 4)
		return 0;

	size_t image_size=0;
	size_t pixel_size=0;
	size_t data_len=0;

	/* set an upper bound on width so we don't overflow when we multiply uint8_t * 4 * width */
	if (stream_format->width > (1 << 20))
		return 0;

	if (SizeTMult(stream_format->width, stream_format->height, &pixel_size) != S_OK || SizeTMult(pixel_size, bytes_per_pixel, &image_size) != S_OK)
		return 0;

	// calculate worst-case data length (3 * pixel_size / 255 + image_size)
	if (SizeTMult(pixel_size, 3, &data_len) != S_OK)
		return 0;
	pixel_size /= 255;
	if (SizeTAdd(pixel_size, data_len, &data_len) != S_OK)
		return 0;

	void *video_frame = (uint8_t *)malloc(image_size);
	if (!video_frame)
		return 0;

	// upper bound for decompressed data size
	
	void *data = malloc(data_len);
	if (!data)
	{
		free(video_frame);
		return 0;
	}

	AVITSCC *decoder = new AVITSCC(video_frame, image_size, data, data_len, stream_format);
	if (!decoder)
	{
		free(video_frame);
		free(data);
		return 0;	
	}
	
	return decoder;
}

AVITSCC::AVITSCC(void *video_frame, size_t video_frame_size, void *data, size_t data_len, nsavi::video_format *stream_format) : stream_format(stream_format), video_frame_size(video_frame_size), video_frame((uint8_t *)video_frame), data((uint8_t *)data), data_len(data_len)
{
	video_outputted=false;

	zlib_stream.next_in = Z_NULL;
	zlib_stream.avail_in = Z_NULL;
	zlib_stream.next_out = Z_NULL;
	zlib_stream.avail_out = Z_NULL;
	zlib_stream.zalloc = (alloc_func)0;
	zlib_stream.zfree = (free_func)0;
	zlib_stream.opaque = 0;
	inflateInit(&zlib_stream);
}

int AVITSCC::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
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


static bool BoundsCheckX(uint8_t delta_x, size_t bytes_per_pixel, size_t video_frame_size, size_t output_pointer)
{
	if ((size_t)delta_x*bytes_per_pixel >= (video_frame_size - output_pointer))
			return false;
	return true;
}

static bool BoundsCheckY(uint8_t delta_y, size_t bytes_per_pixel, size_t width, size_t video_frame_size, size_t output_pointer)
{
	if ((size_t)delta_y*bytes_per_pixel*width >= (video_frame_size - output_pointer))
			return false;
	return true;
}

int AVITSCC::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
{
	if (stream_format)
	{
		if (inflateReset(&zlib_stream) != Z_OK)
			return AVI_FAILURE;

		size_t bytes_per_pixel = stream_format->bits_per_pixel / 8U;

		zlib_stream.next_in = (Bytef *)inputBuffer;
    zlib_stream.avail_in = (uInt)inputBufferBytes;
    zlib_stream.next_out = data;
    zlib_stream.avail_out = (uInt)data_len;

		int ret = inflate(&zlib_stream, Z_FINISH);

		if (ret == Z_OK || ret == Z_STREAM_END)
		{
			if (bytes_per_pixel == 2)
			{
				 RLE16(data, data_len, (uint16_t *)video_frame, video_frame_size, stream_format->width);
			}
			else if (bytes_per_pixel == 1)
			{
				RLE8(data, data_len, (uint8_t *)video_frame, video_frame_size, stream_format->width);
			}
			else
			{
			const uint8_t * const rle = data;
			int input = 0;
			size_t output = 0;
			int next_line = (int)output + (int)bytes_per_pixel*stream_format->width;
			for (;;)
			{
				uint8_t b0 = rle[input++];
				if (b0)
				{
					uint8_t pixel[4] = {0};
					memcpy(pixel, &rle[input], bytes_per_pixel);
					input += (int)bytes_per_pixel;

					if (!BoundsCheckX(b0, bytes_per_pixel, video_frame_size, output))
						return AVI_FAILURE;

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
						if (next_line > (int)video_frame_size)
							return AVI_FAILURE;

						output = next_line;
						next_line = (int)output + (int)bytes_per_pixel*stream_format->width;
					}
					else if (b1 == 1)
					{
						break;
					}
					else if (b1 == 2)
					{
						uint8_t p1 = rle[input++];
						uint8_t p2 = rle[input++];
						if (!BoundsCheckX(p1, bytes_per_pixel, video_frame_size, output))
							return AVI_FAILURE;

						output += bytes_per_pixel*p1;

						if (!BoundsCheckY(p2, bytes_per_pixel, stream_format->width, video_frame_size, output))
							return AVI_FAILURE;

						output += bytes_per_pixel*p2*stream_format->width;
						next_line += (int)bytes_per_pixel*p2*stream_format->width;
					}
					else
					{
						if (!BoundsCheckX(b1, bytes_per_pixel, video_frame_size, output))
							return AVI_FAILURE;

						memcpy(&video_frame[output], &rle[input], b1*bytes_per_pixel);
						input += b1* (int)bytes_per_pixel;
						output += b1*bytes_per_pixel;
						if (bytes_per_pixel == 1 && (b1 & 1))
							input++;
					}
				}
			}
			}
		}
		else if (ret != Z_DATA_ERROR)
		{
			return AVI_FAILURE;
		}

		video_outputted=false;
		return AVI_SUCCESS;
	}

	return AVI_FAILURE;
}

void AVITSCC::Flush()
{

}

int AVITSCC::GetPicture(void **data, void **decoder_data)
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

void AVITSCC::Close()
{
	free(video_frame);
	free(data);
	inflateEnd(&zlib_stream);
	delete this;
}

#define CBCLASS AVITSCC
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
CB(GET_PICTURE, GetPicture)
END_DISPATCH;
#undef CBCLASS
