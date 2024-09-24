#ifndef NULLSOFT_PNG_PNGLOADER_H
#define NULLSOFT_PNG_PNGLOADER_H

#include <api/service/svcs/svc_imgload.h>

class ifc_xmlreaderparams;

class BMPLoader : public svc_imageLoader
{
public:
  // service
  static const char *getServiceName() { return "BMP loader"; }

  virtual int isMine(const wchar_t *filename);
  virtual const wchar_t *mimeType();
  virtual int getHeaderSize();
  virtual int testData(const void *data, int datalen);
  virtual ARGB32 *loadImage(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params=NULL);

protected:
	RECVS_DISPATCH;
};
#endif
