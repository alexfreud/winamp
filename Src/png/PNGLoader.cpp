#include "PNGLoader.h"
#include "api__png.h"
#include <png.h>
#include <wchar.h>
#include <malloc.h>
#include <bfc/platform/strcmp.h>
#include <intsafe.h>
void premultiplyARGB32(ARGB32 *words, int nwords)
{
  for (; nwords > 0; nwords--, words++) {
    unsigned char *pixel = (unsigned char *)words;
    unsigned int alpha = pixel[3];
    if (alpha == 255) continue;
    pixel[0] = (pixel[0] * alpha) >> 8;	// blue
    pixel[1] = (pixel[1] * alpha) >> 8;	// green
    pixel[2] = (pixel[2] * alpha) >> 8;	// red
  }
}

static bool StringEnds(const wchar_t *a, const wchar_t *b)
{
	size_t aLen = wcslen(a);
	size_t bLen	= wcslen(b);
	if (aLen < bLen) return false;  // too short
	if (!_wcsicmp(a + aLen- bLen, b))
		return true;
	return false;
}

int PNGLoader::isMine(const wchar_t *filename) 
{
	if (filename && StringEnds(filename, L".PNG"))
		return 1;
	else
		return 0;
}

const wchar_t *PNGLoader::mimeType() 
{
	return L"image/png";
}

int PNGLoader::getHeaderSize() 
{
	return 8;
}

int PNGLoader::testData(const void *data, int datalen) {
	unsigned char *ptr = (unsigned char *)data;
	return !png_sig_cmp(ptr, 0, datalen);
}

typedef struct {
	const unsigned char *data;
	size_t pos;
	size_t datalen;
} my_read_info;

static void my_png_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	my_read_info *mri = (my_read_info *)png_get_io_ptr(png_ptr);
	if (mri->datalen - mri->pos < length)
		length = mri->datalen - mri->pos;
	memmove(data, mri->data + mri->pos, length);
	mri->pos += length;
}

ARGB32 *PNGLoader::loadImage(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params) {
	int w0=0, h0=0;

	ARGB32 *pixels = read_png(data, datalen, &w0, &h0, FALSE);

	if (pixels == NULL) return NULL;

	premultiplyARGB32(pixels, w0 * h0);

	if(w) *w = w0;
	if(h) *h = h0;

	return pixels;
}

ARGB32 *PNGLoader::loadImageData(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params) 
{
	return read_png(data, datalen, w, h, FALSE);
}

int PNGLoader::getDimensions(const void *data, int datalen, int *w, int *h) {
	return (read_png(data, datalen, w, h, TRUE) == reinterpret_cast<ARGB32*>(-1));
}

// From libpng example.c
ARGB32 *PNGLoader::read_png(const void *data, int datalen, int *w, int *h, int dimensions_only) {
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	my_read_info mri;
	mri.data = static_cast<const unsigned char *>(data);
	mri.pos = 0;
	mri.datalen = datalen;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also supply the
	* the compiler header file version, so that we know if the application
	* was compiled with a compatible version of the library.  REQUIRED
	*/
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		return NULL;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return NULL;
	}

	/* Set error handling if you are using the setjmp/longjmp method (this is
	* the normal method of doing things with libpng).  REQUIRED unless you
	* set up your own error handlers in the png_create_read_struct() earlier.
	*/
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		/* If we get here, we had a problem reading the file */
		return NULL;
	}

	png_set_read_fn(png_ptr, &mri, my_png_read_data);

	/* The call to png_read_info() gives us all of the information from the
	* PNG file before the first IDAT (image data chunk).  REQUIRED
	*/
	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		&interlace_type, NULL, NULL);

	if (w) *w = (int)width;
	if (h) *h = (int)height;

	ARGB32 *retval = 0;

	if (!dimensions_only) {

		/* tell libpng to strip 16 bit/color files down to 8 bits/color */
		if (bit_depth == 16) png_set_strip_16(png_ptr);
		if (bit_depth < 8) png_set_packing(png_ptr);

		/* flip the RGB pixels to BGR (or RGBA to BGRA) */
		png_set_bgr(png_ptr);

		/* Expand paletted colors into true RGB triplets */
		if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(png_ptr);

		/* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
		if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
			png_set_expand(png_ptr);

		png_set_gray_to_rgb(png_ptr);

		/* Expand paletted or RGB images with transparency to full alpha channels
		* so the data will be available as RGBA quartets.
		*/
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
			png_set_expand(png_ptr);
		}

		/* Add filler (or alpha) byte (before/after each RGB triplet) */
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

		/* Optional call to gamma correct and add the background to the palette
		* and update info structure.  REQUIRED if you are expecting libpng to
		* update the palette for you (ie you selected such a transform above).
		*/
		png_read_update_info(png_ptr, info_ptr);

		/* Allocate the memory to hold the image using the fields of info_ptr. */

		/* The easiest way to read the image: */
		//row_pointers = (png_bytep*)malloc(sizeof(png_bytep*)*height);
		size_t row_ptr_size = 0;
		if (SizeTMult(sizeof(png_bytep*), height, &row_ptr_size) == S_OK)
		{
			png_bytep *row_pointers = (png_bytep*)alloca(row_ptr_size);
			size_t image_size=0;
			if (SizeTMult(width, height, &image_size) == S_OK && SizeTMult(image_size, 4, &image_size)== S_OK)
			{
				ARGB32 *bytes = (ARGB32 *)WASABI_API_MEMMGR->sysMalloc(image_size);

				for (unsigned int row = 0; row < height; row++) {
					row_pointers[row] = ((unsigned char *)bytes) + width * 4 * (row);
				}

				/* Now it's time to read the image.  One of these methods is REQUIRED */
				png_read_image(png_ptr, row_pointers);

				/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
				png_read_end(png_ptr, info_ptr);

				retval = bytes;
			}
		}

		//free(row_pointers);
	}

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	/* that's it */
	return retval;
}

#define CBCLASS PNGLoader
START_DISPATCH;
  CB(ISMINE, isMine);
  CB(MIMETYPE, mimeType);
  CB(TESTDATA, testData);
  CB(GETHEADERSIZE, getHeaderSize);
  CB(GETDIMENSIONS, getDimensions);
  CB(LOADIMAGE, loadImage);
  CB(LOADIMAGEDATA, loadImageData);
END_DISPATCH;
#undef CBCLASS