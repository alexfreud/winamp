#include "mp4_jpeg_decoder.h"
#include "api__jpeg.h"
#include <api/service/waservicefactory.h>

MP4JPEGDecoder::MP4JPEGDecoder()
{
	jpegLoader=0;
	width=0;
	height=0;
	decoded_image = 0;
}

int MP4JPEGDecoder::Open(MP4FileHandle mp4_file, MP4TrackId mp4_track)
{
	// load JPEG loader
	jpegLoader = new JpgLoad;

	if (jpegLoader)
		return MP4_VIDEO_SUCCESS;
	else
		return MP4_VIDEO_FAILURE;

}

void MP4JPEGDecoder::Close()
{
	delete jpegLoader;
	delete this;
}

int MP4JPEGDecoder::GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio)
{
	if (!height || !width)
		return MP4_VIDEO_FAILURE;

	*x = width;
	*y = height;
	*color_format = '23GR'; // RGB32
	return MP4_VIDEO_SUCCESS;
}

int MP4JPEGDecoder::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp)
{
	bool change_in_size=false;
	if (decoded_image)
		WASABI_API_MEMMGR->sysFree(decoded_image);
	int decode_width, decode_height;
	decoded_image = jpegLoader->loadImage(inputBuffer, (int)inputBufferBytes, &decode_width, &decode_height);
	if (!decoded_image)
		return MP4_VIDEO_FAILURE;
	if (width && decode_width != width // if we have a different width from last time
		|| height && decode_height != height) 
		change_in_size = true; 

	width = decode_width;
	height = decode_height;
	return change_in_size?MP4_VIDEO_OUTPUT_FORMAT_CHANGED:MP4_VIDEO_SUCCESS;
}

int MP4JPEGDecoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "jpeg");
}

int MP4JPEGDecoder::GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp)
{
	if (!decoded_image)
		return MP4_VIDEO_FAILURE;

	*data = decoded_image;
	*decoder_data = 0;
	decoded_image = 0; // wipe our hands clean of it so we don't double free
	return MP4_VIDEO_SUCCESS;
}

void MP4JPEGDecoder::FreePicture(void *data, void *decoder_data)
{
	WASABI_API_MEMMGR->sysFree(data);
}

#define CBCLASS MP4JPEGDecoder
START_DISPATCH;
CB(MPEG4_VIDEO_OPEN, Open)
CB(MPEG4_VIDEO_GETOUTPUTFORMAT, GetOutputFormat)
CB(MPEG4_VIDEO_DECODE, DecodeSample)
CB(MPEG4_VIDEO_HANDLES_CODEC, CanHandleCodec)
CB(MPEG4_VIDEO_GET_PICTURE, GetPicture)
VCB(MPEG4_VIDEO_FREE_PICTURE, FreePicture)
END_DISPATCH;
#undef CBCLASS

