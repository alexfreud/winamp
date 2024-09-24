#pragma once
typedef struct image_cache
{
	int size_x, size_y;
	struct video_image *head;
} ImageCache;

void image_cache_set_dimensions(ImageCache *cache, int width, int height);
int image_cache_dimensions_match(ImageCache *cache, int width, int height);
void image_cache_add(ImageCache *cache, struct video_image *image);
struct video_image *image_cache_get(ImageCache *cache);
void image_cache_flush(ImageCache *cache);

typedef struct motion_cache
{
	int size_x, size_y;
	struct pic_motion **head;
} MotionCache;

void motion_cache_set_dimensions(MotionCache *cache, int width, int height);
int motion_cache_dimensions_match(MotionCache *cache, int width, int height);
void motion_cache_add(MotionCache *cache, struct pic_motion **image);
struct pic_motion **motion_cache_get(MotionCache *cache);
void motion_cache_flush(MotionCache *cache);