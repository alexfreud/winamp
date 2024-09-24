#pragma once
#include <bfc/platform/types.h>
#ifdef __cplusplus
extern "C" {
#endif
	#include "ldecod/inc/mbuffer.h"
#include "lcommon/inc/frame.h"

typedef void *h264_decoder_t;

int H264_Init(); // initializes the library.  currently just does a CPU feature check (sse2, etc)
h264_decoder_t H264_CreateDecoder();
void H264_DestroyDecoder(h264_decoder_t decoder);

void H264_DecodeFrame(h264_decoder_t decoder, const void *buffer, size_t bufferlen, uint64_t time_code);
void H264_GetPicture(h264_decoder_t decoder, StorablePicture **pic);
void H264_FreePicture(h264_decoder_t decoder, StorablePicture *pic);
void H264_Flush(h264_decoder_t decoder);
void H264_EndOfStream(h264_decoder_t decoder);
void H264_HurryUp(h264_decoder_t decoder, int state);
const FrameFormat *H264_GetOutputFormat(h264_decoder_t decoder, double *aspect_ratio);

#ifdef __cplusplus
}
#endif