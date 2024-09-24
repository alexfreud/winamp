#include "mp4_mp4v_decoder.h"
#include "../winamp/wa_ipc.h"


MP4VMP4Decoder::MP4VMP4Decoder()
{
	
}

MP4VMP4Decoder::~MP4VMP4Decoder()
{
	
}

int MP4VMP4Decoder::Open(MP4FileHandle mp4_file, MP4TrackId mp4_track)
{
	HRESULT hr;
	VIDEOINFOHEADER header = {0, };

	header.bmiHeader.biHeight = MP4GetTrackVideoHeight(mp4_file, mp4_track);
	header.bmiHeader.biWidth = MP4GetTrackVideoWidth(mp4_file, mp4_track);
	header.bmiHeader.biCompression = 'v4pm';

	hr = decoder.Open(&header);
	if (FAILED(hr)) {
		return MP4_VIDEO_FAILURE;
	}

	uint8_t *buffer = 0;
	uint32_t buffer_size = 0;

	if (MP4GetTrackESConfiguration(mp4_file, mp4_track, &buffer, &buffer_size)
		&& buffer && buffer_size) {
			hr = decoder.Feed(buffer, buffer_size, 0);
			MP4Free(buffer);
			if (FAILED(hr)) {
				return MP4_VIDEO_FAILURE;
			}
	}

	return MP4_VIDEO_SUCCESS;
}

int MP4VMP4Decoder::GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio)
{
	UINT width, height;
	bool local_flip=false;
	if (SUCCEEDED(decoder.GetOutputFormat(&width, &height, &local_flip, aspect_ratio))) {
		*x = width;
		*y = height;
		*color_format = '21VY';
		return MP4_VIDEO_SUCCESS;
	}
	return MP4_VIDEO_FAILURE;		
}

int MP4VMP4Decoder::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp)
{
	HRESULT hr;
	hr=decoder.Feed(inputBuffer, inputBufferBytes, timestamp);
	return MP4_VIDEO_SUCCESS;
}

int MP4VMP4Decoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "mp4v");
}

void MP4VMP4Decoder::Flush()
{
	decoder.Flush();
}

int MP4VMP4Decoder::GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp)
{
	if (SUCCEEDED(decoder.GetFrame((YV12_PLANES **)data, decoder_data, timestamp))) {
		return MP4_VIDEO_SUCCESS;
	} else {
		return MP4_VIDEO_FAILURE;
	}
}

void MP4VMP4Decoder::FreePicture(void *data, void *decoder_data)
{
	decoder.FreeFrame((YV12_PLANES *)data, decoder_data);
}

void MP4VMP4Decoder::Close()
{
}

#define CBCLASS MP4VMP4Decoder
START_DISPATCH;
CB(MPEG4_VIDEO_OPEN, Open)
CB(MPEG4_VIDEO_GETOUTPUTFORMAT, GetOutputFormat)
CB(MPEG4_VIDEO_DECODE, DecodeSample)
CB(MPEG4_VIDEO_HANDLES_CODEC, CanHandleCodec)
VCB(MPEG4_VIDEO_FLUSH, Flush)
CB(MPEG4_VIDEO_GET_PICTURE, GetPicture)
VCB(MPEG4_VIDEO_FREE_PICTURE, FreePicture)
VCB(MPEG4_VIDEO_CLOSE, Close)
END_DISPATCH;
#undef CBCLASS
