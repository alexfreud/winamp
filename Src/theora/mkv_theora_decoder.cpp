#include "mkv_theora_decoder.h"
#include "../nsmkv/Lacing.h"
#include "../nsmkv/Cluster.h"
#include <mmsystem.h>

int MKVDecoder::CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder)
{
	if (!strcmp(codec_id, "V_THEORA"))
	{
		MKVTheora *theora = new MKVTheora(video_data);
		nsmkv::LacingState lacing_state;
		if (nsmkv::Lacing::GetState(nsmkv::BlockBinary::XIPH_LACING, (const uint8_t *)track_entry_data->codec_private, track_entry_data->codec_private_len, &lacing_state))
		{
			const uint8_t *frame;
			size_t frame_len;
			uint16_t frame_number=0;
			while (nsmkv::Lacing::GetFrame(frame_number, (const uint8_t *)track_entry_data->codec_private, track_entry_data->codec_private_len, &frame, &frame_len, &lacing_state))
			{
				ogg_packet packet = {const_cast<uint8_t *>(frame), (long)frame_len, (frame_number==0), 0, 0 /*-1?*/, theora->packet_number++};
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

MKVTheora::MKVTheora(const nsmkv::VideoData *video_data) : video_data(video_data)
{
	th_info_init(&info);
	memset(&comment, 0, sizeof(comment));
	setup=0;
	packet_number=0;
	frame_ready=false;
	flushing=false;
}

int MKVTheora::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio)
{
	if (decoder)
	{
				th_ycbcr_buffer buffer;
		if (th_decode_ycbcr_out(decoder, buffer) == 0)
		{
			*x = buffer[0].width;
			*y = buffer[0].height;
			*aspect_ratio=1.0;
			*color_format = mmioFOURCC('Y','V','1','2');
			return MKV_SUCCESS;
		}
	}
	return MKV_FAILURE;
}

int MKVTheora::DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp)
{
	if (decoder)
	{
		ogg_packet packet = {(uint8_t *)inputBuffer, (long)inputBufferBytes, 0, 0, 0 -1, packet_number++};
		if (flushing)
		{
			if (th_packet_iskeyframe(&packet))
			{
				flushing=false;
			}
			else
				return MKV_FAILURE;
		}
		
		if (th_decode_packetin(decoder, &packet, 0) == 0)
			frame_ready=true;
		last_timestamp=timestamp;
		return MKV_SUCCESS;
	}

	return MKV_FAILURE;
}

void MKVTheora::Flush()
{
	packet_number = 0;
	flushing=true;
}

int MKVTheora::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	if (decoder && frame_ready)
	{
		th_ycbcr_buffer buffer;
		if (th_decode_ycbcr_out(decoder, buffer) == 0)
		{
			planes.y.baseAddr = buffer[0].data;
			planes.y.rowBytes = buffer[0].stride;
			planes.u.baseAddr = buffer[1].data;
			planes.u.rowBytes = buffer[1].stride;
			planes.v.baseAddr = buffer[2].data;
			planes.v.rowBytes = buffer[2].stride;
			*data = &planes;
			*decoder_data = 0;
			*timestamp = last_timestamp;
			frame_ready = false;
			return MKV_SUCCESS;
		}
	}

	return MKV_FAILURE;
}

void MKVTheora::FreePicture(void *data, void *decoder_data)
{
}

void MKVTheora::HurryUp(int state)
{

}

void MKVTheora::Close()
{
	if (decoder)
		th_decode_free(decoder);
	delete this;
}

#define CBCLASS MKVTheora
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

