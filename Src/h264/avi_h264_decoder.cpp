#include "avi_h264_decoder.h"
#include "../Winamp/wa_ipc.h"
#include <mmsystem.h>
#include <assert.h>
#include <Mferror.h>


int AVIDecoderCreator::CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder)
{
	nsavi::video_format *format = (nsavi::video_format *)stream_format;
	if (format)
	{
		if (format->compression == '462H')
		{
			MFTDecoder *ctx = new MFTDecoder();
			if (!ctx)
				return CREATEDECODER_FAILURE;

			if (FAILED(ctx->Open())) {
				delete ctx;
				return CREATEDECODER_FAILURE;
			}
			*decoder = new AVIH264(ctx, stream_header);
			return CREATEDECODER_SUCCESS;
		}
	}
	return CREATEDECODER_NOT_MINE;
}


#define CBCLASS AVIDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS

AVIH264::AVIH264(MFTDecoder *ctx, const nsavi::STRH *stream_header) : decoder(ctx), stream_header(stream_header)
{
}

AVIH264::~AVIH264()
{
	for ( size_t i = 0; i < buffered_frames.size(); i++ )
	{
		nullsoft_h264_frame_data frame_data = buffered_frames[ i ];
		decoder->FreeFrame( (YV12_PLANES *)frame_data.data, frame_data.decoder_data );
	}

	delete decoder;
}

int AVIH264::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
{
	UINT width, height;
	bool local_flip=false;
	if (SUCCEEDED(decoder->GetOutputFormat(&width, &height, &local_flip, aspect_ratio))) {
		*x = width;
		*y = height;
		*color_format = '21VY';
		*flip = local_flip;
		return AVI_SUCCESS;
	}
	return AVI_FAILURE;		
}

int AVIH264::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
{
	for (;;) {
		HRESULT hr = decoder->FeedRaw(inputBuffer, inputBufferBytes, 0);
		if (hr == MF_E_NOTACCEPTING) {
			nullsoft_h264_frame_data frame_data;
			if (FAILED(decoder->GetFrame((YV12_PLANES **)&frame_data.data, &frame_data.decoder_data, &frame_data.local_timestamp))) {
				continue;
			}
			buffered_frames.push_back(frame_data);
		} else if (FAILED(hr)) {
			return AVI_FAILURE;
		} else {
			break;
		}
	}
	return AVI_SUCCESS;
}

void AVIH264::Flush()
{
	for (size_t i=0;i<buffered_frames.size();i++) {
		nullsoft_h264_frame_data frame_data = buffered_frames[i];
		decoder->FreeFrame((YV12_PLANES *)frame_data.data, frame_data.decoder_data);
	}
	decoder->Flush();
}

int AVIH264::GetPicture(void **data, void **decoder_data)
{
	if (!buffered_frames.empty()) {
		nullsoft_h264_frame_data frame_data = buffered_frames[0];
		buffered_frames.erase(buffered_frames.begin());
		*data = frame_data.data;
		*decoder_data = frame_data.decoder_data;
		return AVI_SUCCESS;
	}

	if (SUCCEEDED(decoder->GetFrame((YV12_PLANES **)data, decoder_data, 0))) {
		return AVI_SUCCESS;
	} else {
		return AVI_FAILURE;
	}
}

void AVIH264::FreePicture(void *data, void *decoder_data)
{
	decoder->FreeFrame((YV12_PLANES *)data, decoder_data);
}

void AVIH264::EndOfStream()
{
	decoder->Drain();
}

void AVIH264::HurryUp(int state)
{
	// TODO(benski)
	//if (decoder)
//		H264_HurryUp(decoder, state);
}

void AVIH264::Close()
{
	delete this;
}

#define CBCLASS AVIH264
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
VCB(CLOSE, Close)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
VCB(END_OF_STREAM, EndOfStream)
VCB(HURRY_UP, HurryUp)
END_DISPATCH;
#undef CBCLASS
