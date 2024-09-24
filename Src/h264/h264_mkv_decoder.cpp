#include "h264_mkv_decoder.h"
#include "../Winamp/wa_ipc.h" // for YV12_PLANES
#include <winsock.h>
#include <mmsystem.h>
#include <Mferror.h>

int MKVDecoderCreator::CreateVideoDecoder(const char *codec_id, const nsmkv::TrackEntryData *track_entry_data, const nsmkv::VideoData *video_data, ifc_mkvvideodecoder **decoder)
{
	if (!strcmp(codec_id, "V_MPEG4/ISO/AVC"))
	{
		const uint8_t *init_data = (const uint8_t *)track_entry_data->codec_private;
		size_t init_data_len = track_entry_data->codec_private_len;
		if (init_data && init_data_len >= 6)
		{
			MFTDecoder *ctx = new MFTDecoder;
			if (!ctx)
				return CREATEDECODER_FAILURE;

			if (FAILED(ctx->Open())) {
				delete ctx;
				return CREATEDECODER_FAILURE;
			}


			init_data+=4; // don't care about level & profile
			init_data_len-=4;

			// read NALU header size length
			uint8_t nalu_minus_one = *init_data++ & 0x3;
			init_data_len--;

			// number of SPS NAL units
			uint8_t num_sps = *init_data++ & 0x1F;
			init_data_len--;
			for (uint8_t i=0;i!=num_sps;i++)
			{
				if (init_data_len < 2)
				{
					delete ctx;
					return CREATEDECODER_FAILURE;
				}
				uint16_t *s = (uint16_t *)init_data;
				uint16_t sps_size = htons(*s);
				init_data+=2;
				init_data_len-=2;
				if (init_data_len < sps_size)
				{
					delete ctx;
					return CREATEDECODER_FAILURE;
				}
				ctx->Feed(init_data, sps_size, 0);
				init_data+=sps_size;
				init_data_len-=sps_size;
			}

			// read PPS NAL units
			if (init_data_len)
			{
				// number of PPS NAL units
				uint8_t num_pps = *init_data++ & 0x1F;
				init_data_len--;
				for (uint8_t i=0;i!=num_pps;i++)
				{
					if (init_data_len < 2)
					{
						delete ctx;
						return CREATEDECODER_FAILURE;
					}
					uint16_t *s = (uint16_t *)init_data;
					uint16_t pps_size = htons(*s);
					init_data+=2;
					init_data_len-=2;
					if (init_data_len < pps_size)
					{
						delete ctx;
						return CREATEDECODER_FAILURE;
					}
					ctx->Feed(init_data, pps_size, 0);
					init_data+=pps_size;
					init_data_len-=pps_size;
				}
			}
			// if we made it here, we should be good
			*decoder = new MKVH264(ctx, nalu_minus_one, video_data);
			return CREATEDECODER_SUCCESS;
		}
		else
		{
			return CREATEDECODER_FAILURE;
		}
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

MKVH264::MKVH264(MFTDecoder *ctx, uint8_t nalu_minus_one, const nsmkv::VideoData *video_data) : decoder(ctx), video_data(video_data)
{
	nalu_size = nalu_minus_one + 1;
	width=0;
	height=0;
}

MKVH264::~MKVH264()
{
	for (size_t i=0;i<buffered_frames.size();i++) {
		nullsoft_h264_frame_data frame_data = buffered_frames[i];
		decoder->FreeFrame((YV12_PLANES *)frame_data.data, frame_data.decoder_data);
	}
	delete decoder;
}

int MKVH264::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio)
{

	if (decoder) {
		bool flip;
		if (SUCCEEDED(decoder->GetOutputFormat(&width, &height, &flip, aspect_ratio))) {
			*x = width;
			*y = height;
			*color_format = htonl('YV12');
			return MKV_SUCCESS;
		}
	}
	return MKV_FAILURE;
}

uint32_t GetNALUSize(uint64_t nalu_size_bytes, const uint8_t *h264_data, size_t data_len);

int MKVH264::DecodeBlock(const void *inputBuffer, size_t inputBufferBytes, uint64_t timestamp)
{
	const uint8_t *h264_data = (const uint8_t *)inputBuffer;
	while (inputBufferBytes) {
		uint32_t this_size = GetNALUSize(nalu_size, h264_data, inputBufferBytes);
		if (this_size == 0)
			return MKV_FAILURE;

		inputBufferBytes-=nalu_size;
		h264_data+=nalu_size;
		if (this_size > inputBufferBytes)
			return MKV_FAILURE;

		for (;;) {
			HRESULT hr = decoder->Feed(h264_data, this_size, timestamp);
			if (hr == MF_E_NOTACCEPTING) {
				nullsoft_h264_frame_data frame_data;
				if (FAILED(decoder->GetFrame((YV12_PLANES **)&frame_data.data, &frame_data.decoder_data, &frame_data.local_timestamp))) {
					continue;
				}
				buffered_frames.push_back(frame_data);
			} else if (FAILED(hr)) {
				return MKV_FAILURE;
			} else {
				break;
			}
		}

		inputBufferBytes-=this_size;
		h264_data+=this_size;
	}
	return MKV_SUCCESS;
}

void MKVH264::Flush()
{
	for (size_t i=0;i<buffered_frames.size();i++) {
		nullsoft_h264_frame_data frame_data = buffered_frames[i];
		decoder->FreeFrame((YV12_PLANES *)frame_data.data, frame_data.decoder_data);
	}
	if (decoder) {
		decoder->Flush();
	}
}

int MKVH264::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	if (!buffered_frames.empty()) {
		nullsoft_h264_frame_data frame_data = buffered_frames[0];
		buffered_frames.erase(buffered_frames.begin());
		*data = frame_data.data;
		*decoder_data = frame_data.decoder_data;
		*timestamp = frame_data.local_timestamp;
		return MKV_SUCCESS;
	}

	if (SUCCEEDED(decoder->GetFrame((YV12_PLANES **)data, decoder_data, timestamp))) {
		return MKV_SUCCESS;
	} else {
		return MKV_FAILURE;
	}
}

void MKVH264::FreePicture(void *data, void *decoder_data)
{
	decoder->FreeFrame((YV12_PLANES *)data, decoder_data);
}

void MKVH264::EndOfStream()
{
	if (decoder) {
		decoder->Drain();
	}
}

void MKVH264::HurryUp(int state)
{
	// TODO(benski)
	//if (decoder)
	//	H264_HurryUp(decoder, state);
}

void MKVH264::Close()
{
	delete this;
}

#define CBCLASS MKVH264
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_BLOCK, DecodeBlock)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
VCB(END_OF_STREAM, EndOfStream)
VCB(HURRY_UP, HurryUp)
END_DISPATCH;
#undef CBCLASS

