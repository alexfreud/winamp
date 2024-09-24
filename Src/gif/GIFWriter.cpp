#include "GIFWriter.h"
#include "api__gif.h"
#include <wchar.h>
#include <bfc/platform/strcmp.h>

extern "C" {
#include "gif_lib.h"
};

// valid items include "quality" for jpeg files with value "0" to "100"
// return value is 1 if the config item is supported, 0 if it is not.
int GIFWriter::setConfig(const wchar_t * item, const wchar_t * value) {
	return 0; // no config yet
}

// valid items include "quality" for jpeg files with value "0" to "100", "lossless" returns "1" if it is "0" otherwise
// return value is 1 if the config item is supported, 0 if it is not.
int GIFWriter::getConfig(const wchar_t * item, wchar_t * value, int valuelen) {
	if(!_wcsicmp(item,L"lossless")) lstrcpynW(value,L"0",valuelen);
	else return 0;
	return 1;
}

// returns 1 if the bit depth is supported (eg 32 for ARGB32, 24 for RGB24)
// ARGB32 MUST be supported
int GIFWriter::bitDepthSupported(int depth) {
	if(depth == 32 || depth == 24) return 1;
	return 0;
}

typedef struct {
	BYTE * data;
	unsigned int len;
	unsigned int alloc;
} writeStruct;

extern "C" int MyOutputFunc(GifFileType * gif, const GifByteType * data, int len) {
	writeStruct * p = (writeStruct *)gif->UserData;
	while (len + p->len > p->alloc) { // allocate more memory
		int d = ((p->alloc / 4) & 0xffffff00) + 0x100;
		if(d < 4096) d = 4096;
		p->alloc+=d;
		p->data = (BYTE*)WASABI_API_MEMMGR->sysRealloc(p->data,p->alloc);
	}
	memcpy(p->data+p->len,data,len);
	p->len += len;

	return len;
}

// returns the image in our format, free the returned buffer with api_memmgr::sysFree()
void * GIFWriter::convert(const void *pixels, int bitDepth, int w, int h, int *length) {
	// split into planes
	BYTE *redplane, *blueplane, *greenplane;
	redplane = (BYTE *)malloc(w*h*sizeof(BYTE));
	greenplane = (BYTE *)malloc(w*h*sizeof(BYTE));
	blueplane = (BYTE *)malloc(w*h*sizeof(BYTE));
	
	BYTE * px = (BYTE *)pixels;
	BYTE * end = px + (w * h * (bitDepth/8));
	int i=0;
	if(bitDepth == 24) {
		while(px < end) {
			blueplane[i] = *(px++);
			greenplane[i] = *(px++);
			redplane[i] = *(px++);
			i++;
		}
	} else if(bitDepth == 32) {
		while(px < end) {
			blueplane[i] = *(px++);
			greenplane[i] = *(px++);
			redplane[i] = *(px++);
			px++;
			i++;
		}
	}

	// make a color map
	int colormapsize = 256;
	ColorMapObject * OutputColorMap = GifMakeMapObject(colormapsize, NULL);
	BYTE* OutputBuffer = (GifByteType *) malloc(w * h * sizeof(GifByteType));

	// this is actually kinda crappy
	GifQuantizeBuffer(w,h,&colormapsize,redplane,greenplane,blueplane,OutputBuffer,OutputColorMap->Colors);

	unsigned int alloc = ((w*h / 3) & 0xffffff00) + 0x100; // guess at output file size
	if(alloc < 4096) alloc = 4096;
	writeStruct write = {(BYTE*)WASABI_API_MEMMGR->sysMalloc(alloc),0,alloc};
	bool succeeded=false;
	// write the file out
	int* l_error = NULL;
	GifFileType *gif = EGifOpen(&write,MyOutputFunc, l_error);
	if(gif) {
		if(EGifPutScreenDesc(gif,w, h, 8, 0, OutputColorMap)) {
			if(EGifPutImageDesc(gif, 0, 0, w, h, FALSE, NULL)) {	
				BYTE * p = OutputBuffer;
				succeeded=true;
				for (int i = 0; i < h; i++) {
					if(!EGifPutLine(gif, p, w)) {succeeded=false; break;}
					p+=w;
				}
			}
		}
		EGifCloseFile(gif, l_error);
	}
	// done!
	free(OutputBuffer);
	free(redplane);
	free(greenplane);
	free(blueplane);
	if(succeeded) {
		*length = write.len;
		return write.data;
	} else return 0;
}

#define CBCLASS GIFWriter
START_DISPATCH;
  CB(GETIMAGETYPENAME, getImageTypeName);
  CB(GETEXTENSIONS, getExtensions);
  CB(SETCONFIG, setConfig);
	CB(GETCONFIG, getConfig);
	CB(BITDEPTHSUPPORTED, bitDepthSupported);
	CB(CONVERT, convert);
END_DISPATCH;
#undef CBCLASS