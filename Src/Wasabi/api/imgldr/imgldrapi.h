#ifndef __IMGLDRAPI_H
#define __IMGLDRAPI_H

#include <api/imgldr/api_imgldr.h>

class ImgLdrApi : public imgldr_apiI 
{
  public:
     ARGB32 *imgldr_makeBmp(const wchar_t *filename, int *has_alpha, int *w, int *h);
#ifdef _WIN32
     ARGB32 *imgldr_makeBmp2(OSMODULEHANDLE hInst, int id, int *has_alpha, int *w, int *h, const wchar_t *colorgroup = NULL);
#endif
     void imgldr_releaseBmp(ARGB32 *bmpbits);
#ifdef WASABI_COMPILE_SKIN
     ARGB32 *imgldr_requestSkinBitmap(const wchar_t *file, int *has_alpha, int *x, int *y, int *subw, int *subh, int *w, int *h, int cached);
     RegionServer *imgldr_requestSkinRegion(const wchar_t *id);
     void imgldr_cacheSkinRegion(const wchar_t *id, api_region *r);
     void imgldr_releaseSkinBitmap(ARGB32 *bmpbits);
#endif //WASABI_COMPILE_SKIN
};


#endif
