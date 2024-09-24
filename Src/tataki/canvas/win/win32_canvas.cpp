#ifndef _WIN32
#error this file is for windows only.  Don't include it in your project/makefile for other platforms
#else
#include <tataki/export.h>
#include <tataki/api__tataki.h>
#include <tataki/blending/blending.h>
#include "canvas.h"
#include <tataki/bitmap/bitmap.h>
#include <tataki/region/region.h>
#include <api/wnd/basewnd.h>
#include <api/wnd/fontdef.h>
#include <api/wnd/paintsets.h>

#include <bfc/assert.h>

#include "bltcanvas.h"
#include <nsutil/alpha.h>
#include <nsutil/image.h>

#define CBCLASS Canvas
START_DISPATCH;
CB(GETHDC, getHDC);
CB(GETROOTWND, getRootWnd);
CB(GETBITS, getBits);
VCB(GETOFFSETS, getOffsets);
CB(ISFIXEDCOORDS, isFixedCoords);
CB(GETDIM, getDim);
CB(GETTEXTFONT, getTextFont);
CB(GETTEXTSIZE, getTextSize);
CB(GETTEXTBOLD, getTextBold);
CB(GETTEXTOPAQUE, getTextOpaque);
CB(GETTEXTUNDERLINE, getTextUnderline);
CB(GETTEXTITALIC, getTextItalic);
CB(GETTEXTALIGN, getTextAlign);
CB(GETTEXTCOLOR, getTextColor);
CB(GETTEXTBKCOLOR, getTextBkColor);
CB(GETTEXTAA, getTextAntialias);
CB(GETCLIPBOX, getClipBox);
END_DISPATCH;
#undef CBCLASS

//NONPORTABLE

extern const wchar_t wasabi_default_fontnameW[];

Canvas::Canvas()
		: hdc(NULL),
		bits(NULL),
		srcwnd(NULL),
		fcoord(FALSE),
		xoffset(0),		yoffset(0),
		width(0),
		height(0),
		pitch(0),
		defpen(NULL),
		curpen(NULL),
		userFontInfo(0)
{

	//tfont = new String; // using dynamic tfont here coz we need to manage em with stack, so stacking fonts won't take sizeof(String) and their destruction will not fuxor everything
	//tfont->setValue(wasabi_default_fontname);
}

Canvas::~Canvas()
{
	if (getHDC() && defpen != NULL)
	{
		SelectObject(getHDC(), defpen);
		DeleteObject(curpen);
	}

	if (!penstack.isempty())
		DebugStringW(L"Pen stack not empty in Canvas::~Canvas !");
}

void Canvas::setBaseWnd(BaseWnd *b)
{
	srcwnd = b;
}

HDC Canvas::getHDC()
{
	return hdc;
}

ifc_window *Canvas::getRootWnd()
{
	return srcwnd;
}

void *Canvas::getBits()
{
	return bits;
}

bool Canvas::getDim(int *w, int *h, int *p)
{
	if (w) *w = width;
	if (h) *h = height;
	if (p) *p = pitch;
	return FALSE;
}

void Canvas::getOffsets(int *x, int *y)
{
	if (x != NULL) *x = getXOffset();
	if (y != NULL) *y = getYOffset();
}

bool Canvas::isFixedCoords()
{
	return fcoord;
}

BaseWnd *Canvas::getBaseWnd()
{
	return srcwnd;
}

void Canvas::fillRgn(RegionI *r, COLORREF color)
{
	ASSERT(r != NULL);
	HBRUSH brush = CreateSolidBrush(color);
	FillRgn(hdc, r->getOSHandle(), brush);
	DeleteObject(brush);
}

void Canvas::fillRect(const RECT *r, COLORREF color)
{
	ASSERT(r != NULL);
#if 0
	HBRUSH brush;
	if (color == RGB(0, 0, 0))
	{
		FillRect(hdc, r, (HBRUSH)GetStockObject(BLACK_BRUSH));
		return ;
	}
	RECT rr = *r;
	offsetRect(&rr);

	brush = CreateSolidBrush(color);
	FillRect(hdc, &rr, brush);
	DeleteObject(brush);
#else
// see: http://ooeygui.typepad.com/ooey_gui/2005/06/tip_fast_solid_.html
COLORREF clrOld = SetBkColor(hdc, color);
RECT rr = *r;
offsetRect(&rr);
ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rr, NULL, 0, NULL);
SetBkColor(hdc, clrOld);
#endif
}

void Canvas::fillRectAlpha(const RECT *r, COLORREF color, int alpha)
{
	RECT blitr;
	RECT clipr;
	getClipBox(&clipr);
	IntersectRect(&blitr, &clipr, r);
	uint8_t *bits8 = (uint8_t *)(bits) + blitr.left*4 + blitr.top * pitch;
	nsutil_image_FillRectAlpha_RGB32((RGB32 *)(bits8), pitch, blitr.right-blitr.left, blitr.bottom-blitr.top, color, alpha);
}

void Canvas::drawRect(const RECT *r, int solid, COLORREF color, int alpha)
{
#if 0
	unsigned int blah = (unsigned int)alpha;
	color = RGBTOBGR(color);
	color = (color & 0xFFFFFF) | (blah << 24);
	BltCanvas::premultiply(&color, 1);
	int ox, oy;
	getOffsets(&ox, &oy);
	int w, h, pitch;
	getDim(&w, &h, &pitch);
	RECT _r = *r;
	_r.right = MIN<int>(r->right, w);
	_r.bottom = MIN<int>(r->bottom, h);
	int _l = r->bottom - r->top;
	int m = _r.bottom - _r.top;
	int l = _l;
	pitch /= 4;
	int *p = (int *)bits + ox + r->left + (oy + r->top) * pitch;
	int n = r->right - r->left;
	int maxn = _r.right - _r.left;
	while (l-- && m--)
	{
		int _n = maxn;
		if (l == _l - 1 || !l)
		{
			if (solid)
			{
				while (_n--)
				{
					*p = Blenders::BLEND_ADJ2(*p, color);
					p++;
				}
			}
			else
			{
				while (_n--)
				{
					if (_n % 2) *p = Blenders::BLEND_ADJ2(*p, color);
					p++;
				}
			}
			p += n - maxn;
		}
		else
		{
			if (solid || l % 2)
				*p = Blenders::BLEND_ADJ2(*p, color);
			p += n - 1;
			if (n == maxn && (solid || l % 2))
				*p = Blenders::BLEND_ADJ2(*p, color);
			p++;
		}
		p += pitch - n;
	}
#else
HBRUSH oldbrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
HPEN oldpen, pen;
pen = CreatePen(solid ? PS_SOLID : PS_DOT, 0, color);
oldpen = (HPEN)SelectObject(hdc, pen);
ASSERT(r != NULL);
RECT rr = *r;
offsetRect(&rr);
Rectangle(hdc, rr.left, rr.top, rr.right, rr.bottom);
SelectObject(hdc, oldpen);
SelectObject(hdc, oldbrush);
DeleteObject(pen);
#endif
}

int Canvas::getTextAlign()
{
	return getFontInfo()->alignFlags;
}

int Canvas::getTextOpaque()
{
	return getFontInfo()->opaque;
}


int Canvas::getTextUnderline()
{
	return getFontInfo()->underline;
}

int Canvas::getTextItalic()
{
	return getFontInfo()->italic;
}

int Canvas::getTextBold()
{
	return getFontInfo()->bold;
}

int Canvas::getTextAntialias()
{
	return getFontInfo()->antialias;
}

void Canvas::pushPen(COLORREF color)
{
	pushPen(PENSTYLE_SOLID, 1, color);
}

void Canvas::pushPen(int style, int width, COLORREF color)
{
	ASSERT(getHDC() != NULL);
	penstyle = style;
	penwidth = width;
	pencolor = color;
	penstruct s;
	curpen = CreatePen(style, width, color);
	HPEN oldpen = (HPEN)SelectObject(getHDC(), curpen);
	s.style = style;
	s.width = width;
	s.color = color;
	s.hpen = oldpen;
	penstack.push(s);
}

void Canvas::popPen()
{
	ASSERT(getHDC() != NULL);
	if (penstack.isempty()) return ;
	penstruct s;
	penstack.pop(&s);
	SelectObject(getHDC(), s.hpen);
	DeleteObject(curpen);
}

int Canvas::getPenStyle()
{
	return penstyle;
}

COLORREF Canvas::getPenColor()
{
	return pencolor;
}

int Canvas::getPenWidth()
{
	return penwidth;
}

COLORREF Canvas::getTextColor()
{
	return getFontInfo()->color;
}

COLORREF Canvas::getTextBkColor()
{
	return getFontInfo()->bgColor;
}

int Canvas::getTextSize()
{
	return getFontInfo()->pointSize;
}

const wchar_t *Canvas::getTextFont()
{
	return getFontInfo()->face;
}

void Canvas::moveTo(int x, int y)
{
	MoveToEx(hdc, x, y, NULL);
}

void Canvas::lineTo(int x, int y)
{
	LineTo(hdc, x, y);
}

void Canvas::lineDraw(int fromX, int fromY, int toX, int toY)
{
	MoveToEx(hdc, fromX, fromY, NULL);
	LineTo(hdc, toX, toY);
}

void Canvas::drawSysObject(const RECT *r, int sysobj, int alpha)
{
#ifndef _NOSTUDIO
	RECT i_dont_trust_ms_with_my_rect = *r;
	switch (sysobj)
	{
	case DrawSysObj::BUTTON:
		WASABI_API_WND->paintset_render(Paintset::BUTTONUP, this, r, alpha);
		break;
	case DrawSysObj::BUTTON_PUSHED:
		WASABI_API_WND->paintset_render(Paintset::BUTTONDOWN, this, r, alpha);
		break;
	case DrawSysObj::BUTTON_DISABLED:
		WASABI_API_WND->paintset_render(Paintset::BUTTONDISABLED, this, r, alpha);
		break;
#ifdef WIN32
	case DrawSysObj::OSBUTTON:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_BUTTON, DFCS_BUTTONPUSH);
	}
	break;
	case DrawSysObj::OSBUTTON_PUSHED:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
	}
	break;
	case DrawSysObj::OSBUTTON_DISABLED:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_INACTIVE);
	}
	break;

	case DrawSysObj::OSBUTTON_CLOSE:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_CAPTION, DFCS_CAPTIONCLOSE);
	}
	break;
	case DrawSysObj::OSBUTTON_CLOSE_PUSHED:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_CAPTION, DFCS_CAPTIONCLOSE | DFCS_PUSHED);
	}
	break;
	case DrawSysObj::OSBUTTON_CLOSE_DISABLED:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_CAPTION, DFCS_CAPTIONCLOSE | DFCS_INACTIVE);
	}
	break;

	case DrawSysObj::OSBUTTON_MINIMIZE:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_CAPTION, DFCS_CAPTIONMIN);
	}
	break;
	case DrawSysObj::OSBUTTON_MINIMIZE_PUSHED:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_CAPTION, DFCS_CAPTIONMIN | DFCS_PUSHED);
	}
	break;
	case DrawSysObj::OSBUTTON_MINIMIZE_DISABLED:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_CAPTION, DFCS_CAPTIONMIN | DFCS_INACTIVE);
	}
	break;

	case DrawSysObj::OSBUTTON_MAXIMIZE:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_CAPTION, DFCS_CAPTIONMAX);
	}
	break;
	case DrawSysObj::OSBUTTON_MAXIMIZE_PUSHED:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_CAPTION, DFCS_CAPTIONMAX | DFCS_PUSHED);
	}
	break;
	case DrawSysObj::OSBUTTON_MAXIMIZE_DISABLED:
	{
		DrawFrameControl(getHDC(), &i_dont_trust_ms_with_my_rect, DFC_CAPTION, DFCS_CAPTIONMAX | DFCS_INACTIVE);
	}
	break;
#else
#error port me!
#endif
	break;
	}
#endif
}

void Canvas::textOut(int x, int y, const wchar_t *txt, const Wasabi::FontInfo *fontInfo)
{
	userFontInfo = fontInfo;
	WASABI_API_FONT->font_textOut(this, WA_FONT_TEXTOUT_NORMAL, x, y, 0, 0, txt);
	userFontInfo = 0;
}

void Canvas::textOut(int x, int y, int w, int h, const wchar_t *txt, const Wasabi::FontInfo *fontInfo)
{
	userFontInfo = fontInfo;
	WASABI_API_FONT->font_textOut(this, WA_FONT_TEXTOUT_RECT, x, y, w, h, txt);
	userFontInfo = 0;
}

void Canvas::textOutEllipsed(int x, int y, int w, int h, const wchar_t *txt, const Wasabi::FontInfo *fontInfo)
{
	userFontInfo = fontInfo;
	WASABI_API_FONT->font_textOut(this, WA_FONT_TEXTOUT_ELLIPSED, x, y, w, h, txt);
	userFontInfo = 0;
}

void Canvas::textOutWrapped(int x, int y, int w, int h, const wchar_t *txt, const Wasabi::FontInfo *fontInfo)
{
	userFontInfo = fontInfo;
	WASABI_API_FONT->font_textOut(this, WA_FONT_TEXTOUT_WRAPPED, x, y, w, h, (txt));
	userFontInfo = 0;
}

void Canvas::textOutWrappedPathed(int x, int y, int w, const wchar_t *txt, const Wasabi::FontInfo *fontInfo)
{
	userFontInfo = fontInfo;
	WASABI_API_FONT->font_textOut(this, WA_FONT_TEXTOUT_WRAPPEDPATHED, x, y, w, 0, (txt));
	userFontInfo = 0;
}

void Canvas::textOutCentered(RECT *r, const wchar_t *txt, const Wasabi::FontInfo *fontInfo)
{
	userFontInfo = fontInfo;
	WASABI_API_FONT->font_textOut(this, WA_FONT_TEXTOUT_CENTERED, r->left, r->top, r->right, r->bottom, (txt));
	userFontInfo = 0;
}

int Canvas::getTextWidth(const wchar_t *text, const Wasabi::FontInfo *fontInfo)
{
	userFontInfo = fontInfo;
	int ret = WASABI_API_FONT->font_getInfo(this, fontInfo->face, WA_FONT_GETINFO_WIDTH, (text), NULL, NULL);
	userFontInfo = 0;
	return ret;
}

int Canvas::getTextHeight(const wchar_t *text, const Wasabi::FontInfo *fontInfo)
{
	userFontInfo = fontInfo;
	int ret = WASABI_API_FONT->font_getInfo(this, fontInfo->face, WA_FONT_GETINFO_HEIGHT, (text), NULL, NULL);
	userFontInfo = 0;
	return ret;
}

void Canvas::getTextExtent(const wchar_t *txt, int *w, int *h, const Wasabi::FontInfo *fontInfo)
{
	userFontInfo = fontInfo;
	WASABI_API_FONT->font_getInfo(this, fontInfo->face, WA_FONT_GETINFO_WIDTHHEIGHT, (txt), w, h);
	userFontInfo = 0;
}

void Canvas::offsetRect(RECT *r)
{
	ASSERT(r != NULL);
	r->left += xoffset;
	r->right += xoffset;
	r->top += yoffset;
	r->bottom += yoffset;
}

void Canvas::selectClipRgn(api_region *r)
{
	SelectClipRgn(hdc, r ? r->getOSHandle() : NULL);
}

int Canvas::getClipBox(RECT *r)
{
	RECT dummy;
	if (!r) r = &dummy;
	return GetClipBox(hdc, r);
}

int Canvas::getClipRgn(api_region *r)
{
	ASSERT(r != NULL);
	return GetClipRgn(hdc, r->getOSHandle());
}

//FG> added blit canvas to canvas
void Canvas::blit(int srcx, int srcy, Canvas *dest, int dstx, int dsty, int dstw, int dsth)
{
	char *srcbits = (char *)getBits();
	char *destbits = (char *)dest->getBits();
	RECT clipr;
	if (srcbits && destbits && GetClipBox(dest->getHDC(), &clipr) == SIMPLEREGION)
	{
		int srcimg_w, srcimg_h, srcimg_p;
		getDim(&srcimg_w, &srcimg_h, &srcimg_p);
		int dstimg_w, dstimg_h, dstimg_p;
		dest->getDim(&dstimg_w, &dstimg_h, &dstimg_p);

		if (srcx < 0)
		{
			dstx -= srcx; dstw += srcx; srcx = 0;
		}
		if (srcy < 0)
		{
			dsty -= srcy; dsth += srcy; srcy = 0;
		}
		if (srcx + dstw >= srcimg_w) dstw = srcimg_w - srcx;
		if (srcy + dsth >= srcimg_h) dsth = srcimg_h - srcy;

		if (dstx < clipr.left)
		{
			srcx += clipr.left - dstx; dstw -= clipr.left - dstx; dstx = clipr.left;
		}
		if (dsty < clipr.top)
		{
			srcy += clipr.top - dsty; dsth -= clipr.top - dsty; dsty = clipr.top;
		}

		if (dstx + dstw >= clipr.right) dstw = clipr.right - dstx;
		if (dsty + dsth >= clipr.bottom) dsth = clipr.bottom - dsty;

		if (!dstw || !dsth) return ;

		int y;
		int yl = dsty + dsth;
		for (y = dsty; y < yl; y++)
		{
			MEMCPY32(destbits + y*dstimg_p + dstx*4, srcbits + srcy*srcimg_p + srcx*4, dstw);
			srcy++;
		}
	}
	else
	{
		//GdiFlush();
		BitBlt(dest->getHDC(), dstx, dsty, dstw, dsth, getHDC(), srcx, srcy, SRCCOPY);
	}
}

#pragma comment(lib, "msimg32.lib")

void Canvas::stretch(ifc_canvas *canvas, int x, int y, int w, int h)
{
	if (bits)
	{
		SkinBitmap temp((ARGB32 *)bits, width, height);
		temp.stretch(canvas, x,y,w,h);
	}
	else
	{
		BLENDFUNCTION blendFn;
		blendFn.BlendOp = AC_SRC_OVER;
		blendFn.BlendFlags  = 0;
		blendFn.SourceConstantAlpha  = 255;
		blendFn.AlphaFormat = AC_SRC_ALPHA;

		AlphaBlend(canvas->getHDC(),
		           x, y,
		           w, h,
		           getHDC(),
		           0, 0,
		           width, height,
		           blendFn);
	}
}

void Canvas::blitAlpha(ifc_canvas *canvas, int x, int y, int alpha)
{
	if (bits)
	{
		SkinBitmap temp((ARGB32 *)bits, width, height);
		temp.blitAlpha(canvas, x,y,alpha);
	}
	else
	{
		BLENDFUNCTION blendFn;
		blendFn.BlendOp = AC_SRC_OVER;
		blendFn.BlendFlags  = 0;
		blendFn.SourceConstantAlpha  = alpha;
		blendFn.AlphaFormat = AC_SRC_ALPHA;

		AlphaBlend(canvas->getHDC(),
		           x, y,
		           width, height,
		           getHDC(),
		           0, 0,
		           width, height,
		           blendFn);
	}
}

void Canvas::stretchToRectAlpha(ifc_canvas *canvas, RECT *src, RECT *dst, int alpha)
{
	if (bits)
	{
		SkinBitmap temp((ARGB32 *)bits, width, height);
		temp.stretchToRectAlpha(canvas, src, dst, alpha);
	}
	else
	{
		BLENDFUNCTION blendFn;
		blendFn.BlendOp = AC_SRC_OVER;
		blendFn.BlendFlags  = 0;
		blendFn.SourceConstantAlpha  = alpha;
		blendFn.AlphaFormat = AC_SRC_ALPHA;

		AlphaBlend(canvas->getHDC(),
		           dst->left, dst->top,
		           dst->right - dst->left, dst->bottom - dst->top,
		           getHDC(),
		           src->left, src->top,
		           src->right - src->left, src->bottom - src->top,
		           blendFn);
	}
}

void Canvas::blitToRect(ifc_canvas *canvas, RECT *src, RECT *dst, int alpha)
{
	if (bits)
	{
		SkinBitmap temp((ARGB32 *)bits, width, height);
		temp.blitToRect(canvas, src, dst, alpha);
	}
	else
	{
		BLENDFUNCTION blendFn;
		blendFn.BlendOp = AC_SRC_OVER;
		blendFn.BlendFlags  = 0;
		blendFn.SourceConstantAlpha  = alpha;
		blendFn.AlphaFormat = AC_SRC_ALPHA;

		AlphaBlend(canvas->getHDC(),
		           dst->left, dst->top,
		           dst->right - dst->left, dst->bottom - dst->top,
		           getHDC(),
		           src->left, src->top,
		           src->right - src->left, src->bottom - src->top,
		           blendFn);
	}
}

// src* are in fixed point
static void scale_internal(int srcx, int srcy, int srcw, int srch, void *srcdib, int srcdib_w, int srcdib_h, int srcdib_p, int dstx, int dsty, int dstw, int dsth, void *dstdib, int nofilter)
{
	// scaling up
	if ((dstw << 16) >= srcw && (dsth << 16) >= srch)
	{
		int y;
		int SY, dX, dY;
		int Xend = (srcdib_w - 2) << 16;
		SY = srcy;
		dX = srcw / dstw;
		dY = srch / dsth;

		int xstart = 0;
		int xp = srcx >> 16;
		if (xp < 0)
		{
			xstart = -xp;
			srcx += xstart * dX;
		}

		int xend = dstw;
		xp = (srcx + (dX * (xend - xstart))) >> 16;
		if (xp > srcdib_w)
		{
			xend = xstart + srcdib_w - (srcx >> 16);
		}

		for (y = 0; y < dsth; y ++)
		{
			int yp = (SY >> 16);
			if (yp >= 0)
			{
				int x;
				int SX = srcx;
				unsigned int *out = (unsigned int*)dstdib + xstart + y * dstw;
				int end = yp >= srcdib_h - 1;
				if (nofilter || end)
				{
					if (end) yp = srcdib_h - 1;
					unsigned int *in = (unsigned int*)((char *)srcdib + yp * srcdib_p);
					for (x = xstart; x < xend; x ++) // quick hack to draw last line
					{
						*out++ = in[SX >> 16];
						SX += dX;
					}
					if (end) break;
				}
				else
				{
					unsigned int *in = (unsigned int*)((char *)srcdib + yp * srcdib_p);

#ifndef NO_MMX
					if (Blenders::MMX_AVAILABLE())
					{
						for (x = xstart; x < xend; x ++)
						{
							if (SX > Xend) *out++ = Blenders::BLEND4_MMX(in + (Xend >> 16), srcdib_w, 0xffff, SY);
							else *out++ = Blenders::BLEND4_MMX(in + (SX >> 16), srcdib_w, SX, SY);
							SX += dX;
						}
					}
					else
#endif

					{
						for (x = xstart; x < xend; x ++)
						{
							if (SX > Xend) *out++ = Blenders::BLEND4(in + (Xend >> 16), srcdib_w, 0xffff, SY);
							else *out++ = Blenders::BLEND4(in + (SX >> 16), srcdib_w, SX, SY);
							SX += dX;
						}
					}
				}
			}
			SY += dY;
		}
		// end of scaling up
	}
	else // we are scaling down -- THIS IS SLOW AND MAY BREAK THINGS. :)
	{
		int y;
		int SY, dX, dY;
		SY = srcy;
		dX = srcw / dstw;
		dY = srch / dsth;

		int xstart = 0;
		int xp = srcx >> 16;
		if (xp < 0)
		{
			xstart = -xp;
			srcx += xstart * dX;
		}

		int xend = dstw;
		xp = (srcx + (dX * (xend - xstart))) >> 16;
		if (xp > srcdib_w)
		{
			xend = xstart + srcdib_w - (srcx >> 16);
		}

		for (y = 0; y < dsth; y ++)
		{
			// start and end of y source block
			int vStart = SY;
			int vEnd = SY + dY;

			int x;
			int SX = srcx;
			unsigned char *out = (unsigned char *)((unsigned int*)dstdib + xstart + y * dstw);
			for (x = xstart; x < xend; x ++)
			{
				if (((char *)out+4) >= ((char *)dstdib + 4*dstw*dsth))
					break;
				int uStart = SX;
				int uEnd = SX + dX;
				// calculate sum of rectangle.

				int cnt = 0;
				__int64 accum[4] = {0, };
				int v, u;
				for (v = vStart; v < vEnd; v += 65536)
				{
					unsigned int vscale = 65535;

					if (v == vStart)
					{
						vscale = 65535 - (v & 0xffff);
					}
					else if ((vEnd - v) < 65536)
					{
						vscale = (vEnd - v) & 0xffff;
					}

					int vp = v >> 16;
					unsigned char *in = (unsigned char*)((char *)srcdib + vp * srcdib_p + 4 * (uStart >> 16));
					for (u = uStart; u < uEnd; u += 65536)
					{
						if (((char *)in+4) >= ((char *)srcdib + srcdib_p*srcdib_h))
							break;
						unsigned int uscale = vscale;
						if (u == uStart)
						{
							uscale *= 65535 - (u & 0xffff);
							uscale >>= 16;
						}
						else if ((uEnd - u) < 65536)
						{
							uscale *= (uEnd - u) & 0xffff;
							uscale >>= 16;
						}
						cnt += uscale;
						if (uscale == 65535)
						{
							accum[0] += (in[0] << 16) - in[0];
							accum[1] += (in[1] << 16) - in[1];
							accum[2] += (in[2] << 16) - in[2];
							accum[3] += (in[3] << 16) - in[3];
						}
						else
						{
							accum[0] += in[0] * uscale;
							accum[1] += in[1] * uscale;
							accum[2] += in[2] * uscale;
							accum[3] += in[3] * uscale;
						}
						in += 4;
					}
				}
				if (!cnt) cnt++;

				out[0] = (uint8_t)(accum[0] / cnt);
				out[1] = (uint8_t)(accum[1] / cnt);
				out[2] = (uint8_t)(accum[2] / cnt);
				out[3] = (uint8_t)(accum[3] / cnt);
				out += 4;

				SX += dX;
			}
			SY += dY;
		}
		// end of scaling down
	}

#ifndef NO_MMX
	Blenders::BLEND_MMX_END();
#endif
}


// src* are in fixed point
void Canvas::stretchblit(int srcx, int srcy, int srcw, int srch, Canvas *dest, int dstx, int dsty, int dstw, int dsth)
{
	//GdiFlush();
	int done = 0;
	void *srcdib = getBits();

	if (!dstw || !dsth || !srcw || !srch) return ;
	if (srcdib)
	{
		int srcdib_w, srcdib_h, srcdib_p;
		getDim(&srcdib_w, &srcdib_h, &srcdib_p);

		void *dstdib;
		BITMAPINFO dstbmi = {0};
		HDC hMemDC;
		HBITMAP hsrcdib;
		dstbmi.bmiHeader.biSize = sizeof(dstbmi.bmiHeader);
		dstbmi.bmiHeader.biWidth = dstw;
		dstbmi.bmiHeader.biHeight = -ABS(dsth);
		dstbmi.bmiHeader.biPlanes = 1;
		dstbmi.bmiHeader.biBitCount = 32;
		dstbmi.bmiHeader.biCompression = BI_RGB;
		hMemDC = CreateCompatibleDC(NULL);
		hsrcdib = CreateDIBSection(hMemDC, &dstbmi, DIB_RGB_COLORS, &dstdib, NULL, 0);
		if (hsrcdib)
		{
			HBITMAP hprev = (HBITMAP)SelectObject(hMemDC, hsrcdib);

			scale_internal(srcx,srcy,srcw,srch,srcdib,srcdib_w,srcdib_h,srcdib_p,dstx,dsty,dstw,dsth,dstdib,0);

			BitBlt(dest->getHDC(), dstx, dsty, dstw, dsth, hMemDC, 0, 0, SRCCOPY);
			done++;

			SelectObject(hMemDC, hprev);
			DeleteObject(hsrcdib);
		}
		DeleteDC(hMemDC);
	}

	if (!done)
	{
		SetStretchBltMode(dest->getHDC(), COLORONCOLOR);
		StretchBlt(dest->getHDC(), dstx, dsty, dstw, dsth, getHDC(), srcx >> 16, srcy >> 16, srcw >> 16, srch >> 16, SRCCOPY);
	}
}

#define DEBUG_SCREEN_SHIFT 0
void Canvas::debug()
{
	SysCanvas c;
	int w, h;
	getDim(&w, &h, NULL);
	blit(0, 0, &c, DEBUG_SCREEN_SHIFT, 0, w, h);
}

#define BF2 (~((3<<24)|(3<<16)|(3<<8)|3))

void Canvas::antiAliasTo(Canvas *dest, int w, int h, int aafactor)
{
	ASSERT(aafactor != 0);
	if (aafactor == 1)
	{
		blit(0, 0, dest, 0, 0, w, h);
		return ;
	}
	ASSERT(getBits() != NULL);
	ASSERT(dest->getBits() != NULL);
	if (getBits() == NULL || dest->getBits() == NULL) return ;
	ASSERTPR(aafactor <= 2, "too lazy to generalize the code right now :)");
	//GdiFlush();
	// we should really store the bpp too
	int aaw = w * aafactor;
	unsigned long *s1 = (unsigned long *)getBits(), *s2 = s1 + 1;
	unsigned long *s3 = s1 + aaw, *s4 = s3 + 1;
	unsigned long *d = (unsigned long *)dest->getBits();
#if 1
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			unsigned long tmp = ((*s1 & BF2) >> 2) + ((*s2 & BF2) >> 2) + ((*s3 & BF2) >> 2) + ((*s4 & BF2) >> 2);
			*d++ = tmp;

			s1 += 2; s2 += 2;
			s3 += 2; s4 += 2;
		}
		s1 += aaw; s2 += aaw;
		s3 += aaw; s4 += aaw;
	}
#else
for (int x = 0; x < w * h; x++) d[x] = s1[x];
#endif
}

void Canvas::colorToColor(COLORREF from, COLORREF to, RECT *r)
{
	int w, h, ox, oy;

	// convert to bitmap order
	from = RGBTOBGR(from);
	to = RGBTOBGR(to);

	COLORREF *p;
	getDim(&w, &h, NULL);
	p = (COLORREF *)getBits();
	getOffsets(&ox, &oy);
	p += ox + r->left + (oy + r->top) * w;
	int rw = r->right - r->left;
	for (int j = r->top;j < r->bottom;j++)
	{
		for (int i = r->left;i < r->right;i++)
		{
			if (*p == from)
				*p = to;
			p++;
		}
		p += w - rw;
	}
}

double Canvas::getSystemFontScale()
{
	if (WASABI_API_CONFIG)
	{
		int v = WASABI_API_CONFIG->getIntPublic(L"manualsysmetrics", -1);
		if (v != -1) return (v / 100.0f);
	}

	int nLogDPIX = GetDeviceCaps(getHDC(), LOGPIXELSX);
	return ((float)nLogDPIX / 96.0f);
}

void Canvas::premultiply(ARGB32 *m_pBits, int nwords, int newalpha)
{
	if (newalpha == -1)
	{
		nsutil_alpha_Premultiply_RGB32(m_pBits, nwords, nwords, 1);
		/*
		for (; nwords > 0; nwords--, m_pBits++)
		{
			unsigned char *pixel = (unsigned char *)m_pBits;
			unsigned int alpha = pixel[3];
			if (alpha == 255) continue;
			pixel[0] = (pixel[0] * alpha) >> 8;	// blue
			pixel[1] = (pixel[1] * alpha) >> 8;	// green
			pixel[2] = (pixel[2] * alpha) >> 8;	// red
		}
		*/
	}
	else
	{
		nsutil_alpha_PremultiplyValue_RGB8(m_pBits, nwords, nwords, 1, newalpha);
		/*
		for (; nwords > 0; nwords--, m_pBits++)
		{
			unsigned char *pixel = (unsigned char *)m_pBits;
			pixel[0] = (pixel[0] * newalpha) >> 8;	// blue
			pixel[1] = (pixel[1] * newalpha) >> 8;	// green
			pixel[2] = (pixel[2] * newalpha) >> 8;	// red
			pixel[3] = (pixel[3] * newalpha) >> 8;	// alpha
		}
		*/
	}
}

TextInfoCanvas::TextInfoCanvas(BaseWnd *basewnd)
{
	ASSERT(basewnd != NULL);
	hWnd = basewnd->gethWnd();
	hdc = GetDC(hWnd);
}

TextInfoCanvas::~TextInfoCanvas()
{
	if (hdc)
		ReleaseDC(hWnd, hdc);
}

WndCanvas::WndCanvas(BaseWnd *basewnd)
{
	attachToClient(basewnd);
}

WndCanvas::WndCanvas()
{
	hWnd = NULL;
}

WndCanvas::~WndCanvas()
{
	if (hWnd != NULL && hdc != NULL) ReleaseDC(hWnd, hdc);
	hdc = NULL;
}

int WndCanvas::attachToClient(BaseWnd *basewnd)
{
	if (basewnd == NULL)
		return 0;

	hWnd = basewnd->gethWnd();
	if (hWnd == NULL)
		return 0;

	hdc = GetDC(hWnd);
	if (hdc == NULL)
		return 0;

	srcwnd = basewnd;
	return 1;
}

#if 0//CUT
int WndCanvas::attachToWnd(HWND _hWnd)
{
	hWnd = _hWnd;
	ASSERT(hWnd != NULL);
	hdc = GetWindowDC(hWnd);
	ASSERT(hdc != NULL);
	return 1;
}
#endif

PaintCanvas::PaintCanvas()
{
	hWnd = NULL;
}

PaintCanvas::~PaintCanvas()
{

	if (hdc != NULL) EndPaint(hWnd, &ps);
	hdc = NULL;
}

void PaintCanvas::getRcPaint(RECT *r)
{
	*r = ps.rcPaint;
}

int PaintCanvas::beginPaint(BaseWnd *basewnd)
{
	hWnd = basewnd->gethWnd();	// NONPORTABLE
	ASSERT(hWnd != NULL);
	hdc = BeginPaint(hWnd, &ps);
	ASSERT(hdc != NULL);
	srcwnd = basewnd;
	return 1;
}

int PaintCanvas::beginPaint(HWND wnd)
{
	hWnd = wnd;	// NONPORTABLE
	ASSERT(hWnd != NULL);
	hdc = BeginPaint(hWnd, &ps);
	ASSERT(hdc != NULL);
	srcwnd = NULL;
	return 1;
}

PaintBltCanvas::PaintBltCanvas()
{
	hWnd = NULL;
	wnddc = NULL;
	hbmp = NULL;
	prevbmp = NULL;
	bits = NULL;
	fcoord = TRUE;
	nonclient = FALSE;
}

PaintBltCanvas::~PaintBltCanvas()
{
	RECT r;

	if (hdc == NULL) return ;

	ASSERT(srcwnd != NULL);
	if (nonclient) //FG> nonclient painting fix
		srcwnd->getNonClientRect(&r);
	else
		srcwnd->getClientRect(&r);

	// blt here
	//GdiFlush();
	BitBlt(wnddc, r.left, r.top, r.right - r.left, r.bottom - r.top, hdc, 0, 0, SRCCOPY);

	//SelectClipRgn(hdc, NULL);
	// kill the bitmap and its DC
	SelectObject(hdc, prevbmp);
	DeleteDC(hdc);
	hdc = NULL;
	DeleteObject(hbmp);
	bits = NULL;
	width = 0;
	height = 0;
	pitch = 0;

	EndPaint(hWnd, &ps);	// end of wnddc
	wnddc = NULL;
}

//FG> nonclient painting fix
int PaintBltCanvas::beginPaintNC(BaseWnd *basewnd)
{
	nonclient = TRUE;
	return beginPaint(basewnd);
}

void PaintBltCanvas::getRcPaint(RECT *r)
{
	*r = ps.rcPaint;
}

int PaintBltCanvas::beginPaint(BaseWnd *basewnd)
{

	RECT r;

	if (nonclient)
		basewnd->getNonClientRect(&r); //FG> nonclient painting fix
	else
		basewnd->getClientRect(&r);

	if (r.right - r.left <= 0 || r.bottom - r.top <= 0) return 0;

	hWnd = basewnd->gethWnd();	// NONPORTABLE
	ASSERT(hWnd != NULL);

	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof bmi);
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = r.right - r.left;
	bmi.bmiHeader.biHeight = -(r.bottom - r.top);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;

	wnddc = BeginPaint(hWnd, &ps);

	ASSERT(wnddc != NULL);

	//GdiFlush();
	width = r.right - r.left;
	height = -ABS(r.bottom - r.top);
	pitch = width * 4;
	hbmp = CreateDIBSection(wnddc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);

	if (hbmp == NULL)
	{
		EndPaint(hWnd, &ps);	// end of wnddc
		wnddc = NULL;
		return 0;
	}

	// create tha DC
	hdc = CreateCompatibleDC(wnddc);
	if (hdc == NULL)
	{
		DeleteObject(hbmp);
		EndPaint(hWnd, &ps);	// end of wnddc
		wnddc = NULL;
		return 0;
	}
	prevbmp = (HBITMAP)SelectObject(hdc, hbmp);

	RegionI clip(&ps.rcPaint);

	selectClipRgn(&clip);

	srcwnd = basewnd;

	return 1;
}

void *PaintBltCanvas::getBits()
{
	return bits;
}

MemCanvas::MemCanvas()
{}

MemCanvas::~MemCanvas()
{
	DeleteDC(hdc);
	hdc = NULL;
}

int MemCanvas::createCompatible(Canvas *canvas)
{
	ASSERT(canvas != NULL);
	ASSERT(canvas->getHDC() != NULL);
	hdc = CreateCompatibleDC(canvas->getHDC());
	ASSERT(hdc != NULL);
	srcwnd = canvas->getBaseWnd();
	return 1;
}


DCCanvas::DCCanvas(HDC clone, BaseWnd *srcWnd)
{
	if (clone != NULL) cloneDC(clone, srcWnd);
}

DCCanvas::~DCCanvas()
{
	hdc = NULL;
}

int DCCanvas::cloneDC(HDC clone, BaseWnd *srcWnd)
{
	ASSERT(clone != NULL);
	hdc = clone;
	srcwnd = srcWnd;
	return 1;
}

SysCanvas::SysCanvas()
{
	hdc = GetDC(NULL);
}

SysCanvas::~SysCanvas()
{
	ReleaseDC(NULL, hdc);
	hdc = NULL;
}

DCBltCanvas::DCBltCanvas()
{
	origdc = NULL;
	hbmp = prevbmp = NULL;
}

DCBltCanvas::~DCBltCanvas()
{

	commitDC();

	// kill the bitmap and its DC
	SelectObject(hdc, prevbmp);
	DeleteDC(hdc);
	hdc = NULL;
	DeleteObject(hbmp);

	// don't kill origdc, it's been cloned
}

int DCBltCanvas::setOrigDC(HDC neworigdc)
{
	// FG> allows custom draw on lists to be much faster
	origdc = neworigdc;
	return 1;
}

int DCBltCanvas::commitDC(void)
{
	//FG

	if (origdc)
	{

		RECT c;

		if (GetClipBox(origdc, &c) == NULLREGION)
			c = rect;

		// shlap it down in its original spot
		//GdiFlush();
		BitBlt(origdc, c.left, c.top,
		       c.right - c.left, c.bottom - c.top, hdc, c.left-rect.left, c.top-rect.top, SRCCOPY);

	}

	return 1;
}

int DCBltCanvas::cloneDC(HDC clone, RECT *r, BaseWnd *srcWnd)
{
	origdc = clone;

	srcwnd = srcWnd;

	ASSERT(r != NULL);
	rect = *r;

#if 1
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof bmi);
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = r->right - r->left;
	bmi.bmiHeader.biHeight = -ABS(r->bottom - r->top);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = 0;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	hbmp = CreateDIBSection(origdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	width = bmi.bmiHeader.biWidth;
	height = ABS(bmi.bmiHeader.biHeight);
	pitch = width * 4;
#else
hbmp = CreateCompatibleBitmap(clone, r->right - r->left, r->bottom - r->top);
#endif
	ASSERT(hbmp != NULL);

	// create tha DC
	hdc = CreateCompatibleDC(origdc);
	prevbmp = (HBITMAP)SelectObject(hdc, hbmp);

	// adjust their rect for them
	r->right -= r->left;
	r->left = 0;
	r->bottom -= r->top;
	r->top = 0;

	return 1;
}

DCExBltCanvas::DCExBltCanvas(HWND hWnd,        HRGN hrgnClip,    DWORD flags) : hwnd(hWnd)
{
	origdc = GetDCEx(hWnd, hrgnClip, flags);
	RECT r;
	GetWindowRect(hWnd, &r);
	OffsetRect(&r, -r.left, -r.top);
	cloneDC(origdc, &r);
}

DCExBltCanvas::~DCExBltCanvas()
{
	commitDC();
	ReleaseDC(hwnd, origdc);
	origdc=0;
}


BaseCloneCanvas::BaseCloneCanvas(ifc_canvas *cloner)
{
	if (cloner != NULL) clone(cloner);
}

int BaseCloneCanvas::clone(ifc_canvas *cloner)
{
	ASSERTPR(hdc == NULL, "can't clone twice");
	hdc = cloner->getHDC();
	bits = cloner->getBits();
	cloner->getDim(&width, &height, &pitch);
	//  srcwnd = cloner->getBaseWnd();
	cloner->getOffsets(&xoffset, &yoffset);

	canvasFontInfo.face =  cloner->getTextFont(); // just copies the pointer so be careful
	canvasFontInfo.pointSize = cloner->getTextSize();
	canvasFontInfo.bold = cloner->getTextBold();
	canvasFontInfo.opaque = !!cloner->getTextOpaque();
	canvasFontInfo.underline = !!cloner->getTextUnderline();
	canvasFontInfo.italic = !!cloner->getTextItalic();
	canvasFontInfo.alignFlags = cloner->getTextAlign();
	canvasFontInfo.color = cloner->getTextColor();
	canvasFontInfo.bgColor = cloner->getTextBkColor();

	return (hdc != NULL);
}

BaseCloneCanvas::~BaseCloneCanvas()
{
	hdc = NULL;
}

DDSurfaceCanvas::DDSurfaceCanvas(LPDIRECTDRAWSURFACE surface, int w, int h)
{
	surf = surface;
	_w = w;
	_h = h;
	hdc = NULL;
	bits = NULL;
}

DDSurfaceCanvas::~DDSurfaceCanvas()
{
	if (isready())
		exit();
}

int DDSurfaceCanvas::isready()
{
	return bits != NULL;
}

void DDSurfaceCanvas::enter()
{
	DDSURFACEDESC d = {sizeof(d), };
	if ((surf->Lock(NULL, &d, DDLOCK_WAIT, NULL)) != DD_OK)
		return ;

	surf->GetDC(&hdc);

	bits = d.lpSurface;
}

void DDSurfaceCanvas::exit()
{
	surf->ReleaseDC(hdc);
	surf->Unlock(bits);
	bits = NULL;
	hdc = NULL;
}


#endif//WIN32
