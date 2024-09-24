#pragma once
#include <bfc/platform/types.h>
#ifdef __cplusplus
extern "C" {
#endif

void RLE8(const uint8_t *rle, size_t rle_size_bytes, uint8_t *video_frame, size_t video_frame_size, int stride);
void RLE16(const uint8_t *rle, size_t rle_size_bytes, uint16_t *video_frame, size_t video_frame_size, int stride);

#ifdef __cplusplus
}
#endif