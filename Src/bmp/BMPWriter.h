#ifndef NULLSOFT_BMP_BMPWRITER_H
#define NULLSOFT_BMP_BMPWRITER_H

#include <api/service/svcs/svc_imgwrite.h>

class ifc_xmlreaderparams;

class BMPWriter : public svc_imageWriter
{
public:
  static const char *getServiceName() { return "BMP loader"; }
	const wchar_t * getImageTypeName() { return L"BMP"; }
	const wchar_t * getExtensions() { return L"bmp;dib"; }
	int setConfig(const wchar_t * item, const wchar_t * value);
	int getConfig(const wchar_t * item, wchar_t * value, int valuelen);
	int bitDepthSupported(int depth);
  void * convert(const void *pixels, int bitDepth, int w, int h, int *length);
protected:
	RECVS_DISPATCH;
};
#endif
