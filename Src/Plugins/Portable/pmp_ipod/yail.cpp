#include "iPodDevice.h"
#include <stdlib.h>
#include <memory.h>
#include <api/service/waservicefactory.h>
#include <api/service/svcs/svc_imgload.h>
#include "api.h"
#include <tataki/export.h>

#include "yail.h"

extern PMPDevicePlugin plugin;
static void recTransform (RGB565 *destination, RGB565 *source, int width, int height, int row_stride);

static __forceinline ARGB32 pixto32bit(RGB565 pix0, int format) {
	unsigned long pix = pix0;
	if(format == RGB_565)
		return (ARGB32)(((pix & 0x001F) << 3) | ((pix & 0x07E0) << 5) | ((pix & 0xF800) << 8) | 0xff000000);
	else // format == RGB_555. Ignore alpha channel.
		return (ARGB32)(((pix & 0x001F) << 3) | ((pix & 0x03E0) << 6) | ((pix & 0x7C00) << 9) | 0xff000000);
}

static __forceinline RGB565 pixto16bit(ARGB32 pix, int format) {
	//           10987654321098765432109876543210
	// ARGB32 is AAAAAAAARRRRRRRRGGGGGGGGBBBBBBBB
	// RGB565 is                 RRRRRGGGGGGBBBBB
	// RGB555 is                 ARRRRRGGGGGBBBBB
	if(format == RGB_565)
		return (RGB565)( ((pix >> 8) & 0xF800) | ((pix >> 5) & 0x07E0) | ((pix >> 3) & 0x001F) );
	else // format == RGB_555. set A to 1 for now.
		return (RGB565)( 0x8000 | ((pix >> 9) & 0x7C00) | ((pix >> 6) & 0x03E0) | ((pix >> 3) & 0x001F) );
}

Image::Image(const ARGB32 * d, int w, int h) : width(w), height(h) {
	int size = sizeof(ARGB32)*w*h;
	data = (ARGB32*)calloc(size,1);
	memcpy(data,d,size);
}

#define ALIGN(size, boundary) ((((boundary) - ((size) % (boundary))) % (boundary)) + (size))

Image::Image(const RGB565 * d, int w, int h, int format, int alignRowBytes, int alignImageBytes) : width(w), height(h) {
	data = (ARGB32*)calloc(w*h*sizeof(ARGB32),1);
	int rowgap = (ALIGN(sizeof(RGB565) * width, alignRowBytes) / sizeof(RGB565)) - width;
	int p=0, q=0;
	for(int j=0; j<height; j++)
	{
		for(int i=0; i<width; i++)
		{
			data[p++] = pixto32bit(d[q++], format);
		}
		q += rowgap;
	}
}

Image::~Image() {
	if(data) free(data); data=0;
}

void Image::exportToRGB565(RGB565* d_, int format, int alignRowBytes, int alignImageBytes) const {
	int p=0, q=0;
	int rowgap = (ALIGN(sizeof(RGB565) * width, alignRowBytes) / sizeof(RGB565)) - width;

	RGB565* d;
	if(format == RGB_555_REC)
	{
		int l = get16BitSize(width, height, alignRowBytes, alignImageBytes);
		d = (RGB565*)calloc(l, 1);
	}
	else
		d = d_;

	for(int j=0; j<height; j++)
	{
		for(int i=0; i<width; i++)
		{
			d[p++] = pixto16bit(data[q++], format);
		}
		p += rowgap;
	}

	if(format == RGB_555_REC)
	{ // do wierd transform
		recTransform(d_, d, width, height, width + rowgap);
		free(d);
	}
}

void Image::exportToARGB32(ARGB32* d) const {
	memcpy(d,data,sizeof(ARGB32)*width*height);
}

int Image::get16BitSize(int width, int height, int alignRowBytes, int alignImageBytes) {
	int rowSize = ALIGN(sizeof(RGB565) * width, alignRowBytes);
	return ALIGN(rowSize * height, alignImageBytes);
}


static void recTransform (RGB565 *destination, RGB565 *source, int width, int height, int row_stride)
{
	if (width == 1)
	{
		*destination = *source;
	}
	else
	{
		recTransform(destination, source, width/2, height/2, row_stride);

		recTransform(destination + (width/2)*(height/2), source + (height/2)*row_stride, width/2, height/2, row_stride);

		recTransform(destination + 2*(width/2)*(height/2), source + width/2, width/2, height/2, row_stride);

		recTransform(destination + 3*(width/2)*(height/2), source + (height/2)*row_stride + width/2, width/2, height/2, row_stride);
	}
}