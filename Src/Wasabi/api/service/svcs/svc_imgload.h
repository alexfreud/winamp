#ifndef _SVC_IMGLOAD_H
#define _SVC_IMGLOAD_H

#include <api/service/services.h>
#include <bfc/platform/platform.h>
#include <bfc/dispatch.h>


class ifc_xmlreaderparams;

class NOVTABLE svc_imageLoader : public Dispatchable 
{
public:
  static FOURCC getServiceType() { return WaSvc::IMAGELOADER; }

  // assuming there is an extension of this type, is it yours?
  int isMine(const wchar_t *filename);

  // returns the mime type for this type of image
  const wchar_t *mimeType();

  // returns how many bytes needed to get image info
  int getHeaderSize();

  // test image data, return TRUE if you can load it
  int testData(const void *data, int datalen);

  // just gets the width and height from the data, if possible
  int getDimensions(const void *data, int datalen, int *w, int *h);

  // converts the data into pixels + premultiply, use api->sysFree to deallocate
  ARGB32 *loadImage(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params=NULL);\
  
  // converts the data into pixels, use api->sysFree to deallocate
  ARGB32 *loadImageData(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params=NULL);

  enum {
    ISMINE=50,
	MIMETYPE=75,
    GETHEADERSIZE=100,
    TESTDATA=200,
    GETDIMENSIONS=300,
    LOADIMAGE=400,
	LOADIMAGEDATA=500,
  };
};

inline int svc_imageLoader::isMine(const wchar_t *filename) {
  return _call(ISMINE, 0, filename);
}

inline const wchar_t *svc_imageLoader::mimeType() {
  return _call(MIMETYPE, L"");
}

inline int svc_imageLoader::getHeaderSize() {
  return _call(GETHEADERSIZE, -1);
}

inline int svc_imageLoader::testData(const void *data, int datalen) {
  return _call(TESTDATA, 0, data, datalen);
}

inline int svc_imageLoader::getDimensions(const void *data, int datalen, int *w, int *h) {
  return _call(GETDIMENSIONS, 0, data, datalen, w, h);
}

inline ARGB32 *svc_imageLoader::loadImage(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params) {
  return _call(LOADIMAGE, (ARGB32*)0, data, datalen, w, h, params);
}

inline ARGB32 *svc_imageLoader::loadImageData(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params) {
  return _call(LOADIMAGEDATA, (ARGB32*)0, data, datalen, w, h, params);
}

// derive from this one
class NOVTABLE svc_imageLoaderI : public svc_imageLoader 
{
public:
  virtual int isMine(const wchar_t *filename)=0;
  virtual const wchar_t *mimeType(void)=0;
  // return the header size needed to get w/h and determine if it can be loaded
  virtual int getHeaderSize() { return -1; }//don't know
  // test image data, return TRUE if you can load it
  virtual int testData(const void *data, int datalen)=0;
  // just gets the width and height from the data, if possible
  virtual int getDimensions(const void *data, int datalen, int *w, int *h) { return 0; }
  // converts the data into pixels + premultiply, use api->sysFree to deallocate
  virtual ARGB32 *loadImage(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params=NULL)=0;
#if 0
  // converts the data into pixels, use api->sysFree to deallocate
  virtual ARGB32 *loadImageData(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params=NULL)=0;
#endif

protected:
  RECVS_DISPATCH;
};



#endif
