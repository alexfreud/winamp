#include "mkv_vp8x_decoder.h"
#include "../nsmkv/Lacing.h"
#include "../nsmkv/Cluster.h"
#include <mmsystem.h>

int MKVDecoder::CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder)
{
	if (!strcmp(codec_id, "V_VP8"))
	{
		vpx_codec_ctx_t codec;
		if (vpx_codec_dec_init(&codec, &vpx_codec_vp8_dx_algo, NULL, 0) == VPX_CODEC_OK)
		{
			MKVVP8 *vp8 = new MKVVP8(codec, video_data);
			*decoder = vp8;
			return CREATEDECODER_SUCCESS;
		}
#if 0

		nsmkv::LacingState lacing_state;
		if (nsmkv::Lacing::GetState(nsmkv::BlockBinary::XIPH_LACING, (const uint8_t *)track_entry_data->codec_private, track_entry_data->codec_private_len, &lacing_state))
		{
			const uint8_t *frame;
			size_t frame_len;
			uint16_t frame_number=0;
			while (nsmkv::Lacing::GetFrame(frame_number, (const uint8_t *)track_entry_data->codec_private, track_entry_data->codec_private_len, &frame, &frame_len, &lacing_state))
			{
				ogg_packet packet = {const_cast<uint8_t *>(frame), frame_len, (frame_number==0), 0, 0 /*-1?*/, theora->packet_number++};
				int ret = th_decode_headerin(&theora->info, &theora->comment, &theora->setup, &packet);
				if (ret < 0)
					goto bail;
				frame_number++;
			}
			theora->decoder = th_decode_alloc(&theora->info, theora->setup);
			if (!theora->decoder)
				goto bail;

			*decoder = theora;
			return CREATEDECODER_SUCCESS;
		}

bail:
		delete theora;
#endif
		return CREATEDECODER_FAILURE;

	}
	else
	{
		return CREATEDECODER_NOT_MINE;
	}
}


#define CBCLASS MKVDecoder
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS

MKVVP8::MKVVP8(vpx_codec_ctx_t decoder, const nsmkv::VideoData *video_data) : decoder(decoder), video_data(video_data)
{
	flushing=false;
}

int MKVVP8::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio)
{
	vpx_codec_stream_info_t stream_info;
	stream_info.sz = sizeof(stream_info);
	if (vpx_codec_get_stream_info(&decoder, &stream_info) == VPX_CODEC_OK)
	{
		*x = stream_info.w;
		*y = stream_info.h;
		*aspect_ratio=1.0;
		*color_format = mmioFOURCC('Y','V','1','2');
		return MKV_SUCCESS;
	}

	return MKV_FAILURE;
}

int MKVVP8::DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp)
{
	frame_iterator = 0;
	vpx_codec_decode(&decoder, (const uint8_t *)inputBuffer, (unsigned int)inputBufferBytes, 0, 0);
	return MKV_SUCCESS;
}

void MKVVP8::Flush()
{
		flushing=true;
}

int MKVVP8::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	if (flushing)
	{
		vpx_codec_stream_info_t stream_info;
		stream_info.sz = sizeof(stream_info);
		if (vpx_codec_get_stream_info(&decoder, &stream_info) == VPX_CODEC_OK)
		{
			if (!stream_info.is_kf)
				return MKV_FAILURE;
			flushing=false;
		}
	}

	vpx_image_t *image = vpx_codec_get_frame(&decoder, &frame_iterator);
	if (image)
	{
		planes.y.baseAddr = image->planes[0];
		planes.y.rowBytes = image->stride[0];
		planes.u.baseAddr = image->planes[1];
		planes.u.rowBytes = image->stride[1];
		planes.v.baseAddr = image->planes[2];
		planes.v.rowBytes = image->stride[2];
		*data = &planes;
		*decoder_data = 0;

		return MKV_SUCCESS;
	}

	return MKV_FAILURE;
}

void MKVVP8::FreePicture(void *data, void *decoder_data)
{
}

void MKVVP8::HurryUp(int state)
{
}

void MKVVP8::Close()
{
	vpx_codec_destroy(&decoder);
	delete this;
}

#define CBCLASS MKVVP8
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

