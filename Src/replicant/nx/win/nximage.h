#pragma once
#include "foundation/types.h"
#include "nxapi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nx_image
{
	size_t ref_count;
	uint32_t width;
	uint32_t height;
	size_t len;
	ARGB32 image[1];
} nx_image_s, *nx_image_t;

NX_API void NXImageSetHeap(HANDLE image_heap);
NX_API nx_image_t NXImageMalloc(uint32_t width, uint32_t height);
NX_API nx_image_t NXImageRetain(nx_image_t image);

#ifdef __cplusplus
}
#endif