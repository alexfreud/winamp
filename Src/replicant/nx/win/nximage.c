#include "nximage.h"

static HANDLE image_heap;
void NXImageSetHeap(HANDLE _image_heap)
{
	if (!image_heap)
		image_heap = _image_heap;
}

static size_t NXImageMallocSize(size_t bytes)
{
	return sizeof(nx_image_s) + bytes - sizeof(ARGB32);
}

nx_image_t NXImageMalloc(uint32_t width, uint32_t height)
{
	size_t bytes;
	nx_image_t img;
	bytes = width*height*4; // TODO: overflow check

	img = (nx_image_t)malloc(NXImageMallocSize(bytes));
	img->ref_count = 1;
	img->len = bytes;
	img->width = width;
	img->height = height;
	return img;
	
}

nx_image_t NXImageRetain(nx_image_t image)
{
	image->ref_count++;
	return image;
}