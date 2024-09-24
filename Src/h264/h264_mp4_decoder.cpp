#include "h264_mp4_decoder.h"
#include "../winamp/wa_ipc.h"
#include <Mferror.h>
uint32_t Read24(const uint8_t *data)
{
	// ugh, 24bit size
	uint32_t this_size=0;
	uint8_t *this_size_p = (uint8_t *)&this_size;
	this_size_p[0] = data[2];
	this_size_p[1] = data[1];
	this_size_p[2] = data[0];
	return this_size;
}

uint32_t GetNALUSize(uint64_t nalu_size_bytes, const uint8_t *h264_data, size_t data_len)
{
	if ((data_len) < (nalu_size_bytes))
		return 0;

	switch(nalu_size_bytes)
	{
	case 1:
		return *h264_data;
	case 2:
		{
			return (h264_data[0] << 8) | h264_data[1];

		}
	case 3:
		{
			return Read24(h264_data);
		}
	case 4:
		{
			uint32_t this_size = *(uint32_t *)h264_data;
			this_size = htonl(this_size);
			return this_size;
		}
	}
	return 0;
}

H264MP4Decoder::H264MP4Decoder()
{
	nalu_size_bytes=0;
	width=0;
	height=0;
}

H264MP4Decoder::~H264MP4Decoder()
{
	for (size_t i=0;i<buffered_frames.size();i++) {
		nullsoft_h264_frame_data frame_data = buffered_frames[i];
		decoder.FreeFrame((YV12_PLANES *)frame_data.data, frame_data.decoder_data);
	}
}

int H264MP4Decoder::Open(MP4FileHandle mp4_file, MP4TrackId mp4_track)
{
	this->mp4_file=mp4_file;
	this->mp4_track=mp4_track;
	decoder.Open();
	// TODO error checking
	uint8_t **seqHeaders = 0, **pictHeaders = 0;
	uint32_t *seqHeadersSize = 0, *pictHeadersSize = 0;

	__try
	{
		MP4GetTrackH264SeqPictHeaders(mp4_file, mp4_track,
			&seqHeaders, &seqHeadersSize,
			&pictHeaders, &pictHeadersSize);

		if (seqHeadersSize)
		{
			for (uint32_t i = 0; seqHeadersSize[i] != 0; i++) 
			{
				decoder.Feed(seqHeaders[i], seqHeadersSize[i], 0);
				MP4Free(seqHeaders[i]);
			}
		}
		MP4Free(seqHeadersSize);

		if (pictHeadersSize)
		{
			for (uint32_t i = 0; pictHeadersSize[i] != 0; i++) 
			{
				decoder.Feed(pictHeaders[i], pictHeadersSize[i], 0);
				MP4Free(pictHeaders[i]);
			}
		}
		MP4Free(pictHeadersSize);
		
		MP4GetTrackH264LengthSize(mp4_file, mp4_track, &nalu_size_bytes);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return MP4_VIDEO_FAILURE;
	}

	return MP4_VIDEO_SUCCESS;
}

int H264MP4Decoder::GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio)
{
	bool flip;

	if (SUCCEEDED(decoder.GetOutputFormat(&width, &height, &flip, aspect_ratio))) {
		*x = width;
		*y = height;
		*color_format = htonl('YV12');
		return MP4_VIDEO_SUCCESS;
	}
	return MP4_VIDEO_FAILURE;		
}


int H264MP4Decoder::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp)
{
	const uint8_t *h264_data = (const uint8_t *)inputBuffer;
	while (inputBufferBytes)
	{
		uint32_t this_size =GetNALUSize(nalu_size_bytes, h264_data, inputBufferBytes);
		if (this_size == 0)
			return MP4_VIDEO_FAILURE;

		inputBufferBytes-=nalu_size_bytes;
		h264_data+=nalu_size_bytes;
		if (this_size > inputBufferBytes)
			return MP4_VIDEO_FAILURE;

		for (;;) {
			uint64_t hundrednanos = MP4ConvertFromTrackTimestamp(mp4_file, mp4_track, timestamp, MP4_NANOSECONDS_TIME_SCALE/100ULL);
			HRESULT hr = decoder.Feed(h264_data, this_size, hundrednanos);
			if (hr == MF_E_NOTACCEPTING) {
				nullsoft_h264_frame_data frame_data;
				if (FAILED(decoder.GetFrame((YV12_PLANES **)&frame_data.data, &frame_data.decoder_data, &frame_data.local_timestamp))) {
					continue;
				}
				buffered_frames.push_back(frame_data);
			} else if (FAILED(hr)) {
				return MP4_VIDEO_FAILURE;
			} else {
				break;
			}
		}

		inputBufferBytes-=this_size;
		h264_data+=this_size;
	}
	return MP4_VIDEO_SUCCESS;
}

int H264MP4Decoder::CanHandleCodec(const char *codecName)
{
	return !strcmp(codecName, "avc1");
}

void H264MP4Decoder::Flush()
{
	for (size_t i=0;i<buffered_frames.size();i++) {
		nullsoft_h264_frame_data frame_data = buffered_frames[i];
		decoder.FreeFrame((YV12_PLANES *)frame_data.data, frame_data.decoder_data);
	}
	decoder.Flush();
}

int H264MP4Decoder::GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp)
{
	if (!buffered_frames.empty()) {
		nullsoft_h264_frame_data frame_data = buffered_frames[0];
		buffered_frames.erase(buffered_frames.begin());
		*data = frame_data.data;
		*decoder_data = frame_data.decoder_data;
		*timestamp = MP4ConvertToTrackTimestamp(mp4_file, mp4_track, frame_data.local_timestamp, MP4_NANOSECONDS_TIME_SCALE/100ULL);
		return MP4_VIDEO_SUCCESS;
	} 

	uint64_t local_timestamp;
	if (SUCCEEDED(decoder.GetFrame((YV12_PLANES **)data, decoder_data, &local_timestamp))) {
		*timestamp = MP4ConvertToTrackTimestamp(mp4_file, mp4_track, local_timestamp, MP4_NANOSECONDS_TIME_SCALE/100ULL);
		return MP4_VIDEO_SUCCESS;
	} else {
		return MP4_VIDEO_FAILURE;
	}
}

void H264MP4Decoder::FreePicture(void *data, void *decoder_data)
{
	decoder.FreeFrame((YV12_PLANES *)data, decoder_data);
}

void H264MP4Decoder::HurryUp(int state)
{
	// TODO if (decoder)
	//H264_HurryUp(decoder, state);
}



#define CBCLASS H264MP4Decoder
START_DISPATCH;
CB(MPEG4_VIDEO_OPEN, Open)
CB(MPEG4_VIDEO_GETOUTPUTFORMAT, GetOutputFormat)
CB(MPEG4_VIDEO_DECODE, DecodeSample)
CB(MPEG4_VIDEO_HANDLES_CODEC, CanHandleCodec)
VCB(MPEG4_VIDEO_FLUSH, Flush)
CB(MPEG4_VIDEO_GET_PICTURE, GetPicture)
VCB(MPEG4_VIDEO_FREE_PICTURE, FreePicture)
VCB(MPEG4_VIDEO_HURRY_UP, HurryUp)
END_DISPATCH;
#undef CBCLASS

