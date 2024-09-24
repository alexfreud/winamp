#include "avi_decoder.h"
#include "avi_tscc_decoder.h"
#include "avi_rle_decoder.h"
#include "avi_yuv_decoder.h"
#include "avi_rgb_decoder.h"

int AVIDecoderCreator::CreateVideoDecoder(const nsavi::AVIH *avi_header, const nsavi::STRH *stream_header, const nsavi::STRF *stream_format, const nsavi::STRD *stream_data, ifc_avivideodecoder **decoder)
{
	nsavi::video_format *format = (nsavi::video_format *)stream_format;
	if (format)
	{
		if (format->compression == 'ccst') // tscc
		{
			*decoder = AVITSCC::CreateDecoder(format);
			if (*decoder)
				return CREATEDECODER_SUCCESS;
			else
				return CREATEDECODER_FAILURE;
		}
		else if (format->compression == nsavi::video_format_rle8) // 8bit RLE
		{
			*decoder = AVIRLE::CreateDecoder(format);
			if (*decoder)
				return CREATEDECODER_SUCCESS;
			else
				return CREATEDECODER_FAILURE;
		}
		else if (format->compression == 'YVYU') // YUV
		{
			*decoder = AVIYUV::CreateDecoder(format);
			if (*decoder)
				return CREATEDECODER_SUCCESS;
			else
				return CREATEDECODER_FAILURE;
		}
		else if (format->compression == nsavi::video_format_rgb)
		{
			*decoder = AVIRGB::CreateDecoder(format);
			if (*decoder)
				return CREATEDECODER_SUCCESS;
			else
				return CREATEDECODER_FAILURE;
		}
	}

	return CREATEDECODER_NOT_MINE;
}


#define CBCLASS AVIDecoderCreator
START_DISPATCH;
CB(CREATE_VIDEO_DECODER, CreateVideoDecoder)
END_DISPATCH;
#undef CBCLASS
