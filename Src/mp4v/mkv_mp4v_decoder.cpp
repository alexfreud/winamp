#include "mkv_mp4v_decoder.h"
#include "../Winamp/wa_ipc.h" // for YV12_PLANES
#include <mmsystem.h>
#include <assert.h>

int MKVDecoderCreator::CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder)
{
	if (!strcmp(codec_id, "V_MPEG4/ISO/ASP")
		|| !strcmp(codec_id, "V_MPEG4/ISO/SP"))
	{
		VIDEOINFOHEADER header = {0, };

		header.bmiHeader.biHeight = (LONG)video_data->pixel_height;
		header.bmiHeader.biWidth = (LONG)video_data->pixel_width;
		header.bmiHeader.biCompression = 'v4pm';

		MFTDecoder *ctx = new MFTDecoder;
		if (!ctx) {
			return CREATEDECODER_FAILURE;
		}
		HRESULT hr = ctx->Open(&header);
		if (FAILED(hr)) {
			delete ctx;
			return CREATEDECODER_FAILURE;
		}

		if (ctx)
		{
			if (track_entry_data->codec_private && track_entry_data->codec_private_len) {
				// mkv stores headers up to first VOP in codec_private
				hr = ctx->Feed(track_entry_data->codec_private, track_entry_data->codec_private_len, 0);
				if (FAILED(hr)) {
					delete ctx;
					return CREATEDECODER_FAILURE;
				}
			}
			*decoder = new MKVMP4V(ctx, video_data);
			return CREATEDECODER_SUCCESS;
		}
		else
		{
			return CREATEDECODER_FAILURE;
		}
	}
	else if (!strcmp(codec_id, "V_MS/VFW/FOURCC"))
	{
		if (track_entry_data->codec_private && track_entry_data->codec_private_len)
		{
			const BITMAPINFOHEADER *header = (const BITMAPINFOHEADER *)track_entry_data->codec_private;
			if (header->biCompression == 'DIVX'
				|| header->biCompression == '05XD')
			{
				if (track_entry_data->codec_private_len < 40) {
					return CREATEDECODER_FAILURE;
				}

				VIDEOINFOHEADER video_header = {0, };
				memcpy(&video_header.bmiHeader, header, 40);
				assert(track_entry_data->codec_private_len == 40);

				MFTDecoder *ctx = new MFTDecoder;
				if (!ctx) {
					return CREATEDECODER_FAILURE;
				}
				if (FAILED(ctx->Open(&video_header))) {
					delete ctx;
					return CREATEDECODER_FAILURE;
				}

				*decoder = new MKVMP4V(ctx, video_data);
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

MKVMP4V::MKVMP4V(MFTDecoder *decoder, const nsmkv::VideoData *video_data) : decoder(decoder), video_data(video_data)
{

}

MKVMP4V::~MKVMP4V()
{
	delete decoder;
}

int MKVMP4V::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio)
{
	UINT width, height;
	bool local_flip=false;
	if (SUCCEEDED(decoder->GetOutputFormat(&width, &height, &local_flip, aspect_ratio))) {
		*x = width;
		*y = height;
		*color_format = '21VY';
		return MKV_SUCCESS;
	}
	return MKV_FAILURE;		
}

int MKVMP4V::DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp)
{
	HRESULT hr;
	hr=decoder->Feed(inputBuffer, inputBufferBytes, timestamp);
	return MKV_SUCCESS;
}

void MKVMP4V::Flush()
{
	if (decoder) {
		decoder->Flush();
	}
}

int MKVMP4V::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	if (SUCCEEDED(decoder->GetFrame((YV12_PLANES **)data, decoder_data, timestamp))) {
		return MKV_SUCCESS;
	} else {
		return MKV_FAILURE;
	}
}

void MKVMP4V::FreePicture(void *data, void *decoder_data)
{
	if (decoder) {
		decoder->FreeFrame((YV12_PLANES *)data, decoder_data);
	}
}

void MKVMP4V::HurryUp(int state)
{
//	if (decoder)
//		MPEG4Video_HurryUp(decoder, state);
}

void MKVMP4V::Close()
{
	delete this;
}

#define CBCLASS MKVMP4V
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
VCB(HURRY_UP, HurryUp)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS
