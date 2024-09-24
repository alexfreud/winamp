#include <precomp.h>
#include "api_imgldr.h"

#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS imgldr_apiI
START_DISPATCH;
  CB(IMGLDR_API_MAKEBMP,            imgldr_makeBmp);
#ifdef _WIN32
  CB(IMGLDR_API_MAKEBMP2,           imgldr_makeBmp2);
#endif
  VCB(IMGLDR_API_RELEASEBMP,        imgldr_releaseBmp);
#ifdef WASABI_COMPILE_SKIN
  CB(IMGLDR_API_REQUESTSKINBITMAP,  imgldr_requestSkinBitmap);
  CB(IMGLDR_API_REQUESTSKINREGION,  imgldr_requestSkinRegion);
  VCB(IMGLDR_API_CACHESKINREGION,   imgldr_cacheSkinRegion);
  VCB(IMGLDR_API_RELEASESKINBITMAP, imgldr_releaseSkinBitmap);
#endif //WASABI_COMPILE_SKIN
END_DISPATCH;
