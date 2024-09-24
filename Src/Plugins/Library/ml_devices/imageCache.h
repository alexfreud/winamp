#ifndef _NULLSOFT_WINAMP_ML_DEVICES_IMAGE_CACHE_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_IMAGE_CACHE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

typedef struct DeviceColoredImage DeviceColoredImage;
typedef struct DeviceImage DeviceImage;
typedef struct DeviceImageCache  DeviceImageCache;

typedef HBITMAP (*DeviceImageLoader)(const wchar_t* /*path*/, int /*width*/, int /*height*/, void* /*user*/);
typedef BOOL (*DeviceImageFilter)(HBITMAP /*bitmap*/, COLORREF /*color1*/, COLORREF /*color2*/, void* /*user*/);

DeviceImageCache *
DeviceImageCache_Create();

void 
DeviceImageCache_Free(DeviceImageCache *self);

DeviceImage *
DeviceImageCache_GetImage(DeviceImageCache *self, 
						  const wchar_t *path, 
						  int width, 
						  int height,
						  DeviceImageLoader loader, 
						  void *user);

size_t
DeviceImage_AddRef(DeviceImage *self);

size_t
DeviceImage_Release(DeviceImage *self);

BOOL
DeviceImage_GetSize(DeviceImage *self, 
					int *width, 
					int *height);

typedef enum DeviceImageFlags
{
	
	DeviceImage_ExactSize = (1 << 0),
	DeviceImage_AlignLeft = 0,
	DeviceImage_AlignRight = (1 << 1),
	DeviceImage_AlignHCenter = (1 << 2),
	DeviceImage_AlignTop = 0,
	DeviceImage_AlignBottom = (1 << 3),	
	DeviceImage_AlignVCenter = (1 << 4),
	DeviceImage_Normal = (DeviceImage_AlignLeft | DeviceImage_AlignTop),
} DeviceImageFlags;
DEFINE_ENUM_FLAG_OPERATORS(DeviceImageFlags);

HBITMAP
DeviceImage_GetBitmap(DeviceImage *self, DeviceImageFlags flags);

DeviceColoredImage *
DeviceImage_GetColoredImage(DeviceImage *self, 
							COLORREF color1, 
							COLORREF color2, 
							DeviceImageFilter filter, 
							void *user);

size_t
DeviceColoredImage_AddRef(DeviceColoredImage *self);

size_t
DeviceColoredImage_Release(DeviceColoredImage *self);

HBITMAP
DeviceColoredImage_GetBitmap(DeviceColoredImage *self, DeviceImageFlags flags);

DeviceImage*
DeviceColoredImage_GetBaseImage(DeviceColoredImage *self);

#endif // _NULLSOFT_WINAMP_ML_DEVICES_IMAGE_CACHE_HEADER