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

NSUTIL_EXPORT int nsutil_alpha_Premultiply_RGB32(void *image, size_t image_stride, int width, int height);
NSUTIL_EXPORT int nsutil_alpha_PremultiplyValue_RGB8(void *image, size_t image_stride, int width, int height, uint8_t alpha);

#ifdef __cplusplus
}
#endif