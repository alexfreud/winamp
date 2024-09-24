#pragma once
#ifdef __cplusplus
extern "C" 
{
#endif
#include <bfc/platform/types.h>
#include "mp4def.h"
typedef void *mpeg4vid_decoder_t;

mpeg4vid_decoder_t MPEG4Video_CreateDecoder(int filetype, int codec);
void MPEG4Video_DestroyDecoder(mpeg4vid_decoder_t decoder);
void MPEG4Video_DecodeFrame(mpeg4vid_decoder_t decoder, const void *buffer, size_t bufferlen, uint64_t time_code);
void MPEG4Video_GetPicture(mpeg4vid_decoder_t decoder, mp4_Frame **frame);
int MPEG4Video_GetOutputFormat(mpeg4vid_decoder_t decoder, int *width, int *height, double *aspect_ratio);
void MPEG4Video_Flush(mpeg4vid_decoder_t decoder);
void MPEG4Video_HurryUp(mpeg4vid_decoder_t decoder, int state);
void MPEG4Video_EndOfStream(mpeg4vid_decoder_t decoder);
void MPEG4Video_ReleaseFrame(mpeg4vid_decoder_t d,  mp4_Frame *frame);
#define MPEG4_PROFILE_SIMPLE                      0
#define MPEG4_PROFILE_SIMPLE_SCALABLE             1
#define MPEG4_PROFILE_CORE                        2
#define MPEG4_PROFILE_MAIN                        3
#define MPEG4_PROFILE_NBIT                        4
#define MPEG4_PROFILE_SCALABLE_TEXTURE            5
#define MPEG4_PROFILE_SIMPLE_FACE                 6
#define MPEG4_PROFILE_BASIC_ANIMATED_TEXTURE      7
#define MPEG4_PROFILE_HYBRID                      8
#define MPEG4_PROFILE_ADVANCED_REAL_TIME_SIMPLE   9
#define MPEG4_PROFILE_CORE_SCALABLE              10
#define MPEG4_PROFILE_ADVANCED_CODE_EFFICIENCY   11
#define MPEG4_PROFILE_ADVANCED_CORE              12
#define MPEG4_PROFILE_ADVANCED_SCALABLE_TEXTURE  13
#define MPEG4_PROFILE_STUDIO                     14
#define MPEG4_PROFILE_ADVANCED_SIMPLE            15
#define MPEG4_PROFILE_FGS                        16

#define MPEG4_LEVEL_0    0
#define MPEG4_LEVEL_1    1
#define MPEG4_LEVEL_2    2
#define MPEG4_LEVEL_3    3
#define MPEG4_LEVEL_4    4
#define MPEG4_LEVEL_5    5
#define MPEG4_LEVEL_3B   13

#define MPEG4_FILETYPE_RAW 0
#define MPEG4_FILETYPE_MP4 1
#define MPEG4_FILETYPE_AVI 2

#define MPEG4_CODEC_DEFAULT 0
#define MPEG4_CODEC_DIVX5 1 

#ifdef __cplusplus
}
#endif