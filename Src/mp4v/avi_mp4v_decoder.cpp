#include "avi_mp4v_decoder.h"
#include "../Winamp/wa_ipc.h"
#include "../nsavi/read.h"
#include <mmsystem.h>
#include <Amvideo.h>
#include <Dvdmedia.h>

int AVIDecoderCreator::CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder)
{
	nsavi::video_format *format = (nsavi::video_format *)stream_format;
	if (format) {
		if (
			   format->compression == 'DIVX' || format->compression == 'divx' // xvid  
			|| format->compression == 'xvid' || format->compression == 'XVID' // divx
			|| format->compression == 'v4pm' // mp4v
			|| format->compression == '05XD' // divx 5
			|| format->compression == nsaviFOURCC('S','E','D','G') // dunno what this is exactly
			/* || format->compression == '3VID' // divx 3, let's hope it plays */
			) {
				VIDEOINFOHEADER header = {0, };
				//header.rcSource.right = format->width;
				//header.rcSource.bottom = format->height;
				//header.rcTarget = header.rcSource;
				memcpy(&header.bmiHeader, &format->video_format_size_bytes, 40);
				
			MFTDecoder *ctx = new MFTDecoder;
			if (!ctx) {
				return CREATEDECODER_FAILURE;
			}
			if (FAILED(ctx->Open(&header))) {
				delete ctx;
				return CREATEDECODER_FAILURE;
			}
			*decoder = new AVIMP4V(ctx, stream_header, format);
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

AVIMP4V::AVIMP4V(MFTDecoder *decoder, const nsavi::STRH *stream_header, const nsavi::video_format *stream_format) 
: decoder(decoder), stream_header(stream_header), stream_format(stream_format)
{
	if (stream_format->size_bytes > 40) {
		//MPEG4Video_DecodeFrame(decoder, ((const uint8_t *)stream_format) + 44, stream_format->size_bytes - stream_format->video_format_size_bytes, 0);
	}
}

AVIMP4V::~AVIMP4V()
{
	delete decoder;
}

int AVIMP4V::GetOutputProperties(int *x, int *y, int *color_format, double *aspect_ratio, int *flip)
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

int AVIMP4V::DecodeChunk(uint16_t type, const void *inputBuffer, size_t inputBufferBytes)
{
	if (decoder) {
		decoder->Feed(inputBuffer, inputBufferBytes, 0);
		return AVI_SUCCESS;
	}

	return AVI_FAILURE;
}

void AVIMP4V::Flush()
{
	if (decoder) {
		decoder->Flush();
	}
}

int AVIMP4V::GetPicture(void **data, void **decoder_data)
{
	if (SUCCEEDED(decoder->GetFrame((YV12_PLANES **)data, decoder_data, 0))) {
		return AVI_SUCCESS;
	} else {
		return AVI_FAILURE;
	}
}

void AVIMP4V::FreePicture(void *data, void *decoder_data)
{
	decoder->FreeFrame((YV12_PLANES *)data, decoder_data);
}

void AVIMP4V::EndOfStream()
{
	decoder->Drain();
}

void AVIMP4V::HurryUp(int state)
{
	//if (decoder)
		//MPEG4Video_HurryUp(decoder, state);
}

void AVIMP4V::Close()
{
	delete this;
}

#define CBCLASS AVIMP4V
START_DISPATCH;
CB(GET_OUTPUT_PROPERTIES, GetOutputProperties)
CB(DECODE_CHUNK, DecodeChunk)
VCB(FLUSH, Flush)
CB(GET_PICTURE, GetPicture)
VCB(FREE_PICTURE, FreePicture)
VCB(END_OF_STREAM, EndOfStream)
VCB(HURRY_UP, HurryUp)
VCB(CLOSE, Close)
END_DISPATCH;
#undef CBCLASS

