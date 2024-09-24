#include "osx_bitmap_cgimage.h"

SkinBitmap::SkinBitmap(ARGB32 *_bits, int w, int h) : image(0)
{
  // TODO: allow a mechanism for SkinBitmap to take ownership of the data
  bits = malloc(w*h*4);
  if (bits)
  {
    memcpy(bits, _bits, w*h*4);
    CGColorSpaceRef colorSpace =  CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    imageContext = CGBitmapContextCreate(bits, w, h, 8, w*4, colorSpace, kCGBitmapByteOrder32Little|kCGImageAlphaPremultipliedFirst);
    image = CGBitmapContextCreateImage(imageContext);
    CGColorSpaceRelease(colorSpace);
  }
}

SkinBitmap::~SkinBitmap()
{
  CGImageRelease(image);
#ifndef SKINBITMAP_USE_CGIMAGE  
  free(bits);
  CGContextRelease(imageContext);
#endif
}

#ifdef WASABI_COMPILE_IMGLDR
SkinBitmap::SkinBitmap(const wchar_t *elementname, int _cached) 
{
  ASSERT(elementname!= NULL);
  
  bitmapname = elementname;
  x_offset = -1;
  y_offset = -1;
  subimage_w = -1;
  subimage_h = -1;
  fullimage_w=fullimage_h=0;
  ownbits=1;
  bits = NULL;
  fromskin = 0;
  last_failed = 0;
#ifdef WASABI_COMPILE_SKIN
  bits = WASABI_API_IMGLDR->imgldr_requestSkinBitmap(elementname, &has_alpha, &x_offset, &y_offset, &subimage_w, &subimage_h, &fullimage_w, &fullimage_h,_cached);
  fromskin = (bits != NULL);
#endif
  if (bits == NULL) 
		bits = WASABI_API_IMGLDR->imgldr_makeBmp(elementname, &has_alpha, &fullimage_w, &fullimage_h);
#ifdef WASABI_COMPILE_SKIN
  if (bits == NULL) 
	{
		bits = WASABI_API_IMGLDR->imgldr_requestSkinBitmap(ERRORBMP, &has_alpha, &x_offset, &y_offset, &subimage_w, &subimage_h, &fullimage_w, &fullimage_h,_cached);
    last_failed = 1;
	}
#endif
  if (bits == NULL)
	{
    bits = WASABI_API_IMGLDR->imgldr_makeBmp(HARDERRORBMP, &has_alpha, &fullimage_w, &fullimage_h);
    last_failed = 1;
  }
  
  // check that coordinates are correct
  if(x_offset!=-1 && x_offset>fullimage_w) x_offset=fullimage_w-1;
  if(y_offset!=-1 && y_offset>fullimage_h) y_offset=fullimage_h-1;
  if(subimage_w!=-1 && (x_offset+subimage_w)>fullimage_w) subimage_w=fullimage_w-x_offset;
  if(subimage_h!=-1 && (y_offset+subimage_h)>fullimage_h) subimage_h=fullimage_h-y_offset;
  
  // ASSERTPR(bits != NULL, elementname);
  if (bits == NULL) {
    DebugString("element not found ! %s\n", elementname);
    int n = 10*10;
    bits = (ARGB32 *)WASABI_API_MEMMGR->sysMalloc(n * 4);
    
    
    ARGB32 *p = bits;
    while (n--)
      *p++ = 0xFFFF00FF;
  }
}
#endif


int SkinBitmap::getWidth()
{
  if (!image)
    return 0;

  return CGImageGetWidth(image);
}

int SkinBitmap::getFullWidth()
{
  if (!image)
    return 0;

  return CGImageGetBytesPerRow(image)/4; // assumes 32bit pixel data
}

int SkinBitmap::getHeight()
{
  if (!image)
    return 0;
  return CGImageGetHeight(image);
}

void SkinBitmap::blit(api_canvas *canvas, int x, int y)
{
  if (!image)
    return;
  
  CGContextRef context = canvas->getHDC();
  CGRect rect = CGRectMake(x, y, getWidth(), getHeight());
  CGContextDrawImage(context, rect, image);
}

void SkinBitmap::blitAlpha(api_canvas *canvas, int x, int y, int alpha)
{
  if (!image)
    return;
  
  float floatAlpha = alpha / 255.f;
  
  CGContextRef context = canvas->getHDC();
  CGContextSaveGState(context);
  
//  CGContextTranslateCTM(context, 0, r->bottom);
//  CGContextScaleCTM(context, 1.0, -1.0);
  
  CGContextSetAlpha(context, floatAlpha);
  CGRect rect = CGRectMake(x, y, getWidth(), getHeight());
  CGContextDrawImage(context, rect, image);
  CGContextRestoreGState(context);
}

void SkinBitmap::stretchToRect(api_canvas *canvas, RECT *r)
{
  if (!image)
    return;
  
  CGContextRef context = canvas->getHDC();
  CGContextSaveGState(context);
  
  CGContextTranslateCTM (context, 0, r->bottom);
  CGContextScaleCTM(context, 1.0, -1.0);
  
  CGRect rect = CGRectMake(r->left, r->top, r->right-r->left, r->bottom-r->top);
  CGContextDrawImage(context, rect, image);
  
  CGContextRestoreGState(context);
}

void SkinBitmap::stretchToRectAlpha(api_canvas *canvas, RECT *r, int alpha)
{
  if (!image)
    return;
  
  float floatAlpha = alpha / 255.f;
  
  CGContextRef context = canvas->getHDC();
  CGContextSaveGState(context);

  CGContextTranslateCTM (context, 0, r->bottom);
  CGContextScaleCTM(context, 1.0, -1.0);
  
  CGContextSetAlpha(context, floatAlpha);
  CGRect rect = CGRectMake(r->left, r->top, r->right-r->left, r->bottom-r->top);
  CGContextDrawImage(context, rect, image);
  CGContextRestoreGState(context);
}

void SkinBitmap::stretchToRectAlpha(api_canvas *canvas, RECT *src, RECT *dst, int alpha)
{
  if (!image)
    return;
    
  float floatAlpha = alpha / 255.f;
  
  // make a new image ref clipped to the source rect
  CGRect srcRect = CGRectMake(src->left, src->top, src->right-src->left, src->bottom-src->top);
  CGImageRef clippedImage = CGImageCreateWithImageInRect(image, srcRect);
  
  // blit onto canvas
  CGContextRef context = canvas->getHDC();
  CGContextSaveGState(context);
  
  CGContextTranslateCTM(context, 0, dst->bottom);
  CGContextScaleCTM(context, 1.0, -1.0);
  
  CGContextSetAlpha(context, floatAlpha);
  CGRect rect = CGRectMake(dst->left, dst->top, dst->right-dst->left, dst->bottom-dst->top);
  CGContextDrawImage(context, rect, clippedImage);
  CGContextRestoreGState(context);
  
  // release the reference to our clipped image
  CGImageRelease(clippedImage);
}

uint8_t *SkinBitmap::getBits()
{
  return static_cast<uint8_t *>(CGBitmapContextGetData(imageContext));
}

void SkinBitmap::UpdateBits(uint8_t *bits)
{
  CGImageRelease(image);
  image = CGBitmapContextCreateImage(imageContext);
}

ARGB32 SkinBitmap::getPixel(int x, int y)
{
  ARGB32 *array = (ARGB32 *)getBits();
  return array[x + y*getFullWidth()];
}

#define CBCLASS SkinBitmap
START_DISPATCH;
CB(IFC_BITMAP_GETBITMAP, GetBitmap);
CB(IFC_BITMAP_GETBITS, getBits);
VCB(IFC_BITMAP_UPDATEBITS, UpdateBits);
END_DISPATCH;
#undef CBCLASS