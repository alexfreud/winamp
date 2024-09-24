#include "precomp.h"
#include <api.h>
#include "imgldrapi.h"
#include <api/imgldr/imgldr.h>

#include <api/skin/skinelem.h>
imgldr_api *imgLoaderApi = NULL;

ARGB32 *ImgLdrApi::imgldr_makeBmp(const wchar_t *filename, int *has_alpha, int *w, int *h)
{
  if (filename == NULL) 
	{
    DebugString("illegal param : filename == NULL");
    return NULL;
  }
  return imageLoader::makeBmp(filename, NULL, has_alpha, w, h, NULL, TRUE, NULL);
}

#ifdef _WIN32
ARGB32 *ImgLdrApi::imgldr_makeBmp2(HINSTANCE hInst, int id, int *has_alpha, int *w, int *h, const wchar_t *colorgroup) 
{
  return imageLoader::makeBmp(hInst, id, has_alpha,w,h, colorgroup);
}
#endif

void ImgLdrApi::imgldr_releaseBmp(ARGB32 *bmpbits)
{
  if (bmpbits == NULL) {
    DebugString("illegal param : bmpbits == NULL");
    return;
  }
  imageLoader::release(bmpbits);
}

#ifdef WASABI_COMPILE_SKIN

ARGB32 *ImgLdrApi::imgldr_requestSkinBitmap(const wchar_t *file, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached) 
{
  if (file == NULL) 
	{
    DebugString("illegal param : file == NULL");
    return NULL;
  }
  return imageLoader::requestSkinBitmap(file, has_alpha, x, y, subw, subh, w, h, cached);
}

void ImgLdrApi::imgldr_releaseSkinBitmap(ARGB32 *bmpbits) 
{
  if (bmpbits == NULL) 
	{
    DebugString("illegal param : bmpbits == NULL");
    return;
  }
  imageLoader::releaseSkinBitmap(bmpbits);
}

RegionServer *ImgLdrApi::imgldr_requestSkinRegion(const wchar_t *id) 
{
  if (id == NULL) 
	{
    DebugString("illegal param : id == NULL");
    return NULL;
  }
  return WASABI_API_PALETTE->requestSkinRegion(id);
}

void ImgLdrApi::imgldr_cacheSkinRegion(const wchar_t *id, api_region *r) 
{
  if (id == NULL) 
	{
    DebugString("illegal param : id == NULL");
  }
  if (r == NULL) 
	{
    DebugString("illegal param : region == NULL");
  }
  WASABI_API_PALETTE->cacheSkinRegion(id, r);
}

#endif

