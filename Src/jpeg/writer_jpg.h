#pragma once

#include <api/service/svcs/svc_imgwrite.h>

// {7BC27468-0475-4c0d-AEED-0C51195DC2EA}
static const GUID JPEGwriteguid = 
{ 0x7bc27468, 0x475, 0x4c0d, { 0xae, 0xed, 0xc, 0x51, 0x19, 0x5d, 0xc2, 0xea } };

class JpgWrite : public svc_imageWriter 
{
public:
	JpgWrite();
  static const char *getServiceName() { return "JPEG writer"; }
	static GUID getServiceGuid() { return JPEGwriteguid; } 
	const wchar_t * getImageTypeName() { return L"JPEG"; }
	const wchar_t * getExtensions() { return L"jpg;jpeg"; }
	int setConfig(const wchar_t * item, const wchar_t * value);
	int getConfig(const wchar_t * item, wchar_t * value, int valuelen);
	int bitDepthSupported(int depth);
  void * convert(const void *pixels, int bitDepth, int w, int h, int *length);
protected:
	RECVS_DISPATCH;
	
	int quality;
};
