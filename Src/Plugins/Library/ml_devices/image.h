#ifndef _NULLSOFT_WINAMP_ML_DEVICES_IMAGE_HEADER
#define _NULLSOFT_WINAMP_ML_DEVICES_IMAGE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define IMAGE_FILTER_NORMAL		0x00000000
#define IMAGE_FILTER_GRAYSCALE	0x00010000
#define IMAGE_FILTER_BLEND		0x00020000
#define IMAGE_FILTER_MASK		0xFFFF0000


typedef struct ImageInfo
{
	unsigned int width;
	unsigned int height;
	const wchar_t *path;
} ImageInfo;


HBITMAP
Image_Load(const wchar_t *path,
		   unsigned int type, 
		   unsigned int flags,
		   int width,
		   int height);

HBITMAP
Image_LoadEx(HINSTANCE instance, 
		   const wchar_t *path,
		   unsigned int type, 
		   unsigned int flags,
		   int width,
		   int height);

HBITMAP 
Image_LoadSkinned(const wchar_t *path,
				  unsigned int type,
				  unsigned int flags, //ISF_XXX + IMAGE_FILTER_XXX
				  int width,
				  int height,
				  COLORREF backColor, 
				  COLORREF frontColor,
				  COLORREF blendColor); // only valid if IMAGE_FILTER_BLEND set

HBITMAP 
Image_LoadSkinnedEx(HINSTANCE instance, 
				  const wchar_t *path,
				  unsigned int type,
				  unsigned int flags, //ISF_XXX + IMAGE_FILTER_XXX
				  int width,
				  int height,
				  COLORREF backColor, 
				  COLORREF frontColor,
				  COLORREF blendColor); // only valid if IMAGE_FILTER_BLEND set

BOOL
Image_FilterEx(void *pixelData,
			   long width,
			   long height,
			   unsigned short bpp, 
			   unsigned int flags, 
			   COLORREF backColor, 
			   COLORREF frontColor, 
			   COLORREF blendColor);

BOOL
Image_Filter(HBITMAP bitmap, 
			 unsigned int flags, 
			 COLORREF backColor, 
			 COLORREF frontColor, 
			 COLORREF blendColor);

BOOL
Image_BlendEx(void *pixelData,
			  long width,
			  long height,
			  unsigned short bpp,
			  COLORREF blendColor);
BOOL
Image_Blend(HBITMAP bitmap,
			COLORREF blendColor);

HBITMAP
Image_DuplicateDib(HBITMAP source);

BOOL 
Image_ColorOver(HBITMAP hbmp, 
				const RECT *prcPart, 
				BOOL premult, 
				COLORREF rgb);

BOOL 
Image_ColorOverEx(unsigned char *pPixels, 
				  int bitmapCX, 
				  int bitmapCY, 
				  long x, 
				  long y, 
				  long cx, 
				  long cy, 
				  unsigned short bpp, 
				  BOOL premult, 
				  COLORREF rgb);

BOOL 
Image_Premultiply(HBITMAP hbmp, 
				  const RECT *prcPart);

BOOL 
Image_PremultiplyEx(unsigned char *pPixels, 
					int bitmapCX, 
					int bitmapCY, 
					long x, 
					long y, 
					long cx, 
					long cy, 
					unsigned short bpp);

BOOL 
Image_Demultiply(HBITMAP hbmp, 
				  const RECT *prcPart);

BOOL 
Image_DemultiplyEx(unsigned char *pPixels, 
					int bitmapCX, 
					int bitmapCY, 
					long x, 
					long y, 
					long cx, 
					long cy, 
					unsigned short bpp);

BOOL 
Image_Saturate(HBITMAP hbmp, 
			   const RECT *prcPart, 
			   int n, 
			   BOOL fScale);

BOOL 
Image_SaturateEx(unsigned char *pPixels, 
				 int bitmapCX, 
				 int bitmapCY, 
				 long x, 
				 long y, 
				 long cx, 
				 long cy, 
				 unsigned short bpp, 
				 int n, 
				 BOOL fScale);

BOOL 
Image_AdjustAlpha(HBITMAP hbmp, 
				  const RECT *prcPart, 
				  int n, 
				  BOOL fScale);

BOOL 
Image_AdjustAlphaEx(unsigned char *pPixels, 
					int bitmapCX, 
					int bitmapCY, 
					long x, 
					long y, 
					long cx, 
					long cy, 
					unsigned short bpp, 
					int n, 
					BOOL fScale);

BOOL 
Image_AdjustSaturationAlpha(HBITMAP hbmp, 
							const RECT *prcPart, 
							int nSaturation, 
							int nAlpha);

BOOL 
Image_AdjustSaturationAlphaEx(unsigned char *pPixels, 
							  int bitmapCX, 
							  int bitmapCY, 
							  long x, 
							  long y, 
							  long cx, 
							  long cy, 
							  unsigned short bpp, 
							  int nSaturation, 
							  int nAlpha);

BOOL
Image_FillBorder(HDC targetDC, 
				 const RECT *targetRect, 
				 HDC sourceDC, 
				 const RECT *sourceRect, 
				 BOOL fillCenter, 
				 BYTE alphaConstant);


const ImageInfo *
Image_GetBestFit(const ImageInfo *images, 
				 size_t count, 
				 unsigned int width, 
				 unsigned int height);

typedef enum AlphaBlendFlags
{
	AlphaBlend_Normal = 0,
	AlphaBlend_ScaleSource = (1 << 0),
	AlphaBlend_AlignLeft = (1 << 1),
	AlphaBlend_AlignRight = (1 << 2),
	AlphaBlend_AlignCenter = 0,
	AlphaBlend_AlignTop = (1 << 3),
	AlphaBlend_AlignBottom = (1 << 4),
	AlphaBlend_AlignVCenter = 0,
}AlphaBlendFlags;
DEFINE_ENUM_FLAG_OPERATORS(AlphaBlendFlags);

BOOL
Image_AlphaBlend(HDC targetDC, 
				 const RECT *targetRect,
				 HDC sourceDC, 
				 const RECT *sourceRect, 
				 BYTE sourceAlpha, 
				 HBITMAP sourceBitmap, 
				 const RECT *paintRect, 
				 AlphaBlendFlags flags, 
				 RECT *rectOut);

#endif //_NULLSOFT_WINAMP_ML_DEVICES_IMAGE_HEADER
