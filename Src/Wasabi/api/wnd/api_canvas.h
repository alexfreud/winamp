#ifndef __WASABI_API_CANVAS_H
#define __WASABI_API_CANVAS_H

#include <bfc/dispatch.h>
#include <bfc/platform/platform.h>
#include <api/service/svcs/svc_font.h> // for STDFONT_* stuff.  should make a std_font thingy later
#include <bfc/std.h> // for WASABI_DEFAULT_FONTNAMEW

namespace Wasabi
{
	// benski> move this to std_font later
struct FontInfo
{
	FontInfo()
	{
		// defaults
		face = WASABI_DEFAULT_FONTNAMEW;
		pointSize = 12;
		bold = 0;
		opaque = false;
		underline = false;
		italic = false;
		alignFlags = STDFONT_LEFT;
		antialias = 1;
		bgColor = RGBA(255, 255, 255, 255);
		color = RGBA(0, 0, 0, 0);
	}

	const wchar_t *face;
	unsigned int pointSize;
	int bold; // bold level
	bool opaque;
	bool underline;
	bool italic;
	int alignFlags;
	int antialias; // anti-alias level
	ARGB32 color;
	ARGB32 bgColor;
};
}

class ifc_window;
// abstract base class: safe to use in API
class NOVTABLE ifc_canvas : public Dispatchable
{
protected:
	ifc_canvas()
	{} // protect constructor
	~ifc_canvas()
	{}

public:
	DISPATCH_CODES
	{
	  GETHDC	= 100,
	  GETROOTWND	= 200,
	  GETBITS	= 300,
	  GETOFFSETS	= 400,
	  ISFIXEDCOORDS	= 500,
	  GETDIM	= 600,
	  GETTEXTFONT	= 700,
	  GETTEXTSIZE	= 710,
	  GETTEXTBOLD	= 720,
	  GETTEXTOPAQUE	= 730,
	  GETTEXTALIGN	= 740,
	  GETTEXTCOLOR	= 750,
	  GETTEXTBKCOLOR	= 760,
	  GETTEXTAA = 770,
	  GETTEXTUNDERLINE	= 780,
	  GETTEXTITALIC	= 790,
	  GETCLIPBOX	= 800,
	};
public:
	HDC getHDC();
	ifc_window *getRootWnd();
	void *getBits();
	void getOffsets(int *x, int *y);
	bool isFixedCoords(); //FG> allows onPaint to handle double buffers as well as normal DCs
	bool getDim(int *w, int *h = NULL, int *p = NULL);  // w & h in pixels, pitch in bytes. 0 on success.
	int getClipBox(RECT *r); // returns 0 if no clipping region
	const wchar_t *getTextFont();
	int getTextSize();
	int getTextBold();
	int getTextAntialias();
	int getTextOpaque();
	int getTextUnderline();
	int getTextItalic();
	int getTextAlign();
	ARGB32 getTextColor();
	ARGB32 getTextBkColor();
};


inline HDC ifc_canvas::getHDC()
{
	return _call(ifc_canvas::GETHDC, (HDC)0);
}
inline ifc_window *ifc_canvas::getRootWnd()
{
	return _call(ifc_canvas::GETROOTWND, (ifc_window*)0);
}
inline void *ifc_canvas::getBits()
{
	return _call(ifc_canvas::GETBITS, (void *)0);
}
inline void ifc_canvas::getOffsets(int *x, int *y)
{
	_voidcall(ifc_canvas::GETOFFSETS, x, y);
}
inline bool ifc_canvas::isFixedCoords()
{ //FG> allows onPaint to handle double buffers as well as normal DCs
	return _call(ifc_canvas::ISFIXEDCOORDS, false);
}
inline bool ifc_canvas::getDim(int *w, int *h, int *p)
{ // w & h in pixels, pitch in bytes. 0 on success.
	return _call(ifc_canvas::GETDIM, false, w, h, p);
}
inline int ifc_canvas::getClipBox(RECT *r)
{ // returns 0 if no clipping region
	return _call(ifc_canvas::GETCLIPBOX, 0, r);
}

inline const wchar_t *ifc_canvas::getTextFont()
{
	return _call(ifc_canvas::GETTEXTFONT, L"");
}
inline int ifc_canvas::getTextSize()
{
	return _call(ifc_canvas::GETTEXTSIZE, -1);
}
inline int ifc_canvas::getTextBold()
{
	return _call(ifc_canvas::GETTEXTBOLD, 0);
}
inline int ifc_canvas::getTextAntialias()
{
	return _call(ifc_canvas::GETTEXTAA, 0);
}
inline int ifc_canvas::getTextOpaque()
{
	return _call(ifc_canvas::GETTEXTOPAQUE, 0);
}
inline int ifc_canvas::getTextUnderline()
{
	return _call(ifc_canvas::GETTEXTUNDERLINE, 0);
}
inline int ifc_canvas::getTextItalic()
{
	return _call(ifc_canvas::GETTEXTITALIC, 0);
}
inline int ifc_canvas::getTextAlign()
{
	return _call(ifc_canvas::GETTEXTALIGN, -1);
}
inline ARGB32 ifc_canvas::getTextColor()
{
	return _call(ifc_canvas::GETTEXTCOLOR, RGB(0, 0, 0));
}
inline ARGB32 ifc_canvas::getTextBkColor()
{
	return _call(ifc_canvas::GETTEXTBKCOLOR, RGB(255, 255, 255));
}
#endif
