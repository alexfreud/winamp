#include <precomp.h>
#include <wasabicfg.h>
#include "paintset.h"

using namespace Paintset;
#include <tataki/canvas/bltcanvas.h>

#include <tataki/bitmap/autobitmap.h>
#include <tataki/color/skinclr.h>
#include <api/font/font.h>
#include <api/service/svcs/svc_font.h>

#ifndef WASABI_COMPILE_IMGLDR
#error This module requires image loading capabilities (WASABI_COMPILE_IMGLDR)
#endif

#define TITLE_PADDING 4	// SKINME

#ifdef WASABI_COMPILE_SKIN
static SkinColor title_fg(L"wasabi.component.title.foreground");
static SkinColor title_border(L"wasabi.component.title.border");
#endif

class NOVTABLE PaintSet
{
protected:
	PaintSet()
	{}
public:
	virtual ~PaintSet()
	{}

	virtual void render(ifc_canvas *canvas, const RECT *r, int alpha) = 0;
};

static PaintSet *sets[NUM_PAINTSETS];

class PaintSetButtonDisabled : public PaintSet
{
public:
	PaintSetButtonDisabled();

	virtual void render(ifc_canvas *canvas, const RECT *r, int alpha);

private:
	AutoSkinBitmap mid;
};

PaintSetButtonDisabled::PaintSetButtonDisabled()
{
	mid = L"wasabi.label.middle";
}

void PaintSetButtonDisabled::render(ifc_canvas *canvasbase, const RECT *_r, int alpha)
{
	BaseCloneCanvas canvas;
	int ret = canvas.clone(canvasbase);
	ASSERT(ret);

	RECT r = *_r;

	mid.stretchToRectAlpha(&canvas, &r, alpha);
}

//----

class PaintSetButtonUp : public PaintSet
{
public:
	PaintSetButtonUp();

	virtual void render(ifc_canvas *canvas, const RECT *r, int alpha);

protected:
	AutoSkinBitmap ul, u, ur;
	AutoSkinBitmap l, mid, right;
	AutoSkinBitmap ll, bot, lr;
};

PaintSetButtonUp::PaintSetButtonUp()
{
	ul = L"wasabi.button.top.left";
	u = L"wasabi.button.top";
	ur = L"wasabi.button.top.right";

	l = L"wasabi.button.left";
	mid = L"wasabi.button.middle";
	right = L"wasabi.button.right";

	ll = L"wasabi.button.bottom.left";
	bot = L"wasabi.button.bottom";
	lr = L"wasabi.button.bottom.right";
}

void PaintSetButtonUp::render(ifc_canvas *canvasbase, const RECT *_r, int alpha)
{
	BaseCloneCanvas canvas;
	int ret = canvas.clone(canvasbase);
	if (!ret) return;

	RECT r = *_r, br;

	// upper left
	br.left = r.left;
	br.top = r.top;
	br.right = r.left + ul.getWidth();
	br.bottom = r.top + ul.getHeight();
	ul.stretchToRectAlpha(&canvas, &br, alpha);

	// top
	br.left = br.right;
	br.right = r.right - ur.getWidth();
	u.stretchToRectAlpha(&canvas, &br, alpha);

	// upper right
	br.left = br.right;
	br.right = r.right;
	ur.stretchToRectAlpha(&canvas, &br, alpha);

	// left
	br.left = r.left;
	br.right = r.left + l.getWidth();
	br.top = r.top + ul.getHeight();
	br.bottom = r.bottom - ll.getHeight();
	l.stretchToRectAlpha(&canvas, &br, alpha);

	// middle
	br.top = r.top + ul.getHeight();
	br.bottom = r.bottom - ll.getHeight();
	br.left = r.left + ul.getWidth();
	br.right = r.right - ur.getWidth();
	mid.stretchToRectAlpha(&canvas, &br, alpha);

	// right
	br.left = r.right - right.getWidth();
	br.right = r.right;
	br.top = r.top + ur.getHeight();
	br.bottom = r.bottom - lr.getHeight();
	right.stretchToRectAlpha(&canvas, &br, alpha);

	// lower left
	br.left = r.left;
	br.right = r.left + ll.getWidth();
	br.top = r.bottom - ll.getHeight();
	br.bottom = r.bottom;
	ll.stretchToRectAlpha(&canvas, &br, alpha);

	// bot
	br.left = r.left + ll.getWidth();
	br.right = r.right - lr.getWidth();
	br.top = r.bottom - bot.getHeight();
	br.bottom = r.bottom;
	bot.stretchToRectAlpha(&canvas, &br, alpha);

	// lower right
	br.left = r.right - lr.getWidth();
	br.right = r.right;
	br.top = r.bottom - lr.getHeight();
	br.bottom = r.bottom;
	lr.stretchToRectAlpha(&canvas, &br, alpha);
}

class PaintSetLabel : public PaintSetButtonUp
{
public:
	PaintSetLabel()
	{
		ul = L"wasabi.label.top.left";
		u = L"wasabi.label.top";
		ur = L"wasabi.label.top.right";

		l = L"wasabi.label.left";
		mid = L"wasabi.label.middle";
		right = L"wasabi.label.right";

		ll = L"wasabi.label.bottom.left";
		bot = L"wasabi.label.bottom";
		lr = L"wasabi.label.bottom.right";
	}
};

//----

class PaintSetButtonDown : public PaintSet
{
public:
	PaintSetButtonDown();

	virtual void render(ifc_canvas *canvas, const RECT *r, int alpha);

private:
	AutoSkinBitmap ul, u, ur;
	AutoSkinBitmap l, mid, right;
	AutoSkinBitmap ll, bot, lr;
};

PaintSetButtonDown::PaintSetButtonDown()
{
	ul = L"wasabi.button.pressed.top.left";
	u = L"wasabi.button.pressed.top";
	ur = L"wasabi.button.pressed.top.right";

	l = L"wasabi.button.pressed.left";
	mid = L"wasabi.button.pressed.middle";
	right = L"wasabi.button.pressed.right";

	ll = L"wasabi.button.pressed.bottom.left";
	bot = L"wasabi.button.pressed.bottom";
	lr = L"wasabi.button.pressed.bottom.right";
}

void PaintSetButtonDown::render(ifc_canvas *canvasbase, const RECT *_r, int alpha)
{
	BaseCloneCanvas canvas;
	int ret = canvas.clone(canvasbase);
	ASSERT(ret);

	RECT r = *_r, br;

	// upper left
	br.left = r.left;
	br.top = r.top;
	br.right = r.left + ul.getWidth();
	br.bottom = r.top + ul.getHeight();
	ul.stretchToRectAlpha(&canvas, &br, alpha);

	// top
	br.left = br.right;
	br.right = r.right - ur.getWidth();
	u.stretchToRectAlpha(&canvas, &br, alpha);

	// upper right
	br.left = br.right;
	br.right = r.right;
	ur.stretchToRectAlpha(&canvas, &br, alpha);

	// left
	br.left = r.left;
	br.right = r.left + l.getWidth();
	br.top = r.top + ul.getHeight();
	br.bottom = r.bottom - ll.getHeight();
	l.stretchToRectAlpha(&canvas, &br, alpha);

	// middle
	br.top = r.top + ul.getHeight();
	br.bottom = r.bottom - ll.getHeight();
	br.left = r.left + ul.getWidth();
	br.right = r.right - ur.getWidth();
	mid.stretchToRectAlpha(&canvas, &br, alpha);

	// right
	br.left = r.right - right.getWidth();
	br.right = r.right;
	br.top = r.top + ur.getHeight();
	br.bottom = r.bottom - lr.getHeight();
	right.stretchToRectAlpha(&canvas, &br, alpha);

	// lower left
	br.left = r.left;
	br.right = r.left + ll.getWidth();
	br.top = r.bottom - ll.getHeight();
	br.bottom = r.bottom;
	ll.stretchToRectAlpha(&canvas, &br, alpha);

	// bot
	br.left = r.left + ll.getWidth();
	br.right = r.right - lr.getWidth();
	br.top = r.bottom - bot.getHeight();
	br.bottom = r.bottom;
	bot.stretchToRectAlpha(&canvas, &br, alpha);

	// lower right
	br.left = r.right - lr.getWidth();
	br.right = r.right;
	br.top = r.bottom - lr.getHeight();
	br.bottom = r.bottom;
	lr.stretchToRectAlpha(&canvas, &br, alpha);
}
//--

class PaintSetAppFrame : public PaintSet
{
public:
	PaintSetAppFrame();
	virtual ~PaintSetAppFrame();

	virtual void render(ifc_canvas *canvas, const RECT *r, int alpha);

private:
	SkinBitmap *ul, *up, *ur;
	SkinBitmap *left, *right;
	SkinBitmap *ll, *bot, *lr;
};

PaintSetAppFrame::PaintSetAppFrame()
{
	ul = new SkinBitmap(L"studio.border.upperLeft");
	up = new SkinBitmap(L"studio.border.top");
	ur = new SkinBitmap(L"studio.border.upperRight");

	left = new SkinBitmap(L"studio.border.left");
	right = new SkinBitmap(L"studio.border.right");

	ll = new SkinBitmap(L"studio.border.lowerLeft");
	bot = new SkinBitmap(L"studio.border.bottom");
	lr = new SkinBitmap(L"studio.border.lowerRight");
}

PaintSetAppFrame::~PaintSetAppFrame()
{
	delete ul; delete up; delete ur;
	delete left; delete right;
	delete ll; delete bot; delete lr;
}

void PaintSetAppFrame::render(ifc_canvas *canvasbase, const RECT *_r, int alpha)
{
	BaseCloneCanvas canvas;
	int ret = canvas.clone(canvasbase);
	ASSERT(ret);

	RECT r = *_r;
	RECT br;

	// upper left
	br = r;
	br.right = br.left + 2;
	br.bottom = br.top + 2;
	ul->stretchToRectAlpha(&canvas, &br, alpha);

	// top
	br = r;
	br.left += 2;
	br.right -= 2;
	br.bottom = br.top + 2;
	up->stretchToRectAlpha(&canvas, &br, alpha);

	// upper right
	br = r;
	br.left = br.right - 2;
	br.bottom = br.top + 2;
	ur->stretchToRectAlpha(&canvas, &br, alpha);

	// left
	br = r;
	br.right = br.left + 2;
	br.top += 2;
	br.bottom -= 2;
	left->stretchToRectAlpha(&canvas, &br, alpha);

	// right
	br = r;
	br.left = br.right - 2;
	br.top += 2;
	br.bottom -= 2;
	right->stretchToRectAlpha(&canvas, &br, alpha);

	// lower left
	br = r;
	br.right = br.left + 2;
	br.top = br.bottom - 2;
	ll->stretchToRectAlpha(&canvas, &br, alpha);

	// bottom
	br = r;
	br.left += 2;
	br.right -= 2;
	br.top = br.bottom - 2;
	bot->stretchToRectAlpha(&canvas, &br, alpha);

	// lower right
	br = r;
	br.left = br.right - 2;
	br.top = br.bottom - 2;
	lr->stretchToRectAlpha(&canvas, &br, alpha);
}

// -----

class PaintSetTitleStreak : public PaintSet
{
public:
	PaintSetTitleStreak();

	virtual void render(ifc_canvas *canvas, const RECT *r, int alpha);

private:
	AutoSkinBitmap title_left, title_middle, title_right;
};

PaintSetTitleStreak::PaintSetTitleStreak()
{
	title_left = L"wasabi.titlebar.left.active";
	title_middle = L"wasabi.titlebar.center.active";
	title_right = L"wasabi.titlebar.right.active";
}

void PaintSetTitleStreak::render(ifc_canvas *canvasbase, const RECT *_r, int alpha)
{
	BaseCloneCanvas canvas;
	int ret = canvas.clone(canvasbase);
	if (!ret) return;

	RECT r = *_r;

	RECT lr = r;
	lr.right = lr.left + title_left.getWidth();
	lr.top += ((lr.bottom - lr.top) - title_left.getHeight()) / 2;
	lr.bottom = lr.top + title_left.getHeight();
	if (lr.right <= lr.left) return;
	title_left.stretchToRectAlpha(&canvas, &lr, alpha);

	RECT rr = r;
	rr.left = rr.right - title_right.getWidth();
	rr.top += ((rr.bottom - rr.top) - title_left.getHeight()) / 2;
	rr.bottom = rr.top + title_left.getHeight();
	if (rr.right <= rr.left) return;
	title_right.stretchToRectAlpha(&canvas, &rr, alpha);

	RECT cr = r;
	cr.left = lr.right;
	cr.right = rr.left;
	cr.top += ((cr.bottom - cr.top) - title_left.getHeight()) / 2;
	cr.bottom = cr.top + title_left.getHeight();
	if (cr.right <= cr.left) return;
	title_middle.stretchToRectAlpha(&canvas, &cr, alpha);
}

class PaintSetFocusRect : public PaintSet
{
public:
	virtual void render(ifc_canvas *canvas, const RECT *r, int alpha)
	{
		BaseCloneCanvas c;
		c.clone(canvas);
		c.drawRect(r, 0, 0xFFFFFF, alpha);
	}
};

// -----

int paintset_present(int set)
{
	return paintset_renderPaintSet(set, NULL, NULL, 255, TRUE);
}

int paintset_renderPaintSet(int type, ifc_canvas *c, const RECT *r, int alpha, int checkonly)
{
	PaintSet *ret = NULL;
	switch (type)
	{
		case BUTTONUP:
			ret = sets[BUTTONUP];
			if (ret == NULL) ret = new PaintSetButtonUp();
			sets[BUTTONUP] = ret;
			break;
		case BUTTONDOWN:
			ret = sets[BUTTONDOWN];
			if (ret == NULL) ret = new PaintSetButtonDown();
			sets[BUTTONDOWN] = ret;
			break;
		case TRAY:
			ret = sets[BUTTONDOWN];
			if (ret == NULL) ret = new PaintSetButtonDown();
			sets[BUTTONDOWN] = ret;
			break;
		case APPFRAME:
			ret = sets[APPFRAME];
			if (ret == NULL) ret = new PaintSetAppFrame();
			sets[APPFRAME] = ret;
			break;
		case BUTTONDISABLED:
			ret = sets[BUTTONDISABLED];
			if (ret == NULL) ret = new PaintSetButtonDisabled();
			sets[BUTTONDISABLED] = ret;
			break;
		case TITLESTREAK:
			ret = sets[TITLESTREAK];
			if (ret == NULL) ret = new PaintSetTitleStreak();
			sets[TITLESTREAK] = ret;
			break;
		case LABEL:
			ret = sets[LABEL];
			if (ret == NULL) ret = new PaintSetLabel();
			sets[LABEL] = ret;
			break;
		case FOCUSRECT:
			ret = sets[FOCUSRECT];
			if (ret == NULL) ret = new PaintSetFocusRect();
			sets[FOCUSRECT] = ret;
			break;
	}

	if (ret != NULL)
	{
		if (!checkonly) ret->render(c, r, alpha);
		return TRUE;
	}
	return FALSE;
}

#ifdef WASABI_COMPILE_FONTS

#define VERTPAD 2
#define AAFACTOR 2
#define FONTNAME "studio.component.title"
#define FONTNAMEW L"studio.component.title"

void paintset_renderTitle(const wchar_t *title, ifc_canvas *canvasbase, const RECT *_r, int alpha, int dostreaks, int doborder)
{
	BaseCloneCanvas canvas;
	int ret = canvas.clone(canvasbase);
	if (!ret) return;

	RECT r = *_r;
	int w = r.right - r.left;
	int h = (r.bottom - r.top) - VERTPAD;

	svc_font *f = Font::requestSkinFont(FONTNAMEW);
	if (f->isBitmap() || !doborder)
	{
		Wasabi::FontInfo fontInfo;
		int tw, th;
		fontInfo.pointSize = h;
		fontInfo.face = FONTNAMEW;
		fontInfo.color = title_fg;
		canvas.getTextExtent(title, &tw, &th, &fontInfo);
		w -= (tw + TITLE_PADDING * 2);
		w /= 2;
		int x = r.left + w + TITLE_PADDING;
		int y = r.top + VERTPAD / 2;
		canvas.textOut(x, y, title, &fontInfo);
	}
	else
	{
		Wasabi::FontInfo fontInfo;
		int tw, th;
		fontInfo.pointSize = h*AAFACTOR;
		fontInfo.face = FONTNAMEW;
		canvas.getTextExtent(title, &tw, &th, &fontInfo);
		tw /= AAFACTOR;
		th /= AAFACTOR;
		tw += 2;
		th += 2;

		BltCanvas bc(tw*AAFACTOR, th*AAFACTOR + 1);
		bc.fillBits(RGB(0, 0, 0));	// not skinned
		fontInfo.opaque=false;
		fontInfo.color = title_border;		
		bc.textOut(-AAFACTOR, AAFACTOR / 2, title, &fontInfo);
		bc.textOut(AAFACTOR, AAFACTOR / 2, title, &fontInfo);
		bc.textOut(AAFACTOR / 2, -AAFACTOR, title, &fontInfo);
		bc.textOut(AAFACTOR / 2, AAFACTOR, title, &fontInfo);

		fontInfo.color = title_fg;
		bc.textOut(AAFACTOR / 2, AAFACTOR / 2, title, &fontInfo);

		// mask it
		bc.maskColor(RGB(0, 0, 0), RGB(0, 0, 0));
		BltCanvas tc(tw, th);
		bc.antiAliasTo(&tc, tw, th, AAFACTOR);


		//int cw = tw + TITLE_PADDING * 2;
		//  if (cw > w || th > h) return;
		w -= (tw + TITLE_PADDING * 2);
		w /= 2;

		int x = r.left + w + TITLE_PADDING;
		int y = r.top + VERTPAD / 2;
//FG>??    tc.vflip();
		tc.blitAlpha(&canvas, x, y, alpha);

		//SkinBitmap splef0(tc.getBitmap(), tc.getHDC(), TRUE, tc.getBits());
//    splef0.blitAlpha(&canvas, x, y, alpha);



	}

	if (dostreaks)
	{
		RECT pr = r;
		pr.right = pr.left + w;
		paintset_renderPaintSet(TITLESTREAK, &canvas, &pr, alpha);

		pr.right = r.right;
		pr.left = r.right - w;
		paintset_renderPaintSet(TITLESTREAK, &canvas, &pr, alpha);
	}
}

#endif

void paintset_reset()
{
	for (int i = 0; i < NUM_PAINTSETS; i++)
	{
		delete sets[i];
		sets[i] = NULL;
	}
}

