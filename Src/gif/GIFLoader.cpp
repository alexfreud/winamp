#include "GIFLoader.h"
#include "api__gif.h"
#include <wchar.h>
#include <bfc/platform/strcmp.h>

extern "C" {
#include "../giflib/gif_lib.h"
};

static bool StringEnds(const wchar_t *a, const wchar_t *b)
{
	size_t aLen = wcslen(a);
	size_t bLen	= wcslen(b);
	if (aLen < bLen) return false;  // too short
	return !_wcsicmp(a + aLen - bLen, b);
}

int GIFLoader::isMine(const wchar_t *filename) 
{
	return (filename && StringEnds(filename, L".GIF"));
}

int GIFLoader::getHeaderSize() 
{
	return 3;
}

const wchar_t *GIFLoader::mimeType() 
{
	return L"image/gif";
}

int GIFLoader::testData(const void *data, int datalen)
{
	if(datalen >= 3 && strncmp((const char*)data,"GIF",3)==0) return 1;
	return 0;
}

typedef struct {
	BYTE * data;
	int len;
} readStruct;

extern "C" int myreader(GifFileType *gif, GifByteType *data, int len) {
	readStruct* r = (readStruct*)gif->UserData;
	int l = min(len,r->len);
	if(l == 0) return 0;
	memcpy(data,r->data,l);
	r->len -= l;
	r->data += l;
	return l;
}

ARGB32 *GIFLoader::loadImage(const void *datain, int datalen, int *w, int *h, ifc_xmlreaderparams *params)
{
	readStruct read = {(BYTE*)datain,datalen};
	int* l_error = NULL;
	GifFileType * gif = DGifOpen(&read, myreader, l_error);
	if(!gif) return 0;

	if(!DGifSlurp(gif) || !gif->ImageCount) { DGifCloseFile(gif, l_error); return 0; }

	ColorMapObject *map = gif->SColorMap;
	if(!map) map = gif->SavedImages[0].ImageDesc.ColorMap;
	if(!map) { DGifCloseFile(gif, l_error); return 0; }
	int iw = gif->SavedImages[0].ImageDesc.Width;
	int ih = gif->SavedImages[0].ImageDesc.Height;
	if(w) *w = iw;
	if(h) *h = ih;

	ARGB32 * data = (ARGB32 *)WASABI_API_MEMMGR->sysMalloc(iw * ih * sizeof(ARGB32));
	ARGB32 * p = data;
	ARGB32 * end = data + (iw * ih);
	
	GifPixelType *line = gif->SavedImages[0].RasterBits;


	if(gif->SavedImages[0].ImageDesc.Interlace)
	{
		// The way Interlaced image should be read - offsets and jumps... 
		const int InterlacedOffset[] = { 0, 4, 2, 1 };
		const int InterlacedJumps[] = { 8, 8, 4, 2 }; 

		for(int i = 0; i < 4; i++)
		{
			for(int j = InterlacedOffset[i]; j < ih; j += InterlacedJumps[i])
			{
				p = &data[j*iw];
				for(int k = 0; k<iw; k++)
				{
					int px = *(line++);
					if(px < map->ColorCount && (px != gif->SBackGroundColor || px == 0)) {
						GifColorType& color = map->Colors[px];
						*(p++) = 0xff000000 | color.Blue | (color.Green << 8) | (color.Red << 16);
					} else
						*(p++) = 0;
				}
			}
		}
	}
	else
	{
		while(p < end) {
			int px = *(line++);
			if(px < map->ColorCount && (px != gif->SBackGroundColor || px == 0)) {
				GifColorType& color = map->Colors[px];
				*(p++) = 0xff000000 | color.Blue | (color.Green << 8) | (color.Red << 16);
			} else
				*(p++) = 0;
		}
	}

	DGifCloseFile(gif, l_error);
	return data;
}

#define CBCLASS GIFLoader
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