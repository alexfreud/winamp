#include "main.h"
#include "./imageCache.h"
#include <wincodec.h>
#include <vector>
#include <algorithm>


struct DeviceColoredImage
{
	size_t	 ref;
	DeviceImage *base;
	HBITMAP  bitmap;
	COLORREF color1;
	COLORREF color2;
	DeviceImageFilter filter;
	void *filterParam;
};

typedef std::vector<DeviceColoredImage*> DeviceColoredImageList;

struct DeviceImage
{
	size_t  ref;
	DeviceImageCache *cache;
	wchar_t *source;
	HBITMAP bitmap;
	HBITMAP exactBitmap;
	int width;
	int height;
	DeviceImageLoader loader;
	void *loaderParam;
	DeviceColoredImageList list;
};
typedef std::vector<DeviceImage*> DeviceImageList;

struct DeviceImageCache
{
	DeviceImageList list;
	IWICImagingFactory *wicFactory;
};

typedef struct DeviceImageSeachParam
{
	const wchar_t *path;
	int width;
	int height;
} DeviceImageSeachParam;

typedef struct DeviceColoredImageSeachParam
{
	COLORREF color1;
	COLORREF color2;
} DeviceColoredImageSeachParam;


DeviceImageCache *
DeviceImageCache_Create()
{
	DeviceImageCache *self;

	self = new DeviceImageCache();
	if (NULL == self)
		return NULL;

	self->wicFactory = NULL;

	return self;
}


void 
DeviceImageCache_Free(DeviceImageCache *self)
{
	if (NULL == self)
		return;

	size_t index = self->list.size();
	while(index--)
	{
		DeviceImage *image = self->list[index];
		image->cache = NULL;
	}

	if (NULL != self->wicFactory)
		self->wicFactory->Release();

	delete(self);
}


static DeviceImage *
DeviceImage_Create(DeviceImageCache *cache, const wchar_t *path, int width, int height, 
				   DeviceImageLoader loader, void *loaderParam)
{
	DeviceImage *self;

	if (NULL == loader)
		return NULL;

	self = new DeviceImage();
	if (NULL == self)
		return NULL;
	
	self->ref = 1;
	self->cache = cache;
	self->source = ResourceString_Duplicate(path);
	self->loader = loader;
	self->loaderParam = loaderParam;
	self->bitmap = NULL;
	self->exactBitmap = NULL;
	self->width = width;
	self->height = height;
	
	return self;
}

static void
DeviceImage_Free(DeviceImage *self)
{
	if (NULL == self)
		return;

	if (NULL != self->source)
		ResourceString_Free(self->source);

	if (NULL != self->bitmap)
		DeleteObject(self->bitmap);

	if (NULL != self->exactBitmap && 
		self->exactBitmap != self->bitmap)
	{
		DeleteObject(self->exactBitmap);
	}

	delete(self);
}

static int 
DeviceImageCache_SearchCb(const void *key, const void *element)
{
	DeviceImageSeachParam *search;
	DeviceImage *image;
	int result;

	search = (DeviceImageSeachParam*)key;
	image = (DeviceImage*)element;
	
	result = search->height - image->height;
	if (0 != result)
		return result;

	result = search->width - image->width;
	if (0 != result)
		return result;
	
	if (FALSE != IS_INTRESOURCE(search->path) ||
		FALSE != IS_INTRESOURCE(image->source))
	{
		return (int)(INT_PTR)(search->path - image->source);
	}

	return CompareString(CSTR_INVARIANT, 0, search->path, -1, image->source, -1) - 2;
}

static int
DeviceImageCache_SortCb(const void *element1, const void *element2)
{
	DeviceImage *image1;
	DeviceImage *image2;
	int result;

	image1 = (DeviceImage*)element1;
	image2 = (DeviceImage*)element2;

	result = image1->height - image2->height;
	if (0 != result)
		return result;

	result = image1->width - image2->width;
	if (0 != result)
		return result;

	if (FALSE != IS_INTRESOURCE(image1->source) ||
		FALSE != IS_INTRESOURCE(image2->source))
	{
		return (int)(INT_PTR)(image1->source - image2->source);
	}

	return CompareString(CSTR_INVARIANT, 0, image1->source, -1, image2->source, -1) - 2;
}
static int
DeviceImageCache_SortCb_V2(const void* element1, const void* element2)
{
	return DeviceImageCache_SortCb(element1, element2) < 0;
}


static HBITMAP
DeviceImage_DefaultImageLoader(const wchar_t *path, int width, int height, void *param)
{
	HBITMAP bitmap;
	unsigned int flags;
	
	flags = ISF_PREMULTIPLY;
	if (FALSE == IS_INTRESOURCE(path))
		flags |= ISF_LOADFROMFILE;

	bitmap = Image_Load(path, SRC_TYPE_PNG, flags, 0, 0);

	return bitmap;
}

DeviceImage *
DeviceImageCache_GetImage(DeviceImageCache *self, const wchar_t *path, int width, int height, DeviceImageLoader loader, void *user)
{
	DeviceImage *image, *image_ptr = 0;
	DeviceImageSeachParam searchParam;

	if (width < 1)
		width = 0;
	
	if (height < 1)
		height = 0;

	if (NULL == self)
		return NULL;

	if (NULL == path ||
		(FALSE == IS_INTRESOURCE(path) && L'\0' == *path))
	{
		return NULL;
	}

	searchParam.height = height;
	searchParam.width = width;
	searchParam.path = path;

	//image_ptr = (DeviceImage**)bsearch(&searchParam, &self->list[0], self->list.size(),
	//									sizeof(DeviceImage**),
	//									DeviceImageCache_SearchCb);
	auto it = std::find_if(self->list.begin(), self->list.end(),
		[&](DeviceImage* upT) -> bool
		{
			return DeviceImageCache_SearchCb(&searchParam, upT) == 0;
		}
	);
	if (it != self->list.end())
	{
		image_ptr = *it;
	}


	if (NULL != image_ptr)
	{
		image = image_ptr;
		DeviceImage_AddRef(image);
		return image;
	}
	
	if (NULL == loader)
		loader = DeviceImage_DefaultImageLoader;

	image = DeviceImage_Create(self, path, width, height, loader, user);
	if (NULL != image)
	{
		self->list.push_back(image);
		//qsort(&self->list[0], self->list.size(), sizeof(DeviceImage**), DeviceImageCache_SortCb);
		std::sort(self->list.begin(), self->list.end(), DeviceImageCache_SortCb_V2);
	}

	return image;
}

static BOOL
DeviceImageCache_RemoveImage(DeviceImageCache *self, DeviceImage *image)
{
	size_t index;

	if (NULL == self || NULL == image)
		return FALSE;

	index = self->list.size();
	while(index--)
	{
		if (self->list[index] == image)
		{
			self->list.erase(self->list.begin() + index);
			return TRUE;
		}
	}

	return FALSE;
}
size_t
DeviceImage_AddRef(DeviceImage *self)
{
	if (NULL == self)
		return 0;
	return InterlockedIncrement((LONG*)&self->ref);
}

size_t
DeviceImage_Release(DeviceImage *self)
{
	size_t r;
	if (NULL == self || 0 == self->ref)
		return 0;
	
	r = InterlockedDecrement((LONG*)&self->ref);
	if (0 == r)
	{
		if (NULL != self->cache)
			DeviceImageCache_RemoveImage(self->cache, self);
		DeviceImage_Free(self);
		return 0;
	}

	return r;
}

BOOL
DeviceImage_GetSize(DeviceImage *self, int *width, int *height)
{
	if (NULL == self)
		return FALSE;

	if (NULL != width)
		*width = self->width;
	
	if (NULL != height)
		*height = self->height;

	return TRUE;
}

static IWICImagingFactory*
DeviceImage_GetWicFactory(DeviceImageCache *cache)
{
	IWICImagingFactory *wicFactory;
	HRESULT hr;
	
	if (NULL != cache &&
		NULL != cache->wicFactory)
	{
		cache->wicFactory->AddRef();
		return cache->wicFactory;
	}
	
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, 
			IID_PPV_ARGS(&wicFactory));
	
	if (FAILED(hr))
		return NULL;

	if (NULL != cache)
	{
		wicFactory->AddRef();
		cache->wicFactory = wicFactory;
	}
	
	return wicFactory;
}


static HBITMAP
DeviceImage_HBitmapFromWicSource(IWICBitmapSource *wicSource, unsigned int targetWidth, 
								unsigned int targetHeight, DeviceImageFlags flags)
{
	HRESULT hr;
    HBITMAP bitmap;
	BITMAPINFO bitmapInfo;
	unsigned int width, height, bitmapWidth, bitmapHeight;
	void *pixelData = NULL;
    WICPixelFormatGUID pixelFormat;
	HDC windowDC;
	unsigned int strideSize, imageSize;

	if (NULL == wicSource)
		return NULL;

	hr = wicSource->GetPixelFormat(&pixelFormat);
	if (FAILED(hr) ||
		(GUID_WICPixelFormat32bppPBGRA != pixelFormat &&
		GUID_WICPixelFormat32bppBGR != pixelFormat &&
		GUID_WICPixelFormat32bppBGRA != pixelFormat &&
		GUID_WICPixelFormat32bppRGBA != pixelFormat &&
		GUID_WICPixelFormat32bppPRGBA != pixelFormat))
	{
		return NULL;
	}
    
    hr = wicSource->GetSize(&width, &height);
	if (FAILED(hr))
		return NULL;
    
	if (0 != (DeviceImage_ExactSize & flags))
	{
		bitmapWidth = (targetWidth > width) ? targetWidth : width;
		bitmapHeight = (targetHeight > height) ? targetHeight : height;
	}
	else
	{
		bitmapWidth = width;
		bitmapHeight = height;
	}
	
	ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));
    bitmapInfo.bmiHeader.biSize  = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = bitmapWidth;
    bitmapInfo.bmiHeader.biHeight = -(LONG)bitmapHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

	windowDC = GetDCEx(NULL, NULL, DCX_WINDOW | DCX_CACHE);
 
	bitmap = CreateDIBSection(windowDC, &bitmapInfo, DIB_RGB_COLORS, &pixelData, NULL, 0);
		
	if (NULL != windowDC)
		ReleaseDC(NULL, windowDC);

	if (NULL == bitmap)
		return NULL;

    hr = UIntMult(bitmapWidth, sizeof(DWORD), &strideSize);
    if (SUCCEEDED(hr))
	{
		if (0 != (DeviceImage_ExactSize & flags) &&
			(bitmapWidth > width || bitmapHeight > height))
		{
			if (SUCCEEDED(UIntMult(strideSize, bitmapHeight, &imageSize)))
				ZeroMemory(pixelData, imageSize);
		}

		hr = UIntMult(strideSize, height, &imageSize);
    	if (SUCCEEDED(hr))
		{
			unsigned int offset, delta;

			offset = 0;

			if (0 != (DeviceImage_AlignVCenter & flags))
			{
				delta = bitmapHeight - height;
				delta = delta/2 + delta%2;
			}
			else if (0 != (DeviceImage_AlignBottom & flags))
				delta = bitmapHeight - height;
			else
				delta = 0;

			if (0 != delta && SUCCEEDED(UIntMult(delta, strideSize, &delta)))
				offset += delta;
						
			if (0 != (DeviceImage_AlignHCenter & flags))
			{
				delta = bitmapWidth - width;
				delta = delta/2;
			}
			else if (0 != (DeviceImage_AlignRight & flags))
				delta = bitmapWidth - width;
			else 
				delta = 0;

			if (0 != delta && SUCCEEDED(UIntMult(delta, sizeof(DWORD), &delta)))
				offset += delta;
			
			hr = wicSource->CopyPixels(NULL, strideSize, imageSize, ((BYTE*)pixelData) + offset);
		}
    }
    
	if (FAILED(hr))
	{
		DeleteObject(bitmap);
		bitmap = NULL;
	}
    
	return bitmap;
}

static BOOL
DeviceImage_ScaleBitmap(HBITMAP sourceBitmap, int width, int height, DeviceImageCache *cache, 
						DeviceImageFlags flags, HBITMAP *scalledBitmap)
{
	HBITMAP resultBitmap;
	BITMAP sourceInfo;
	IWICImagingFactory *wicFactory;
	unsigned int requestedWidth, requestedHeight;
	double scale, scaleH;
	int t;

	if (NULL == sourceBitmap || NULL == scalledBitmap)
		return FALSE;

	if (sizeof(sourceInfo) != GetObject(sourceBitmap, sizeof(sourceInfo), &sourceInfo))
		return FALSE;
	
	if (sourceInfo.bmHeight < 0)
		sourceInfo.bmHeight = -sourceInfo.bmHeight;

	scale = (double)width / sourceInfo.bmWidth;
	scaleH = (double)height / sourceInfo.bmHeight;
				
	if (scale > scaleH)
		scale = scaleH;

	if (1.0 == scale)
	{
		*scalledBitmap = NULL;
		return TRUE;
	}

	requestedWidth = width;
	requestedHeight = height;

	t = (int)((sourceInfo.bmWidth * scale) + 0.5);
	if (t < width)
		width = t;

	t = (int)((sourceInfo.bmHeight * scale) + 0.5);
	if (t < height)
		height = t;
	
	
	resultBitmap = NULL;

	wicFactory = DeviceImage_GetWicFactory(cache);
	if (NULL != wicFactory)
	{
		HRESULT hr;
		IWICBitmap *wicBitmap;
		
		hr = wicFactory->CreateBitmapFromHBITMAP(sourceBitmap, NULL, 
							WICBitmapUsePremultipliedAlpha, &wicBitmap);
		if (SUCCEEDED(hr))
		{
			IWICBitmapScaler *wicScaler;	
			hr = wicFactory->CreateBitmapScaler(&wicScaler);
			if (SUCCEEDED(hr))
			{
				hr = wicScaler->Initialize(wicBitmap, width, height, WICBitmapInterpolationModeFant);
					if (SUCCEEDED(hr))
				{
					resultBitmap = DeviceImage_HBitmapFromWicSource(wicScaler, 
										requestedWidth, requestedHeight, flags);
				}
				wicScaler->Release();
				
			}
			wicBitmap->Release();
		}

		wicFactory->Release();
	}

	*scalledBitmap = resultBitmap;
	return (NULL != resultBitmap);
}

static HBITMAP
DeviceImage_GetExactBitmap(DeviceImage *self, DeviceImageFlags flags)
{
	if (NULL == self)
		return NULL;

	if (NULL == self->exactBitmap)
	{		
		if (NULL != self->bitmap)
		{
			BITMAP bi;
			if (sizeof(bi) == GetObject(self->bitmap, sizeof(bi), &bi) &&
				bi.bmWidth == self->width && bi.bmHeight == self->height)
			{
				self->exactBitmap = self->bitmap;
				return self->exactBitmap;
			}
		}

		if (NULL != self->loader)
		{
			HBITMAP bitmap;
			bitmap = self->loader(self->source, self->width, self->height, self->loaderParam);
			if (NULL != bitmap)
			{				
				if (FALSE != DeviceImage_ScaleBitmap(bitmap, self->width, self->height, 
											self->cache, flags, &self->exactBitmap))
				{
				
					if (NULL == self->exactBitmap)
					{
						self->exactBitmap = bitmap;
						bitmap = NULL;
					}
				}
				
				if (NULL != bitmap)
					DeleteObject(bitmap);
			}
		}
	}

	return self->exactBitmap;
}


HBITMAP
DeviceImage_GetBitmap(DeviceImage *self, DeviceImageFlags flags)
{
	if (NULL == self)
		return NULL;

	if (0 != (DeviceImage_ExactSize & flags))
		return DeviceImage_GetExactBitmap(self, flags);

	if (NULL == self->bitmap)
	{		
		if (NULL != self->loader)
		{
			HBITMAP bitmap;
			bitmap = self->loader(self->source, self->width, self->height, self->loaderParam);
			if (NULL != bitmap)
			{				
				if (FALSE == DeviceImage_ScaleBitmap(bitmap, self->width, self->height, 
											self->cache, flags, &self->bitmap))
				{
					self->bitmap = NULL;
				}

				if (NULL != self->bitmap)
					DeleteObject(bitmap);
				else
					self->bitmap = bitmap;
			}
		}
	}

	return self->bitmap;
}


static DeviceColoredImage *
DeveiceColoredImage_Create(DeviceImage *base, COLORREF color1, COLORREF color2, 
							DeviceImageFilter filter, void *user)
{
	DeviceColoredImage *coloredImage;

	if (NULL == base)
		return NULL;
	
	coloredImage = (DeviceColoredImage*)malloc(sizeof(DeviceColoredImage));
	if (NULL == coloredImage)
		return NULL;

	ZeroMemory(coloredImage, sizeof(DeviceColoredImage));

	coloredImage->ref = 1;
	coloredImage->base = base;
	coloredImage->color1 = color1;
	coloredImage->color2 = color2;
	coloredImage->filter = filter;
	coloredImage->filterParam = user;

	DeviceImage_AddRef(base);

	return coloredImage;
}

static void
DeviceColoredImage_Free(DeviceColoredImage *self)
{
	if (NULL == self)
		return;

	if (NULL != self->bitmap)
		DeleteObject(self->bitmap);
	
	free(self);
}

static int 
DeviceColoredImage_SearchCb(const void *key, const void *element)
{
	DeviceColoredImageSeachParam *search;
	DeviceColoredImage *image;
	int result;

	search = (DeviceColoredImageSeachParam*)key;
	image = (DeviceColoredImage*)element;
	
	result = search->color1 - image->color1;
	if (0 != result)
		return result;

	return search->color2- image->color2;
	
}

static int
DeviceColoredImage_SortCb(const void *element1, const void *element2)
{
	DeviceColoredImage *image1;
	DeviceColoredImage *image2;
	int result;

	image1 = (DeviceColoredImage*)element1;
	image2 = (DeviceColoredImage*)element2;

	result = image1->color1 - image2->color1;
	if (0 != result)
		return result;

	return image1->color2- image2->color2;
}

static int
DeviceColoredImage_SortCb_V2(const void* element1, const void* element2)
{
	return DeviceColoredImage_SortCb(element1, element2) < 0;
}

DeviceColoredImage *
DeviceImage_GetColoredImage(DeviceImage *self,  COLORREF color1, COLORREF color2, 
							DeviceImageFilter filter, void *user)
{
	size_t listSize;
	DeviceColoredImage *image;
	DeviceColoredImageSeachParam searchParam;

	searchParam.color1 = color1;
	searchParam.color2 = color2;
	
	listSize = self->list.size();
	if (listSize > 0)
	{
		DeviceColoredImage* image_ptr = NULL;
		//DeviceColoredImage **image_ptr = (DeviceColoredImage**)bsearch(&searchParam, &self->list[0], listSize,
		//									sizeof(DeviceColoredImage**),
		//									DeviceColoredImage_SearchCb);

		auto it = std::find_if(self->list.begin(), self->list.end(),
			[&](DeviceColoredImage* upT) -> bool
			{
				return DeviceColoredImage_SearchCb(&searchParam, upT) == 0;
			}
		);
		if (it != self->list.end())
		{
			image_ptr = *it;
		}

		if (NULL != image_ptr)
		{
			image = image_ptr;
			DeviceColoredImage_AddRef(image);
			return image;
		}
	}

	image = DeveiceColoredImage_Create(self, color1, color2, filter, user);
	if (NULL == image)
		return NULL;

	self->list.push_back(image);
	listSize++;

	if (listSize > 1)
	{
		//qsort(&self->list[0], self->list.size(), sizeof(DeviceColoredImage**),
		//	DeviceColoredImage_SortCb);
		std::sort(self->list.begin(), self->list.end(), DeviceColoredImage_SortCb_V2);
	}

	return image;
}

static void
DeviceImage_RemoveColored(DeviceImage *self, DeviceColoredImage *coloredImage)
{
	size_t index;

	if (NULL == self || NULL == coloredImage)
		return;

	index = self->list.size();
	while(index--)
	{
		if (coloredImage == self->list[index])
		{
			self->list.erase(self->list.begin() + index);
		}
	}
}

size_t
DeviceColoredImage_AddRef(DeviceColoredImage *self)
{
	if (NULL == self)
		return 0;
	return InterlockedIncrement((LONG*)&self->ref);
}

size_t
DeviceColoredImage_Release(DeviceColoredImage *self)
{
	size_t r;
	if (NULL == self || 0 == self->ref)
		return 0;
	
	r = InterlockedDecrement((LONG*)&self->ref);
	if (0 == r)
	{
		if (NULL != self->base)
		{
			DeviceImage_RemoveColored(self->base, self);
			DeviceImage_Release(self->base);
		}
		DeviceColoredImage_Free(self);
		return 0;
	}

	return r;
}

static BOOL
DeviceColoredImage_DefaultFilter(HBITMAP bitmap, COLORREF color1, COLORREF color2, void *user)
{
	DIBSECTION bitmapDib;
	BITMAP *bitmapInfo;
	MLIMAGEFILTERAPPLYEX filter;
	
	if (sizeof(bitmapDib) != GetObjectW(bitmap, sizeof(bitmapDib), &bitmapDib))
		return FALSE;

	bitmapInfo = &bitmapDib.dsBm;
		
	filter.cbSize = sizeof(filter);
	filter.pData = (BYTE*)bitmapInfo->bmBits;
	filter.cx = bitmapInfo->bmWidth;
	filter.cy = bitmapInfo->bmHeight;
	filter.bpp = bitmapInfo->bmBitsPixel;
	filter.imageTag = NULL;

	filter.filterUID = MLIF_GRAYSCALE_UID;
	MLImageFilter_ApplyEx(Plugin_GetLibraryWindow(), &filter);

	filter.rgbBk = color1;
	filter.rgbFg = color2;

	if (32 == bitmapInfo->bmBitsPixel)
	{
		filter.filterUID = MLIF_FILTER1_PRESERVE_ALPHA_UID;
		MLImageFilter_ApplyEx(Plugin_GetLibraryWindow(), &filter);
	}
	else
	{
		filter.filterUID = MLIF_FILTER1_UID;
		MLImageFilter_ApplyEx(Plugin_GetLibraryWindow(), &filter);
	}

	return TRUE;
}

HBITMAP
DeviceColoredImage_GetBitmap(DeviceColoredImage *self, DeviceImageFlags flags)
{
	if (NULL == self)
		return NULL;

	if (NULL == self->bitmap)
	{
		HBITMAP  bitmap;
		bitmap = DeviceImage_GetBitmap(self->base, flags);
		if (NULL != bitmap)
		{
			self->bitmap = Image_DuplicateDib(bitmap);
			if (NULL != self->bitmap)
			{
				DeviceImageFilter filter;
				if (NULL == self->filter)
					filter = DeviceColoredImage_DefaultFilter;
				else
					filter = self->filter;
			
				Image_Demultiply(self->bitmap, NULL);
				filter(self->bitmap, self->color1, self->color2, self->filterParam);
				Image_Premultiply(self->bitmap, NULL);
			}				
		}
	}

	return self->bitmap;
}

DeviceImage*
DeviceColoredImage_GetBaseImage(DeviceColoredImage *self)
{
	if (NULL == self || NULL == self->base)
		return NULL;

	DeviceImage_AddRef(self->base);
	return self->base;
}