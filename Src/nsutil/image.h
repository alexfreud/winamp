#pragma once
#include <bfc/platform/types.h>
#include <bfc/platform/export.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NSUTIL_EXPORTS
#define NSUTIL_EXPORT __declspec(dllexport)
#else
#define NSUTIL_EXPORT __declspec(dllimport)
#endif

NSUTIL_EXPORT int nsutil_image_CopyFlipped_U8(uint8_t *destination_image, size_t destination_stride, const uint8_t *source_image, size_t source_stride, uint32_t width, uint32_t height);
NSUTIL_EXPORT int nsutil_image_Copy_U8(uint8_t *destination_image, size_t destination_stride, const uint8_t *source_image, size_t source_stride, uint32_t width, uint32_t height);
NSUTIL_EXPORT int nsutil_image_Convert_RGB24_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, const uint8_t *source_image, size_t source_stride /* bytes! */, uint32_t width, uint32_t height);
NSUTIL_EXPORT int nsutil_image_ConvertFlipped_RGB24_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, const uint8_t *source_image, size_t source_stride /* bytes! */, uint32_t width, uint32_t height);
NSUTIL_EXPORT int nsutil_image_Recolor_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, uint32_t R, uint32_t G, uint32_t B, uint32_t width, uint32_t height);
NSUTIL_EXPORT int nsutil_image_Palette_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, const uint8_t *source_image, size_t source_stride /* bytes! */, uint32_t width, uint32_t height, const RGB32 *palette);
NSUTIL_EXPORT int nsutil_image_PaletteFlipped_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, const uint8_t *source_image, size_t source_stride /* bytes! */, uint32_t width, uint32_t height, const RGB32 *palette);
NSUTIL_EXPORT int nsutil_image_FillRectAlpha_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, uint32_t width, uint32_t height, RGB32 color, int alpha);
NSUTIL_EXPORT int nsutil_image_Convert_YUV420_RGB32(RGB32 *destination_image, size_t destination_stride /* bytes! */, uint32_t width, uint32_t height, const uint8_t *planes[3], const size_t strides[3]);
#ifdef __cplusplus
}
#endif