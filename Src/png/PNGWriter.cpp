#include "PNGWriter.h"
#include "api__png.h"
#include <png.h>
#include <wchar.h>
#include <bfc/platform/strcmp.h>


// valid items include "quality" for jpeg files with value "0" to "100"
// return value is 1 if the config item is supported, 0 if it is not.
int PNGWriter::setConfig(const wchar_t * item, const wchar_t * value) {
	return 0; // no config yet
}

// valid items include "quality" for jpeg files with value "0" to "100", "lossless" returns "1" if it is "0" otherwise
// return value is 1 if the config item is supported, 0 if it is not.
int PNGWriter::getConfig(const wchar_t * item, wchar_t * value, int valuelen) {
	if(!_wcsicmp(item,L"lossless")) lstrcpynW(value,L"1",valuelen);
	else return 0;
	return 1;
}

typedef struct {
	BYTE * data;
	unsigned int len;
	unsigned int alloc;
} pngWrite;

extern "C" static void PNGAPI png_write(png_structp png_ptr, png_bytep data, png_size_t len) {
	pngWrite * p = (pngWrite *)png_get_io_ptr(png_ptr);
	while (len + p->len > p->alloc) { // allocate more memory
		int d = ((p->alloc / 4) & 0xffffff00) + 0x100;
		if(d < 4096) d = 4096;
		p->alloc+=d;
		p->data = (BYTE*)WASABI_API_MEMMGR->sysRealloc(p->data,p->alloc);
	}
	memcpy(p->data+p->len,data,len);
	p->len += int(len);
}

extern "C" static void PNGAPI png_flush(png_structp png_ptr) {}

// returns 1 if the bit depth is supported (eg 32 for ARGB32, 24 for RGB24)
// ARGB32 MUST be supported
int PNGWriter::bitDepthSupported(int depth) {
	if(depth == 32 || depth == 24) return 1;
	return 0;
}

// returns the image in our format, free the returned buffer with api_memmgr::sysFree()
void * PNGWriter::convert(const void *pixels0, int bitDepth, int w, int h, int *length) {
	if(bitDepth != 32 && bitDepth != 24) return 0;
	BYTE * pixels = (BYTE*)pixels0;
	png_structp png_ptr;
	png_infop info_ptr;


	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also check that
	 * the library version is compatible with the one used at compile time,
	 * in case we are using dynamically linked libraries.  REQUIRED.
	 */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		(png_voidp*)NULL, NULL, NULL);

	if (png_ptr == NULL)
		return 0;

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_write_struct(&png_ptr,  NULL);
		return 0;
	}
	pngWrite writer={0};

	/* Set error handling.  REQUIRED if you aren't supplying your own
	 * error handling functions in the png_create_write_struct() call.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem reading the file */
		png_destroy_write_struct(&png_ptr, &info_ptr);
		if(writer.data) WASABI_API_MEMMGR->sysFree(writer.data);
		return 0;
	}

	writer.alloc = (int(double(w*h)*1.25) & 0xffffff00) + 0x100;
	if (writer.alloc < 4096) writer.alloc = 4096;
	writer.data = (BYTE*)WASABI_API_MEMMGR->sysMalloc(writer.alloc);
	
	/* If you are using replacement read functions, instead of calling
	 * png_init_io() here you would call */
	png_set_write_fn(png_ptr, (void *)&writer, png_write,
		png_flush);

	/* Set the image information here.  Width and height are up to 2^31,
	 * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
	 * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
	 * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
	 * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	 * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
	 * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
	 */
	int colortype = (bitDepth == 32) ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB;
	png_set_IHDR(png_ptr, info_ptr, w, h, 8, colortype,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	/* optional significant bit chunk */
	png_color_8 sig_bit;
	/* if we are dealing with a grayscale image then */
	sig_bit.gray = 8;
	/* otherwise, if we are dealing with a color image then */
	sig_bit.red = 8;
	sig_bit.green = 8;
	sig_bit.blue = 8;
	/* if the image has an alpha channel then */
	sig_bit.alpha = (bitDepth==32)?8:0;
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);

	/* set up the transformations you want.  Note that these are
	 * all optional.  Only call them if you want them.
	 */

	/* invert monochrome pixels */
	png_set_invert_mono(png_ptr);

	/* Shift the pixels up to a legal bit depth and fill in
	 * as appropriate to correctly scale the image.
	 */
	png_set_shift(png_ptr, &sig_bit);

	/* pack pixels into bytes */
	png_set_packing(png_ptr);

	/* flip BGR pixels to RGB */
	png_set_bgr(png_ptr);

	/* swap bytes of 16-bit files to most significant byte first */
	png_set_swap(png_ptr);

	/* swap bits of 1, 2, 4 bit packed pixel formats */
	png_set_packswap(png_ptr);

	/*if (interlacing)
		number_passes = png_set_interlace_handling(png_ptr);
	else*/
	png_uint_32 number_passes = 1;

	/* The easiest way to write the image (you may have a different memory
	 * layout, however, so choose what fits your needs best).  You need to
	 * use the first method if you aren't handling interlacing yourself.
	 */
	int bytes_per_pixel = bitDepth / 8;

  /* The number of passes is either 1 for non-interlaced images,
   * or 7 for interlaced images.
   */
	for (png_uint_32 pass = 0; pass < number_passes; pass++)
	{
		BYTE * row = pixels; // + (h*w*bytes_per_pixel);
		for (int y = 0; y < h; y++) {
			//row -= w*bytes_per_pixel;
			png_write_rows(png_ptr, &row, 1);
			row += w*bytes_per_pixel;
		}
	}

	/* It is REQUIRED to call this to finish writing the rest of the file */
	png_write_end(png_ptr, info_ptr);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* that's it */
	if(length) *length = writer.len;
	return writer.data;
}

#define CBCLASS PNGWriter
START_DISPATCH;
  CB(GETIMAGETYPENAME, getImageTypeName);
  CB(GETEXTENSIONS, getExtensions);
  CB(SETCONFIG, setConfig);
	CB(GETCONFIG, getConfig);
	CB(BITDEPTHSUPPORTED, bitDepthSupported);
	CB(CONVERT, convert);
END_DISPATCH;
#undef CBCLASS