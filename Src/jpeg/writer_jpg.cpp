#include "main.h"
#include "writer_jpg.h"
#include "api__jpeg.h"

#include <api/memmgr/api_memmgr.h>
#include <shlwapi.h>
#include <strsafe.h>

/*BIG BIG THING TO NOTE
I have modified jmorecfg.h line 319 to specify 4 bytes per pixel with RGB. it is normally three.
*/ 
extern "C" {
#undef FAR
#include "jpeglib.h"
};

JpgWrite::JpgWrite() : quality(80) {}

// valid items include "quality" for jpeg files with value "0" to "100", "lossless" returns "1" if it is "0" otherwise
// return value is 1 if the config item is supported, 0 if it is not.

int JpgWrite::setConfig(const wchar_t * item, const wchar_t * value) {
	if(!_wcsicmp(item,L"quality")) quality = _wtoi(value);
	else return 0;
	return 1;
}

int JpgWrite::getConfig(const wchar_t * item, wchar_t * value, int valuelen) {
	if(!_wcsicmp(item,L"lossless")) lstrcpynW(value,L"0",valuelen);
	else if(!_wcsicmp(item,L"quality")) StringCchPrintfW(value,valuelen,L"%d",quality);
	else return 0;
	return 1;
}

int JpgWrite::bitDepthSupported(int depth) {
	if(depth == 32 || depth == 24) return 1;
	return 0;
}

extern "C" {
	typedef struct {
		jpeg_destination_mgr pub;
		BYTE *buf;
		int len;
	} my_destination_mgr;

	void init_destination(j_compress_ptr cinfo){}
	boolean empty_output_buffer(j_compress_ptr cinfo) {
		my_destination_mgr* m = (my_destination_mgr*)cinfo->dest;
		int used = m->len;
		int d = ((m->len / 4) & 0xffffff00) + 0x100;
		if(d < 4096) d = 4096;
		m->len+=d;
		m->buf = (BYTE*)WASABI_API_MEMMGR->sysRealloc(m->buf,m->len);
		m->pub.next_output_byte = m->buf + used;
		m->pub.free_in_buffer = d;
		
		return TRUE;
	}
	void term_destination(j_compress_ptr cinfo){}
};

// returns the image in our format, free the returned buffer with api_memmgr::sysFree()
void * JpgWrite::convert(const void *pixels, int bitDepth, int w, int h, int *length) {
	if(bitDepth != 32 && bitDepth != 24) return 0;
	jpeg_compress_struct cinfo={0};
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	
	// a guess at the final size...
	int allocsize = (int(double(w*h)*0.2) & 0xffffff00) + 0x100;
	if (allocsize < 4096) allocsize = 4096;
	
	// set up our output stream
	my_destination_mgr dest = {{0,(size_t)allocsize,init_destination,empty_output_buffer,term_destination},0,allocsize};
	dest.buf = (BYTE*)WASABI_API_MEMMGR->sysMalloc(allocsize);
	dest.pub.next_output_byte = dest.buf;
	cinfo.dest = (jpeg_destination_mgr*)&dest;
	
	// image parameters
	cinfo.image_width = w;
	cinfo.image_height = h;
	cinfo.input_components = 4;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
	jpeg_start_compress(&cinfo, TRUE);

	int row_stride = w * 4;

	BYTE * scanline = (BYTE *)malloc(row_stride);

	if(bitDepth == 32) {
		const ARGB32 * p = ((const ARGB32*)pixels);// + (h * w);
		while (cinfo.next_scanline < cinfo.image_height) {
			//p-=w;
			memcpy(scanline,p,row_stride);
			for(unsigned int i=0; i < cinfo.image_width; i++)
				((ARGB32*)scanline)[i] = ((p[i] & 0x0000ff) << 16) | (p[i] & 0x00ff00) | ((p[i] & 0xff0000) >> 16);
			jpeg_write_scanlines(&cinfo, &scanline, 1);
			p+=w;
		}
	} else if(bitDepth == 24) { // FUCKO: untested
		const BYTE * p = ((const BYTE*)pixels);// + (h * w * 3);
		while (cinfo.next_scanline < cinfo.image_height) {
			//p-=w*3;
			memcpy(scanline,p,row_stride);
			int l = cinfo.image_width * 4;
			for(int i=0,j=0; j < l; i+=3,j+=4) {
				scanline[j] = p[i+2];
				scanline[j+1] = p[i+1];
				scanline[j+2] = p[i];
			}
			jpeg_write_scanlines(&cinfo, &scanline, 1);
			p+=w*3;
		}
	}

	free(scanline);

	jpeg_finish_compress(&cinfo);

	if(length) *length = int(dest.len - dest.pub.free_in_buffer);
	jpeg_destroy_compress(&cinfo);
	return dest.buf;
}

#define CBCLASS JpgWrite
START_DISPATCH;
  CB(GETIMAGETYPENAME, getImageTypeName);
  CB(GETEXTENSIONS, getExtensions);
  CB(SETCONFIG, setConfig);
	CB(GETCONFIG, getConfig);
	CB(BITDEPTHSUPPORTED, bitDepthSupported);
	CB(CONVERT, convert);
END_DISPATCH;
#undef CBCLASS