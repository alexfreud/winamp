#include <precomp.h>

#include "svc_imgload.h"

#define CBCLASS svc_imageLoaderI
START_DISPATCH;
  CB(ISMINE, isMine);
  CB(TESTDATA, testData);
  CB(GETHEADERSIZE, getHeaderSize);
  CB(GETDIMENSIONS, getDimensions);
  CB(LOADIMAGE, loadImage);
END_DISPATCH;
#undef CBCLASS

