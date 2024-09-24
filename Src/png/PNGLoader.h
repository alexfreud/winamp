#ifndef NULLSOFT_PNG_PNGLOADER_H
#define NULLSOFT_PNG_PNGLOADER_H

#include <api/service/svcs/svc_imgload.h>

class ifc_xmlreaderparams;

class PNGLoader : public svc_imageLoader
{
public:
	// service
	static const char *getServiceName() { return "PNG loader"; }

	virtual int isMine(const wchar_t *filename);
	virtual const wchar_t *mimeType();
	virtual int getHeaderSize();
	virtual int testData(const void *data, int datalen);
	virtual int getDimensions(const void *data, int datalen, int *w, int *h);
	virtual ARGB32 *loadImage(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params=NULL);
	virtual ARGB32 *loadImageData(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params=NULL);
private:
	ARGB32 *read_png(const void *data, int datalen, int *w, int *h, int dimensions_only);

protected:
	RECVS_DISPATCH;
};
#endif
