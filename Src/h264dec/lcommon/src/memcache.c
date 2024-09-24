#include "memcache.h"
#include "mbuffer.h"
#include "memalloc.h"

void image_cache_flush(ImageCache *cache)
{
	while (cache->head)
	{
		VideoImage *next = cache->head->next;
		free_memImage(cache->head);
		cache->head = next;
	}
	cache->size_x = 0;
	cache->size_y = 0;
}

void image_cache_set_dimensions(ImageCache *cache, int width, int height)
{
	if (width != cache->size_x || height != cache->size_y)
	{
		image_cache_flush(cache);
		cache->size_x = width;
		cache->size_y = height;
	}
}

int image_cache_dimensions_match(ImageCache *cache, int width, int height)
{
	if (width != cache->size_x || height != cache->size_y)
		return 0;

	return 1;
}

void image_cache_add(ImageCache *cache, VideoImage *image)
{
	image->next = cache->head;
	cache->head = image;
}

struct video_image *image_cache_get(ImageCache *cache)
{
	if (cache->head)
	{
		VideoImage *ret = cache->head;
		cache->head = ret->next;
		ret->next = 0;
		return ret;
	}
	return 0;
}

/* ------------- 

PicMotion arrays are allowed with one extra slot in the first dimension
which we use as the next pointer
------------- */


void motion_cache_flush(MotionCache *cache)
{
	while (cache->head)
	{
		PicMotion **next = (PicMotion **)cache->head[cache->size_y];
		free_mem2DPicMotion(cache->head);
		cache->head = next;
	}
	cache->size_x = 0;
	cache->size_y = 0;
}

void motion_cache_set_dimensions(MotionCache *cache, int width, int height)
{
	if (width != cache->size_x || height != cache->size_y)
	{
		motion_cache_flush(cache);
		cache->size_x = width;
		cache->size_y = height;
	}
}

int motion_cache_dimensions_match(MotionCache *cache, int width, int height)
{
	if (width != cache->size_x || height != cache->size_y)
		return 0;

	return 1;
}

void motion_cache_add(MotionCache *cache, PicMotion **image)
{
	image[cache->size_y] = (PicMotion *)cache->head;
	cache->head = image;
}

struct pic_motion **motion_cache_get(MotionCache *cache)
{
	if (cache->head)
	{
		PicMotion **ret = cache->head;
		cache->head = (PicMotion **)ret[cache->size_y];
		ret[cache->size_y] = 0;
		return ret;
	}
	return 0;
}
