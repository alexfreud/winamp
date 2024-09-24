#include "BMPWriter.h"
#include "api__bmp.h"
#include <wchar.h>
#include <bfc/platform/strcmp.h>

// valid items include "quality" for jpeg files with value "0" to "100"
// return value is 1 if the config item is supported, 0 if it is not.
int BMPWriter::setConfig(const wchar_t * item, const wchar_t * value) {
	return 0; // no config yet
}

// valid items include "quality" for jpeg files with value "0" to "100", "lossless" returns "1" if it is "0" otherwise
// return value is 1 if the config item is supported, 0 if it is not.
int BMPWriter::getConfig(const wchar_t * item, wchar_t * value, int valuelen) {
	if(!_wcsicmp(item,L"lossless")) lstrcpynW(value,L"1",valuelen);
	else return 0;
	return 1;
}

// returns 1 if the bit depth is supported (eg 32 for ARGB32, 24 for RGB24)
// ARGB32 MUST be supported
int BMPWriter::bitDepthSupported(int depth) {
	if(depth == 32 || depth == 24) return 1;
	return 0;
}

// returns the image in our format, free the returned buffer with api_memmgr::sysFree()
void * BMPWriter::convert(const void *pixels, int bitDepth, int w, int h, int *length) {
	if(bitDepth != 32 && bitDepth != 24) return 0;

	int pixDataSize = (w * h * (bitDepth/8));
	int headersSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	*length = pixDataSize + headersSize;
	
	BITMAPFILEHEADER fileHeader={0};
	fileHeader.bfType = 'MB';
	fileHeader.bfSize = *length;
	fileHeader.bfOffBits = headersSize;

	BITMAPINFOHEADER infoHeader={0};
	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = w;
	infoHeader.biHeight = h;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = bitDepth;
	infoHeader.biCompression = BI_RGB;
	infoHeader.biSizeImage = pixDataSize;
	infoHeader.biXPelsPerMeter = 2834; //72ppi
	infoHeader.biYPelsPerMeter = 2834; //72ppi
	infoHeader.biClrUsed = 0;
	infoHeader.biClrImportant = 0;
	
	/*
	The structure of bitmap files is like this:
	fileheader
	infoheader
	palette (optional)
	data
	*/
	BYTE * bmp = (BYTE *)WASABI_API_MEMMGR->sysMalloc(*length);
	if(!bmp) return 0;
	memcpy(bmp,&fileHeader,sizeof(BITMAPFILEHEADER));
	memcpy(bmp + sizeof(BITMAPFILEHEADER),&infoHeader,sizeof(BITMAPINFOHEADER));
	
	//memcpy(bmp + headersSize,pixels,pixDataSize);
	{
		BYTE *pOut = bmp + headersSize;
		BYTE *pIn = ((BYTE*)pixels) + w*h*(bitDepth/8);
		int d = w*(bitDepth/8);
		for(int i=0; i<h; i++) {
			pIn-=d;
			memcpy(pOut,pIn,d);
			pOut+=d;
		}
	}
	return bmp;
}

#define CBCLASS BMPWriter
START_DISPATCH;
  CB(GETIMAGETYPENAME, getImageTypeName);
  CB(GETEXTENSIONS, getExtensions);
  CB(SETCONFIG, setConfig);
	CB(GETCONFIG, getConfig);
	CB(BITDEPTHSUPPORTED, bitDepthSupported);
	CB(CONVERT, convert);
END_DISPATCH;
#undef CBCLASS