#include "h264_flv_decoder.h"
#include "../Winamp/wa_ipc.h" // for YV12_PLANES
#include <Mferror.h>

int FLVDecoderCreator::CreateVideoDecoder(int format_type, int width, int height, ifc_flvvideodecoder **decoder)
{
	if (format_type == FLV::VIDEO_FORMAT_AVC)
	{
		MFTDecoder *ctx = new MFTDecoder();
		if (!ctx || FAILED(ctx->Open())) {
			delete ctx;
			return CREATEDECODER_FAILURE;
		}
		*decoder = new FLVH264(ctx);
		return CREATEDECODER_SUCCESS;
	}
	return CREATEDECODER_NOT_MINE;
}

int FLVDecoderCreator::HandlesVideo(int format_type)
{
	if (format_type == FLV::VIDEO_FORMAT_AVC)
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
uint32_t GetNALUSize(uint64_t nalu_size_bytes, const uint8_t *h264_data, size_t data_len);
uint32_t Read24(const uint8_t *data);

FLVH264::FLVH264(MFTDecoder *decoder) : decoder(decoder)
{
	sequence_headers_parsed=0;
	nalu_size_bytes=0;
}

FLVH264::~FLVH264()
{
	for (size_t i=0;i<buffered_frames.size();i++) {
		nullsoft_h264_frame_data frame_data = buffered_frames[i];
		decoder->FreeFrame((YV12_PLANES *)frame_data.data, frame_data.decoder_data);
	}
	delete decoder;
}

int FLVH264::GetOutputFormat(int *x, int *y, int *color_format)
{
	UINT width, height;
	bool local_flip=false;
	double aspect_ratio;
	if (SUCCEEDED(decoder->GetOutputFormat(&width, &height, &local_flip, &aspect_ratio))) {
		*x = width;
		*y = height;
		*color_format = '21VY';
		return FLV_VIDEO_SUCCESS;
	}
	return FLV_VIDEO_FAILURE;	
}

int FLVH264::DecodeSample(const void *inputBuffer, size_t inputBufferBytes, int32_t timestamp)
{
	const uint8_t *h264_data = (const uint8_t *)inputBuffer;
	if (*h264_data == 0 && inputBufferBytes >= 10) // sequence headers 
	{
		h264_data++; // skip packet type 
		uint32_t timestamp_offset = Read24(h264_data); 
		h264_data+=3;
		inputBufferBytes -=4;
		h264_data+=4; // don't care about level & profile
		inputBufferBytes -=4;
		nalu_size_bytes = (*h264_data++ & 0x3)+1;
		inputBufferBytes--;
		size_t num_sps = *h264_data++ & 0x1F;
		inputBufferBytes--;
		for (size_t i=0;i!=num_sps;i++)
		{
			if (inputBufferBytes > 2)
			{
				uint16_t sps_size = (h264_data[0] << 8) | h264_data[1];
				h264_data+=2;
				inputBufferBytes-=2;
				//H264_ProcessSPS(decoder, h264_data+1, sps_size);
				if (inputBufferBytes >= sps_size)
				{
					decoder->Feed(h264_data, sps_size, timestamp+timestamp_offset);
					h264_data+=sps_size;
					inputBufferBytes-=sps_size;
				}
			}
		}
		if (inputBufferBytes)
		{
			size_t num_pps = *h264_data++;
			inputBufferBytes--;
			for (size_t i=0;i!=num_pps;i++)
			{
				if (inputBufferBytes > 2)
				{
					uint16_t sps_size = (h264_data[0] << 8) | h264_data[1];
					h264_data+=2;
					inputBufferBytes-=2;
					//H264_ProcessPPS(decoder, h264_data+1, sps_size);
					if (inputBufferBytes >= sps_size)
					{
						decoder->Feed(h264_data, sps_size, timestamp+timestamp_offset);
						h264_data+=sps_size;
						inputBufferBytes-=sps_size;
					}
				}
			}
		}
		sequence_headers_parsed=1;
	}
	else if (*h264_data == 1) // frame
	{
		h264_data++;
		inputBufferBytes--;
		if (inputBufferBytes < 3)
			return FLV_VIDEO_FAILURE;
		uint32_t timestamp_offset = Read24(h264_data);

		h264_data+=3;
		inputBufferBytes-=3;

		while (inputBufferBytes)
		{
			uint32_t this_size =GetNALUSize(nalu_size_bytes, h264_data, inputBufferBytes);
			if (this_size == 0)
				return FLV_VIDEO_FAILURE;

			inputBufferBytes-=nalu_size_bytes;
			h264_data+=nalu_size_bytes;
			if (this_size > inputBufferBytes)
				return FLV_VIDEO_FAILURE;
			for (;;) {
				HRESULT hr = decoder->Feed(h264_data, this_size, timestamp+timestamp_offset);
				if (hr == MF_E_NOTACCEPTING) {
					nullsoft_h264_frame_data frame_data;
					if (FAILED(decoder->GetFrame((YV12_PLANES **)&frame_data.data, &frame_data.decoder_data, &frame_data.local_timestamp))) {
						continue;
					}
					buffered_frames.push_back(frame_data);
				} else if (FAILED(hr)) {
					return FLV_VIDEO_FAILURE;
				} else {
					break;
				}
			}			

			inputBufferBytes-=this_size;
			h264_data+=this_size;
		}
	}

	return FLV_VIDEO_SUCCESS;
}

void FLVH264::Flush()
{
	for (size_t i=0;i<buffered_frames.size();i++) {
		nullsoft_h264_frame_data frame_data = buffered_frames[i];
		decoder->FreeFrame((YV12_PLANES *)frame_data.data, frame_data.decoder_data);
	}
	decoder->Flush();
}

void FLVH264::Close()
{
	delete this;
}

int FLVH264::GetPicture(void **data, void **decoder_data, uint64_t *timestamp)
{
	if (!buffered_frames.empty()) {
		nullsoft_h264_frame_data frame_data = buffered_frames[0];
		buffered_frames.erase(buffered_frames.begin());
		*data = frame_data.data;
		*decoder_data = frame_data.decoder_data;
		*timestamp = frame_data.local_timestamp;
		return FLV_VIDEO_SUCCESS;
	}

	if (SUCCEEDED(decoder->GetFrame((YV12_PLANES **)data, decoder_data, timestamp))) {
		return FLV_VIDEO_SUCCESS;
	} else {
		return FLV_VIDEO_FAILURE;
	}
}

void FLVH264::FreePicture(void *data, void *decoder_data)
{
	decoder->FreeFrame((YV12_PLANES *)data, decoder_data);
}

int FLVH264::Ready()
{
	return sequence_headers_parsed;
}

#define CBCLASS FLVH264
START_DISPATCH;
CB(FLV_VIDEO_GETOUTPUTFORMAT, GetOutputFormat)
CB(FLV_VIDEO_DECODE, DecodeSample)
VCB(FLV_VIDEO_FLUSH, Flush)
VCB(FLV_VIDEO_CLOSE, Close)
CB(FLV_VIDEO_GET_PICTURE, GetPicture)
VCB(FLV_VIDEO_FREE_PICTURE, FreePicture)
CB(FLV_VIDEO_READY, Ready)
END_DISPATCH;
#undef CBCLASS

