#include "mkv_f263_decoder.h"
#include "lib.h"

int MKVDecoderCreator::CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder)
{
	if (!strcmp(codec_id, "V_MS/VFW/FOURCC"))
	{
		if (track_entry_data->codec_private && track_entry_data->codec_private_len)
		{
			const BITMAPINFOHEADER *header = (const BITMAPINFOHEADER *)track_entry_data->codec_private;
			if (header->biCompression == '1VLF')
			{
						void *ctx = F263_CreateDecoder();
				*decoder = new MKVFLV1(ctx);
				return CREATEDECODER_SUCCESS;
			}
		}
		return CREATEDECODER_NOT_MINE;
	}
	else
	{
		return CREATEDECODER_NOT_MINE;
	}
}


#define CBCLASS MKVDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS



MKVFLV1::MKVFLV1(void *ctx) : decoder(ctx)
{

}

int MKVFLV1::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio)
{
	if (width && height)
	{
		*x = width;
		*y = height;
		*color_format = '21VY';
		*aspect_ratio=1.0;
		return MKV_SUCCESS;
	}
	return MKV_FAILURE;
}

int MKVFLV1::DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp)
{
  last_timestamp = (int32_t)timestamp;
	int keyframe;

	int ret = F263_DecodeFrame(decoder, const_cast<void *>(inputBuffer), inputBufferBytes, &yv12, &width, &height, &keyframe);
	if (ret == F263_OK)
	{
		decoded=1;
		return MKV_SUCCESS;
	}
	else
		return MKV_FAILURE;
}

void MKVFLV1::Flush()
{
}

int MKVFLV1::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	if (decoded)
	{
		*timestamp = last_timestamp;
		*decoder_data = 0;
		*data = &yv12;
		decoded=0;
		return MKV_SUCCESS;
	}
	return MKV_FAILURE;		
}


#define CBCLASS MKVFLV1
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
CB(GET_PICTURE, GetPicture)
END_DISPATCH;
#undef CBCLASS

