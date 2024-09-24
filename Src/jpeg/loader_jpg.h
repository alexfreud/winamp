#pragma once

#include <api/service/svcs/svc_imgload.h>

// {AE04FB30-53F5-4032-BD29-032B87EC3404}
static const GUID JPEGguid = 
{ 0xae04fb30, 0x53f5, 0x4032, { 0xbd, 0x29, 0x3, 0x2b, 0x87, 0xec, 0x34, 0x04 } };


class JpgLoad : public svc_imageLoader 
{
public:
	static const char *getServiceName() { return "JPEG loader"; }

	static GUID getServiceGuid() { return JPEGguid; } 
	int isMine(const wchar_t *filename);
	const wchar_t *mimeType();
	int getHeaderSize();
	int testData(const void *data, int datalen);
	ARGB32 *loadImage(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params=NULL);
protected:
	RECVS_DISPATCH;
};
