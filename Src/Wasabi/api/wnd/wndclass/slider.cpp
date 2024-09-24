#include <precomp.h>
#include "slider.h"

#include <tataki/canvas/canvas.h>
#include <api/wnd/notifmsg.h>
#include <api/wnd/PaintCanvas.h>

#define DEFAULT_THUMBWIDTH  16
#define DEFAULT_THUMBHEIGHT 16

SliderWnd::SliderWnd()
{
	seeking = 0;
	enabled = 1;
	hilite = 0;
	pos = 0;
	oldpos = -1;
	thumbwidth = DEFAULT_THUMBWIDTH;
	captured = 0;
	xShift = 0;
	yShift = 0;

	base_texture = NULL;
	use_base_texture = 0;
	no_default_background = 0;

	drawOnBorders = 0;
	hotPosition = -1;
	origPos = 0;

	vertical = 0;

	thumbCentered = 1;
	thumbOffset = 0;
	thumbStretched = 0;
	hotposrange = -1;
	setLimits(START, END);
}

SliderWnd::~SliderWnd()
{}


int SliderWnd::onPaint(Canvas *canvas)
{
	if (canvas == NULL)
	{
		PaintBltCanvas paintcanvas;
		if (!paintcanvas.beginPaint(this)) 
			return 0;
		SliderWnd::onPaint(&paintcanvas);
	}

	SLIDERWND_PARENT::onPaint(canvas);

	RECT r, origr;
	getClientRect(&r);
	origr = r;

	if (use_base_texture)
	{
		if (!base_texture)
		{
			renderBaseTexture(canvas, r);
		}
		else
		{
			RECT cr;
			cr.left = xShift;
			cr.top = yShift;
			cr.right = cr.left + (r.right - r.left);
			cr.bottom = cr.top + (r.bottom - r.top);
			base_texture->blitToRect(canvas, &cr, &r);
		}
	}

	if (vertical)
	{
		RECT br;
		br.left = r.left;
		br.right = r.right;

		if (left.getBitmap())
		{
			br.top = r.top;
			br.bottom = left.getHeight();
			left.getBitmap()->stretchToRectAlpha(canvas, &br, getPaintingAlpha());
		}

		if (right.getBitmap())
		{
			br.top = r.bottom - right.getHeight();
			br.bottom = r.bottom;
			right.stretchToRectAlpha(canvas, &br, getPaintingAlpha());
		}

		if (middle.getBitmap())
		{
			br.top = r.top + (left.getBitmap() ? left.getHeight() : 0);
			br.bottom = r.bottom - (right.getBitmap() ? right.getHeight() : 0);
			middle.getBitmap()->stretchToRectAlpha(canvas, &br, getPaintingAlpha());
		}
	}
	else
	{
		RECT br;
		br.top = r.top;
		br.bottom = r.bottom;

		if (left.getBitmap())
		{
			br.left = r.left;
			br.right = br.left + left.getWidth();
			left.getBitmap()->stretchToRectAlpha(canvas, &br, getPaintingAlpha());
		}

		if (right.getBitmap())
		{
			br.left = r.right - right.getWidth();
			br.right = r.right;
			right.getBitmap()->stretchToRectAlpha(canvas, &br, getPaintingAlpha());
		}

		if (middle.getBitmap())
		{
			br.left = r.left + (left.getBitmap() ? left.getWidth() : 0);
			br.right = r.right - (right.getBitmap() ? right.getWidth() : 0);
			middle.getBitmap()->stretchToRectAlpha(canvas, &br, getPaintingAlpha());
		}
	}

	if (vertical)
	{
		int w = (r.bottom - r.top) - thumbHeight();
		//    ASSERT(w > 0);  // if the control paints off the edge of the screen, this will needlessly assert.
		if (w < 0) w = 0;
		r.top += (pos * w) / length;
		r.bottom = r.top + thumbHeight();
		if (!thumbStretched)
		{
			if (!thumbCentered)
			{
				r.left = origr.left + thumbOffset;
				r.right = origr.left + thumbWidth() + thumbOffset;
			}
			else
			{
				int w = ((r.right - r.left) - thumbWidth()) / 2;
				r.left = origr.left + w + thumbOffset;
				r.right = origr.right - w + thumbOffset;
			}
		}
		else
		{
			r.left = origr.left;
			r.right = origr.right;
		}

	}
	else
	{
		// offset for left bitmap
		if (!drawOnBorders)
		{
			if (left.getBitmap() != NULL) r.left += left.getWidth();
			if (right.getBitmap() != NULL) r.right -= right.getWidth();
		}

		int w = (r.right - r.left) - thumbWidth();
		if (w < 0) w = 0;
		r.left += (pos * w) / length;
		r.right = r.left + thumbWidth();
		if (r.right > origr.right)
		{
			r.left -= r.right - origr.right;
			r.right = origr.right;
		}
		
		if (!thumbStretched)
		{
			int thumbh = thumb.getBitmap() ? thumb.getHeight() : DEFAULT_THUMBWIDTH;
			if (thumbCentered)
			{
				int h = ((r.bottom - r.top) - thumbh) / 2;
				r.top = origr.top + h;
				r.bottom = origr.bottom - h;
			}
			else
			{
				r.top = origr.top + thumbOffset;
				r.bottom = origr.top + thumbh + thumbOffset;
			}
		}
		else
		{
			r.top = origr.top;
			r.bottom = origr.bottom;
		}
	}

	SkinBitmap *sb = getSeekStatus() ? (thumbdown.getBitmap() ? thumbdown.getBitmap() : thumb.getBitmap()) : ((hilite && thumbhilite.getBitmap()) ? thumbhilite.getBitmap() : thumb.getBitmap());

	if (sb != NULL)
		sb->stretchToRectAlpha(canvas, &r, getPaintingAlpha());
	else
		canvas->fillRect(&r, RGB(255, 0, 0));

	return 1;
}

int SliderWnd::onInit()
{
	SLIDERWND_PARENT::onInit();
	if (!no_default_background)
	{
		if (vertical)
		{
			// Please note that these bitmaps here do not yet exist.
			if (left.getBitmapName() == NULL) setLeftBmp(L"wasabi.slider.vertical.top");
			if (middle.getBitmapName() == NULL) setMiddleBmp(L"wasabi.slider.vertical.middle");
			if (right.getBitmapName() == NULL) setRightBmp(L"wasabi.slider.vertical.bottom");
			if (thumb.getBitmapName() == NULL) setThumbBmp(L"wasabi.slider.vertical.button");
			if (thumbdown.getBitmapName() == NULL) setThumbDownBmp(L"wasabi.slider.vertical.button.pressed");
		}
		else
		{
			if (left.getBitmapName() == NULL) setLeftBmp(L"wasabi.slider.horizontal.left");
			if (middle.getBitmapName() == NULL) setMiddleBmp(L"wasabi.slider.horizontal.middle");
			if (right.getBitmapName() == NULL) setRightBmp(L"wasabi.slider.horizontal.right");
			if (thumb.getBitmapName() == NULL) setThumbBmp(L"wasabi.slider.horizontal.button");
			if (thumbdown.getBitmapName() == NULL) setThumbDownBmp(L"wasabi.slider.horizontal.button.pressed");
		}
	}


	return 1;
}

int SliderWnd::onLeftButtonDown(int x, int y)
{
	SLIDERWND_PARENT::onLeftButtonDown(x, y);
	if (!enabled) return 0;
	seeking = 1;

	origPos = 0;
	RECT r;
	getClientRect(&r);
	if (vertical)
	{
		int w = (r.bottom - r.top) - thumbHeight();
		if (w < 0) w = 0;
		r.top += (pos * w) / length;
		origPos = (y - r.top) - 1;
		/*if(origPos<0 || origPos>thumbHeight())*/ origPos =   (thumbHeight() /   2) -   2;
	}
	else
	{
		if (!drawOnBorders)
		{
			if (left.getBitmap() != NULL) r.left += left.getWidth();
			if (right.getBitmap() != NULL) r.right -= right.getWidth();
		}
		int w = (r.right - r.left) - thumbWidth();
		if (w < 0) w = 0;
		r.left += (pos * w) / length;
		origPos = (x - r.left) - 1;
		if (origPos < 0 || origPos > thumbWidth()) origPos = (thumbWidth() / 2) - 2;
	}

	if (!captured)
	{
		captured = 1;
		beginCapture();
	}
	oldpos = pos;
	onMouseMove(x, y);
	return 1;
}

//FG>
//removed cross-hierarchy deletion (crashs due to ancestor in common.dll trying to delete pointers in a different
//heap scope than the one in which they were allocated)
void SliderWnd::setBitmaps(const wchar_t *thumbbmp, const wchar_t *thumbdownbmp, const wchar_t *thumbhighbmp, const wchar_t *leftbmp, const wchar_t *middlebmp, const wchar_t *rightbmp)
{
	setThumbBmp(thumbbmp);
	setThumbDownBmp(thumbdownbmp);
	setThumbHiliteBmp(thumbhighbmp);
	setLeftBmp(leftbmp);
	setRightBmp(rightbmp);
	setMiddleBmp(middlebmp);
}

void SliderWnd::setLeftBmp(const wchar_t *name)
{
	left = name;
	invalidate();
}

void SliderWnd::setMiddleBmp(const wchar_t *name)
{
	middle = name;
	invalidate();
}

void SliderWnd::setRightBmp(const wchar_t *name)
{
	right = name;
	invalidate();
}

void SliderWnd::setThumbBmp(const wchar_t *name)
{
	thumb = name;
	invalidate();
}

void SliderWnd::setThumbDownBmp(const wchar_t *name)
{
	thumbdown = name;
	invalidate();
}

void SliderWnd::setThumbHiliteBmp(const wchar_t *name)
{
	thumbhilite = name;
	invalidate();
}

SkinBitmap *SliderWnd::getLeftBitmap()
{
	return left;
}

SkinBitmap *SliderWnd::getRightBitmap()
{
	return right;
}

SkinBitmap *SliderWnd::getMiddleBitmap()
{
	return middle;
}

SkinBitmap *SliderWnd::getThumbBitmap()
{
	return thumb;
}

SkinBitmap *SliderWnd::getThumbDownBitmap()
{
	return thumbdown;
}

SkinBitmap *SliderWnd::getThumbHiliteBitmap()
{
	return thumbhilite;
}

int SliderWnd::getWidth()
{
	if (vertical)
		return (getThumbBitmap() ? getThumbBitmap()->getWidth() : 0);
	else
	{
		return 64;
	}
}

int SliderWnd::getHeight()
{
	if (!vertical)
		return (getThumbBitmap() ? getThumbBitmap()->getHeight() : 0);
	else
	{
		return 64;
	}
}

void SliderWnd::setEnable(int en)
{
	if (enabled != en) invalidate();
	enabled = en;
}

int SliderWnd::getEnable(void)
{
	return enabled;
}

void SliderWnd::setPosition(int newpos, int wantcb)
{
	if (newpos < minlimit) newpos = minlimit;
	else if (newpos > maxlimit) newpos = maxlimit;

	if (vertical) pos = maxlimit - newpos;
	else /* horizontal */ pos = newpos - minlimit;

	if (wantcb)
		onSetPosition();

	invalidate();
}

int SliderWnd::onMouseMove(int x, int y)
{
	int p, w, mouseover;

	SLIDERWND_PARENT::onMouseMove(x, y);

	POINT po = {x, y};
	clientToScreen(&po);
	mouseover = (WASABI_API_WND->rootWndFromPoint(&po) == this);
	if (mouseover && !seeking && !captured)
	{
		beginCapture();
		captured = 1;
		onEnterArea();
	}
	int lasthilite = hilite;
	hilite = enabled && mouseover;
	if (hilite != lasthilite)
	{
		if (!mouseover && !seeking && captured)
		{
			endCapture();
			captured = 0;
			onLeaveArea();
			invalidate();
			return 0;
		}
		invalidate();
	}

	if (!enabled) return 1;

	RECT r, origr;
	getClientRect(&r);
	x -= r.left;
	y -= r.top;

	origr = r;
	if (vertical)
	{
		w = (r.bottom - r.top) - thumbHeight();
		//    p = (y - (r.top-origr.top)) - (thumbHeight()/2-2);
		p = (y - (r.top - origr.top)) - origPos;
	}
	else
	{
		if (!drawOnBorders)
		{
			if (left != NULL) r.left += left.getWidth();
			if (right != NULL) r.right -= right.getWidth();
		}
		w = (r.right - r.left) - thumbWidth();
		//    p = (x - (r.left - origr.left)) - (thumbWidth()/2-2);
		p = (x - (r.left - origr.left)) - origPos;
	}

	if (seeking)
	{
		pos = (p * length) / w;
		if (pos < 0) pos = 0;
		else if (pos > length) pos = length;

		if (hotPosition != -1)
		{
			int a, c;
			if (vertical) a = r.bottom - r.top;
			else a = r.right - r.left;
			c = getHotPosRange();
			if (c == -1)
			{
				int b = (int)(a * 0.075);
				c = (b * length) / a;
			}

			/**
				EQBand: minlimit -127, maxlimit 127, hotpos 0
				PanBar: minlimit 0, maxlimit 225, hotpos 127

				VSliders pos starts from top by 0 (winamp behaviour reversed!)
			*/

			if (vertical)
			{
				//if (pos > (hotPosition - c) && pos < (hotPosition + c)) pos = hotPosition;
				if ((maxlimit - pos) > (hotPosition - c) && (maxlimit - pos) < (hotPosition + c)) pos = hotPosition - minlimit; // Hehe, now it works ;)
			}
			else
			{
				if (pos > (hotPosition - c) && pos < (hotPosition + c)) pos = hotPosition;
				//if ((pos - maxlimit)> (hotPosition - c) && (pos - maxlimit) < (hotPosition + c)) pos = hotPosition - minlimit;
			}
		}

		onSetPosition();
		invalidate();
	}

	return 1;
}

void SliderWnd::onCancelCapture()
{
	SLIDERWND_PARENT::onCancelCapture();
	if (seeking && captured)
		abort();
}

int SliderWnd::onLeftButtonUp(int x, int y)
{
	SLIDERWND_PARENT::onLeftButtonUp(x, y);
	int wasseeking = seeking;
	seeking = 0;
	captured = 0;
	oldpos = -1;
	endCapture();
	if (wasseeking)
		onSetFinalPosition();
	invalidate();
	return 1;
}

int SliderWnd::onRightButtonDown(int x, int y)
{
	SLIDERWND_PARENT::onRightButtonDown(x, y);
	if (seeking && captured)
	{
		abort();
	}
	return 1;
}

int SliderWnd::onChar(unsigned int c)
{
	SLIDERWND_PARENT::onChar(c);
	if (seeking && captured && (c == 27))
	{
		abort();
	}
	return 1;
}

int SliderWnd::onSetPosition()
{
	if (!isInited()) return 0;
	notifyParent(ChildNotify::SLIDER_INTERIM_POSITION, getSliderPosition());
	return 0;
}

int SliderWnd::onSetFinalPosition()
{
	if (!isInited()) return 0;
	notifyParent(ChildNotify::SLIDER_FINAL_POSITION, getSliderPosition());
	return 0;
}

int SliderWnd::getSliderPosition()
{
	if (vertical) return maxlimit -pos;
	else return pos + minlimit;
}

int SliderWnd::getSeekStatus()
{
	return seeking;
}

int SliderWnd::thumbWidth()
{
	if (thumb.getBitmap() == NULL) return DEFAULT_THUMBWIDTH;
	return thumb.getWidth();
}

int SliderWnd::thumbHeight()
{
	if (thumb.getBitmap() == NULL) return DEFAULT_THUMBHEIGHT;
	return thumb.getHeight();
}

void SliderWnd::setUseBaseTexture(int useit)
{
	use_base_texture = useit;
	invalidate();
}

void SliderWnd::setBaseTexture(SkinBitmap *bmp, int x, int y)
{
	base_texture = bmp;
	use_base_texture = TRUE;
	xShift = x;
	yShift = y;
	invalidate();
}

void SliderWnd::setNoDefaultBackground(int no)
{
	no_default_background = no;
}

void SliderWnd::setDrawOnBorders(int draw)
{
	drawOnBorders = draw;
}

void SliderWnd::onEnterArea()
{
	SLIDERWND_PARENT::onEnterArea();
}

void SliderWnd::onLeaveArea()
{
	SLIDERWND_PARENT::onLeaveArea();
}

void SliderWnd::setOrientation(int o)
{
	vertical = o;
}

void SliderWnd::setHotPosition(int h)
{
	hotPosition = h;
}

void SliderWnd::setThumbCentered(int c)
{
	thumbCentered = c;
}

void SliderWnd::setThumbStretched(int c)
{
	thumbStretched = c;
}


void SliderWnd::setThumbOffset(int o)
{
	thumbOffset = o;
}

void SliderWnd::abort()
{
	if (oldpos != -1)
	{
		seeking = 0;
		captured = 0;
		endCapture();
		pos = oldpos;
		onSetPosition();
		invalidate();
		oldpos = -1;
	}
	return ;
}

void SliderWnd::setLimits(int pminlimit, int pmaxlimit)
{
	minlimit = pminlimit;
	maxlimit = pmaxlimit;
	length = maxlimit - minlimit;
}

int SliderWnd::onKeyDown(int vkcode)
{
	switch (vkcode)
	{
	case VK_LEFT: move_left(Std::keyModifier(STDKEY_CONTROL));	return 1;
	case VK_RIGHT: move_right(Std::keyModifier(STDKEY_CONTROL));	return 1;
	case VK_HOME: move_start();	return 1;
	case VK_END: move_end();	return 1;
	default: return SLIDERWND_PARENT::onKeyDown(vkcode);
	}
}

void SliderWnd::move_left(int bigstep)
{
	int pos = getSliderPosition();
	if (!bigstep) pos--; else pos -= (ABS(maxlimit - minlimit) / 10);
	if (pos < minlimit) pos = minlimit;
	setPosition(pos);
}

void SliderWnd::move_right(int bigstep)
{
	int pos = getSliderPosition();
	if (!bigstep)
		pos++;
	else
		pos += (ABS(maxlimit - minlimit) / 10);
	if (pos > maxlimit)
		pos = maxlimit;
	setPosition(pos);
}

void SliderWnd::move_start()
{
	setPosition(minlimit);
}

void SliderWnd::move_end()
{
	setPosition(maxlimit);
}
