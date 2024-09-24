#include "bltcanvas.h"
#include <tataki/bitmap/bitmap.h>

BltCanvas::~BltCanvas()
{
	if (hdc == NULL) return ;

	// kill the bitmap and its DC
	SelectObject(hdc, prevbmp);
	if (ourbmp)
	{
		//GdiFlush();
		DeleteObject(hbmp);
	}
	DeleteDC(hdc);
	hdc = NULL;

	if (skinbmps)
	{
		for (int i=0;i<skinbmps->getNumItems();i++)
			skinbmps->enumItem(i)->Release();

		delete skinbmps;
	}
	if (envelope)
		envelope->Release();
}

BltCanvas::BltCanvas(HBITMAP bmp)
{
	prevbmp = NULL;
	bits = NULL;
	fcoord = TRUE;
	ourbmp = FALSE;
	skinbmps = NULL;
	envelope = NULL;

	hbmp = bmp;
	ASSERT(hbmp != NULL);

	// create tha DC
	hdc = CreateCompatibleDC(NULL);
	prevbmp = (HBITMAP)SelectObject(hdc, hbmp);
}

BltCanvas::BltCanvas()
{
	hbmp = NULL;
	prevbmp = NULL;
	bits = NULL;
	fcoord = TRUE;
	ourbmp = FALSE;
	bpp = 32; // TODO: benski> pass as parameter?
	skinbmps = NULL;
	envelope = NULL;
	hdc = CreateCompatibleDC(NULL);	
}

BltCanvas::BltCanvas(int w, int h, HWND wnd, int nb_bpp/*, unsigned char *pal, int palsize*/)
{
	hbmp = NULL;
	prevbmp = NULL;
	bits = NULL;
	fcoord = TRUE;
	ourbmp = FALSE;
	bpp = nb_bpp;
	skinbmps = NULL;
	envelope = NULL;
	hdc = CreateCompatibleDC(NULL);
	AllocBitmap(w,h,nb_bpp);

	if (hbmp)
	{
		// create tha DC
		
		if (!hdc) {
//			int x = GetLastError();
		}
		prevbmp = (HBITMAP)SelectObject(hdc, hbmp);
	}
}

void BltCanvas::AllocBitmap(int w, int h, int nb_bpp)
{
	ASSERT(!hbmp);
	ASSERT(w != 0 && h != 0);
	if (w == 0) w = 1;
	if (h == 0) h = 1;

	BITMAPINFO bmi;
	MEMZERO(&bmi, sizeof(BITMAPINFO));
	//bmi.bmiHeader.biClrUsed = 0; // we memzero above, no need
	//bmi.bmiHeader.biClrImportant = 0; // we memzero above, no need
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = ABS(w);
	bmi.bmiHeader.biHeight = -ABS(h);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = nb_bpp;
	bmi.bmiHeader.biCompression = BI_RGB;
	//bmi.bmiHeader.biSizeImage = 0; // we memzero above, no need
	//bmi.bmiHeader.biXPelsPerMeter = 0; // we memzero above, no need
	//bmi.bmiHeader.biYPelsPerMeter = 0; // we memzero above, no need
	//GdiFlush();
	hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	
	if (hbmp == NULL)
	{
		return ;
	}
	
	ourbmp=TRUE;
	GetObject(hbmp, sizeof(BITMAP), &bm);
	width = bm.bmWidth;
	height = ABS(bm.bmHeight);
	pitch = bm.bmWidthBytes;
}

void *BltCanvas::getBits()
{
	return bits;
}

HBITMAP BltCanvas::getBitmap()
{
	return hbmp;
}

SkinBitmap *BltCanvas::getSkinBitmap()
{
	// make a SkinBitmap envelope
	if (!envelope)
		envelope = new SkinBitmap(getBitmap(), getHDC(), 1, getBits());

	// do not delete envelope, it's deleted in destructor
	return envelope;
}

SkinBitmap *BltCanvas::makeSkinBitmap()
{
	// make a clone of the bitmap - JF> what was that crap about envelopes?
	SkinBitmap *clone = new SkinBitmap(getBitmap(), getHDC(), 1);

	if (!skinbmps)
		skinbmps = new PtrList<SkinBitmap>;
	skinbmps->addItem(clone);

	return clone;
}

void BltCanvas::disposeSkinBitmap(SkinBitmap *b)
{
	if (skinbmps->haveItem(b))
	{
		skinbmps->removeItem(b);
		b->Release();
	}
	else
	{
		DebugString("disposeSkinBitmap called on unknown pointer, you should call it from the object used to makeSkinBitmap()\n");
	}
}

void BltCanvas::fillBits(COLORREF color)
{
	if (bpp == 32)
	{	// clear out the bits
		DWORD *dwbits = (DWORD *)bits;
		MEMFILL<DWORD>(dwbits, color, bm.bmWidth * bm.bmHeight);
	}
}

void BltCanvas::vflip(int vert_cells)
{
	ASSERT(bits != NULL);
	//  BITMAP bm;
	//  int r = GetObject(hbmp, sizeof(BITMAP), &bm);
	//  if (r == 0) return;
	int w = bm.bmWidth, h = bm.bmHeight;
	int bytes = 4 * w;
	__int8 *tmpbuf = (__int8 *)MALLOC(bytes);
	if (tmpbuf)
	{
		int cell_h = h / vert_cells;
		for (int j = 0; j < vert_cells; j++)
			for (int i = 0; i < cell_h / 2; i++)
			{
				char *p1, *p2;
				p1 = (__int8 *)bits + bytes * i + (j * cell_h * bytes);
				p2 = (__int8 *)bits + bytes * ((cell_h - 1) - i) + (j * cell_h * bytes);
				if (p1 == p2) continue;
				MEMCPY(tmpbuf, p1, bytes);
				MEMCPY(p1, p2, bytes);
				MEMCPY(p2, tmpbuf, bytes);
			}
		FREE(tmpbuf);
	}
}

void BltCanvas::hflip(int hor_cells)
{
	ASSERT(bits != NULL);
	// todo: optimize
	int w = bm.bmWidth, h = bm.bmHeight;
	for (int i = 0;i < hor_cells;i++)
		for (int x = 0;x < w / 2 / hor_cells;x++)
			for (int y = 0;y < h;y++)
			{
				int *p = ((int *)bits) + x + y * w + (i * w / hor_cells);
				int *d = ((int *)bits) + ((w / hor_cells) - x) + y * w + (i * w / hor_cells) - 1;
				int t = *p;
				*p = *d;
				*d = t;
			}
}

void BltCanvas::maskColor(COLORREF from, COLORREF to)
{
	int n = bm.bmWidth * bm.bmHeight;
	//GdiFlush();
	DWORD *b = (DWORD *)getBits();
	from &= 0xffffff;
	while (n--)
	{
		if ((*b & 0xffffff) == from)
		{
			*b = to;
		}
		else *b |= 0xff000000;	// force all other pixels non masked
		b++;
	}
}

void BltCanvas::makeAlpha(int newalpha)
{
	int w, h;
	getDim(&w, &h, NULL);
	premultiply((ARGB32 *)getBits(), w*h, newalpha);
}

#if 0
void BltCanvas::premultiply(ARGB32 *m_pBits, int nwords, int newalpha)
{
	if (newalpha == -1)
	{
		for (; nwords > 0; nwords--, m_pBits++)
		{
			unsigned char *pixel = (unsigned char *)m_pBits;
			unsigned int alpha = pixel[3];
			if (alpha == 255) continue;
			pixel[0] = (pixel[0] * alpha) >> 8;	// blue
			pixel[1] = (pixel[1] * alpha) >> 8;	// green
			pixel[2] = (pixel[2] * alpha) >> 8;	// red
		}
	}
	else
	{
		for (; nwords > 0; nwords--, m_pBits++)
		{
			unsigned char *pixel = (unsigned char *)m_pBits;
			pixel[0] = (pixel[0] * newalpha) >> 8;	// blue
			pixel[1] = (pixel[1] * newalpha) >> 8;	// green
			pixel[2] = (pixel[2] * newalpha) >> 8;	// red
			pixel[3] = (pixel[3] * newalpha) >> 8;	// alpha
		}
	}
}
#endif

// benski> this may not be completely safe.  it's meant for skinbitmap::blittorect
// it doesn't take into account skin bitmaps, enveloped bitmaps, or any other things like that
void BltCanvas::DestructiveResize(int w, int h, int nb_bpp)
{
	if (hdc != NULL) 
	{
		SelectObject(hdc, prevbmp);
		prevbmp=0;
	}

	if (ourbmp && hbmp)
	{
			DeleteObject(hbmp);
			hbmp=NULL;
			ourbmp=FALSE;
	}

	// create tha DC
	if (hdc == NULL)
		hdc = CreateCompatibleDC(NULL);

	AllocBitmap(w,h,nb_bpp);

	prevbmp = (HBITMAP)SelectObject(hdc, hbmp);
	if (envelope) envelope->Release();
	envelope=0;
}
