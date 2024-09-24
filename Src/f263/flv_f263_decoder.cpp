#include "flv_f263_decoder.h"
#include "lib.h"

int FLVDecoderCreator::CreateVideoDecoder(int format_type, int width, int height, ifc_flvvideodecoder **decoder)
{
	if (format_type == FLV::VIDEO_FORMAT_SORENSON)
	{
		void *ctx = F263_CreateDecoder();
		if (!ctx)
			return CREATEDECODER_FAILURE;
		*decoder = new FLVSorenson(ctx);
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

int FLVDecoderCreator::HandlesVideo(int format_type)
{
	if (format_type == FLV::VIDEO_FORMAT_SORENSON)
	{
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

#define CBCLASS FLVDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
CB(HANDLES_VIDEO, HandlesVideo)
END_DISPATCH;
#undef CBCLASS

/* --- */

FLVSorenson::FLVSorenson(void *decoder) : decoder(decoder)
{
	last_timestamp=0;
	width=0;
	height=0;
	decoded=0;
}

int FLVSorenson::GetOutputFormat(int *x, int *y, int *color_format)
{
	if (width && height)
	{
		*x = width;
		*y = height;
		*color_format = '21VY';
		return FLV_VIDEO_SUCCESS;
	}
	return FLV_VIDEO_FAILURE;
}

int FLVSorenson::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, int32_t timestamp)
{
  last_timestamp = timestamp;
	int keyframe;

	int ret = F263_DecodeFrame(decoder, const_cast<void *>(inputBuffer), inputBufferBytes, &yv12, &width, &height, &keyframe);
	if (ret == F263_OK)
	{
		decoded=1;
		return FLV_VIDEO_SUCCESS;
	}
	else
		return FLV_VIDEO_FAILURE;
}

void FLVSorenson::Flush()
{
}

void FLVSorenson::Close()
{
	if (decoder)
		F263_DestroyDecoder(decoder);
	delete this;
}

int FLVSorenson::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	if (decoded)
	{
		*timestamp = last_timestamp;
		*decoder_data = 0;
		*data = &yv12;
		decoded=0;
		return FLV_VIDEO_SUCCESS;
	}
	return FLV_VIDEO_FAILURE;		
}

#define CBCLASS FLVSorenson
START_DISPATCH;
CB(FLV_VIDEO_GETOUTPUTFORMAT, GetOutputFormat)
CB(FLV_VIDEO_DECODE, DecodeSample)
VCB(FLV_VIDEO_FLUSH, Flush)
VCB(FLV_VIDEO_CLOSE, Close)
CB(FLV_VIDEO_GET_PICTURE, GetPicture)
END_DISPATCH;
#undef CBCLASS
