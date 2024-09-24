#include "BMPLoader.h"
#include "api__bmp.h"
#include <wchar.h>
#include <bfc/platform/strcmp.h>

static bool StringEnds(const wchar_t *a, const wchar_t *b)
{
	size_t aLen = wcslen(a);
	size_t bLen	= wcslen(b);
	if (aLen < bLen) return false;  // too short
	return !_wcsicmp(a + aLen- bLen, b);
}

int BMPLoader::isMine(const wchar_t *filename) 
{
	return (filename && StringEnds(filename, L".BMP"));
}

const wchar_t *BMPLoader::mimeType() 
{
	return L"image/bmp";
}

int BMPLoader::getHeaderSize() 
{
	return 2;
}

int BMPLoader::testData(const void *data, int datalen)
{
	if(datalen < 2) return 0;
	return *((WORD*)data) == (WORD)'MB';
}

static void writeFile(const wchar_t *file, const void * data, int length) {
	FILE *f = _wfopen(file,L"wb");
	if(!f) return;
	fwrite(data,length,1,f);
	fclose(f);
}

ARGB32 *BMPLoader::loadImage(const void *data, int datalen, int *w, int *h, ifc_xmlreaderparams *params)
{
	int w0=0,h0=0;
	if(!w) w = &w0;
	if(!h) h = &h0;

	wchar_t file[MAX_PATH] = {0};
	GetTempPath(MAX_PATH, file);
	GetTempFileName(file,L"wa5bmp",0,file);
	writeFile(file,data,datalen);
	
	HBITMAP hbmp = (HBITMAP)LoadImage(0, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	
	_wunlink(file);
	if(!hbmp) return 0;
	BITMAP bm;
	HDC hMemDC, hMemDC2;
	HBITMAP hsrcdib;
	void *srcdib;
	BITMAPINFO srcbmi = {0, };
	if(GetObject(hbmp, sizeof(BITMAP), &bm) == 0) { DeleteObject(hbmp); return 0; }

	*w = bm.bmWidth;
	*h = abs(bm.bmHeight);

	ARGB32 *newbits=NULL;
	srcbmi.bmiHeader.biSize = sizeof(srcbmi.bmiHeader);
	srcbmi.bmiHeader.biWidth = *w;
	srcbmi.bmiHeader.biHeight = -*h;
	srcbmi.bmiHeader.biPlanes = 1;
	srcbmi.bmiHeader.biBitCount = 32;
	srcbmi.bmiHeader.biCompression = BI_RGB;

	hMemDC = CreateCompatibleDC(NULL);
	hMemDC2 = CreateCompatibleDC(NULL);
	hsrcdib = CreateDIBSection(hMemDC, &srcbmi, DIB_RGB_COLORS, &srcdib, NULL, 0);
	if(hsrcdib) {
		HBITMAP hprev = (HBITMAP) SelectObject(hMemDC, hsrcdib);
		HBITMAP hprev2 = (HBITMAP) SelectObject(hMemDC2, hbmp);
		BitBlt(hMemDC, 0, 0, *w, *h, hMemDC2, 0, 0, SRCCOPY);
		newbits = (ARGB32*)WASABI_API_MEMMGR->sysMalloc((*w) * (*h) * sizeof(ARGB32));
		memcpy(newbits, srcdib, (*w)*(*h)*sizeof(ARGB32));
		{
			// put the alpha channel to 255
			unsigned char *b = (unsigned char *)newbits;
			int l = (*w) * (*h);
			for (int i = 0;i < l;i++)
				b[(i*4) + 3] = 0xff;
		}
		SelectObject(hMemDC, hprev);
		SelectObject(hMemDC2, hprev2);
		DeleteObject(hsrcdib);
	}
	DeleteDC(hMemDC2);
	DeleteDC(hMemDC);

	DeleteObject(hbmp);

	return newbits;
}

#define CBCLASS BMPLoader
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