#ifndef _SVC_IMGWRITE_H
#define _SVC_IMGWRITE_H

#include <api/service/services.h>
#include <bfc/platform/platform.h>
#include <bfc/dispatch.h>

class NOVTABLE svc_imageWriter : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::IMAGEWRITER; }

	// returns a human readable string about the format. eg "JPEG"
  const wchar_t * getImageTypeName();

	// returns a semi-colon delimited list of file extensions for this format. eg "jpg;jpeg"
	// MUST BE LOWER CASE
  const wchar_t * getExtensions();

	// valid items include "quality" for jpeg files with value "0" to "100"
	// return value is 1 if the config item is supported, 0 if it is not.
	int setConfig(const wchar_t * item, const wchar_t * value);
	
	// valid items include "quality" for jpeg files with value "0" to "100", "lossless" returns "1" if it is "0" otherwise
	// return value is 1 if the config item is supported, 0 if it is not.
	int getConfig(const wchar_t * item, wchar_t * value, int valuelen);

	// returns 1 if the bit depth is supported (eg 32 for ARGB32, 24 for RGB24)
	// ARGB32 MUST be supported
	int bitDepthSupported(int depth);

	// returns the image in our format, free the returned buffer with api_memmgr::sysFree()
  void * convert(const void *pixels, int bitDepth, int w, int h, int *length);
	
	enum {
		GETIMAGETYPENAME=10,
    GETEXTENSIONS=20,
		SETCONFIG=30,
    GETCONFIG=40,
		BITDEPTHSUPPORTED=50,
    CONVERT=60,
  };
};

inline const wchar_t *svc_imageWriter::getImageTypeName() {
	return _call(GETIMAGETYPENAME, L"");
}

inline const wchar_t *svc_imageWriter::getExtensions() {
	return _call(GETEXTENSIONS, L"");
}

inline int svc_imageWriter::setConfig(const wchar_t * item, const wchar_t * value) {
	return _call(SETCONFIG, (int)0, item, value);
}

inline int svc_imageWriter::getConfig(const wchar_t * item, wchar_t * value, int valuelen) {
	return _call(GETCONFIG, (int)0, item, value, valuelen);
}

inline int svc_imageWriter::bitDepthSupported(int depth) {
	return _call(BITDEPTHSUPPORTED, (int)0, depth);
}

inline void * svc_imageWriter::convert(const void *pixels, int bitDepth, int w, int h, int *length) {
	return _call(CONVERT, (void*)0, pixels, bitDepth, w, h, length);
}

// derive from this one
class NOVTABLE svc_imageWriterI : public svc_imageWriter {
public:
	virtual const wchar_t * getExtensions()=0;
  virtual const wchar_t * getImageTypeName()=0;
	virtual int setConfig(const wchar_t * item, const wchar_t * value){return 0;}
	virtual int getConfig(const wchar_t * item, wchar_t * value, int valuelen){return 0;}
	virtual int bitDepthSupported(int depth)=0;
  virtual void * convert(const void *pixels, int bitDepth, int w, int h, int *length)=0;
protected:
  RECVS_DISPATCH;
};

/* do we still use svc_enum?
#include <bfc/svc_enum.h>

class ImgWriterEnum : public SvcEnumT<svc_imageWriter> {
public:
  ImgWriterEnum(const char *_ext=NULL) : ext(_ext) { }

protected:
  virtual int testService(svc_imageWriter *svc) {
    if (ext.isempty()) return 1;
    else {
      PathParser pp(svc->getExtensions(), ";");
      for (int i = 0; i < pp.getNumStrings(); i++)
        if (STRCASEEQL(ext, pp.enumString(i))) return 1;
      return 0;
    }
  }
  String ext;
};
*/
#endif
