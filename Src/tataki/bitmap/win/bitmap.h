//NONPORTABLE
#ifndef _BITMAP_H
#define _BITMAP_H

#pragma warning( disable: 4251 )
// http://www.unknownroad.com/rtfm/VisualStudio/warningC4251.html

#include <tataki/export.h>
#include <bfc/platform/platform.h>
class ifc_canvas;  // see canvas.h

//#define NO_MMX

class api_region;

// a skinnable bitmap
class TATAKIAPI SkinBitmap
{
public:
	void AddRef();
	void Release();
#ifndef _NOSTUDIO
#ifdef _WIN32
	SkinBitmap(HINSTANCE hInst, int _id, const wchar_t *colorgroup = NULL);  //NONPORTABLE
#endif
	SkinBitmap(const wchar_t *elementname, int cached = 1);
#endif 
	//  SkinBitmap(SkinBitmap *source, int w, int h);
	SkinBitmap(int w, int h, ARGB32 bgcolor = RGBA(255,255,255,255));  //untested --BU
#ifdef _WIN32
	SkinBitmap(HBITMAP bitmap);
	SkinBitmap(HBITMAP bitmap, HDC dc, int has_alpha = 0, void *bits = NULL);
#endif
	SkinBitmap(ARGB32 *bits, int w, int h, bool own=false); // added by benski, use if you have raw image bits
	SkinBitmap(ifc_canvas *canvas);
	~SkinBitmap();

int getWidth() const { return subimage_w == -1 ? fullimage_w : subimage_w; };
int getHeight() const { return subimage_h == -1 ? fullimage_h : subimage_h; };
	int getFullWidth() const { return fullimage_w; };
	int getFullHeight() const { return fullimage_h; };
int getX() const { return x_offset == -1 ? 0 : x_offset; };
int getY() const { return y_offset == -1 ? 0 : y_offset; };
	int getBpp() const { return 32; };
	int getAlpha() const { return has_alpha; };
	void setHasAlpha(int ha);
	virtual void *getBits();
	int isInvalid();

	const wchar_t *getBitmapName();

	void blit(ifc_canvas *canvas, int x, int y);
	void blitAlpha(ifc_canvas *canvas, int x, int y, int alpha = 255);
	// blits a chunk of source into dest rect
	void blitToRect(ifc_canvas *canvas, RECT *src, RECT *dst, int alpha = 255);
	void blitTile(ifc_canvas *canvas, RECT *dest, int xoffs = 0, int yoffs = 0, int alpha = 255);
	void blitRectToTile(ifc_canvas *canvas, RECT *dest, RECT *src, int xoffs = 0, int yoffs = 0, int alpha = 255);
	void stretch(ifc_canvas *canvas, int x, int y, int w, int h);
	void stretchToRect(ifc_canvas *canvas, RECT *r);
	void stretchRectToRect(ifc_canvas *canvas, RECT *src, RECT *dst);

	void stretchToRectAlpha(ifc_canvas *canvas, RECT *r, int alpha = 255);
	void stretchToRectAlpha(ifc_canvas *canvas, RECT *src, RECT *dst, int alpha = 255);
	ARGB32 getPixel(int x, int y);

private:
#ifdef _WIN32
	void bmpToBits(HBITMAP hbmp, HDC defaultDC = NULL);
#endif

	int has_alpha;

	int x_offset, y_offset, subimage_w, subimage_h, fullimage_w, fullimage_h;

	ARGB32 *bits;
	int ownbits;
	int last_failed;
	wchar_t *bitmapname;
	int fromskin;
	size_t references;
	enum
	{
		OWNBITS_NOTOURS =0 ,
		OWNBITS_USEIMGLDR = 1,
		OWNBITS_USESTDFREE = 2,
		OWNBITS_USECFREE = 3,
		OWNBITS_USESYSFREE = 4,
	};
protected:
	bool high_quality_resampling;
};
#ifndef _NOSTUDIO
class HQSkinBitmap : public SkinBitmap
{
public:
	HQSkinBitmap(ARGB32 *bits, int w, int h, bool own=false) : SkinBitmap(bits, w, h, own)
	{
		high_quality_resampling=true;
	}

	HQSkinBitmap(const wchar_t *elementname, int cached = 1) : SkinBitmap(elementname, cached)
	{
		high_quality_resampling=true;
	}
#ifdef _WIN32
	HQSkinBitmap(HINSTANCE hInst, int _id, const wchar_t *colorgroup = NULL) : SkinBitmap(hInst, _id, colorgroup)
	{
		high_quality_resampling=true;
	}
#endif
};
#endif
#endif
