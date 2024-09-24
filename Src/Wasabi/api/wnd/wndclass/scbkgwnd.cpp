#include <precomp.h>
#include "scbkgwnd.h"
#include <api/wnd/notifmsg.h>
#include <bfc/wasabi_std_wnd.h>
#include <api/wnd/PaintCanvas.h>

#define SCROLLBAR_SEP       4
#define TIMER_SMOOTHSCROLLY 8873
#define TIMER_SMOOTHSCROLLX 8874
#define SMOOTH_STEPS        5
#define DEFAULT_BGCOLOR     RGB(0,0,0)

ScrlBkgWnd::ScrlBkgWnd()
{
	inDestroy = FALSE;
	bmp = NULL;
	bgColor = DEFAULT_BGCOLOR;
	scrollX = 0;
	scrollY = 0;
	dbbuffer = 1;
	needSetSliders = FALSE;
	lineHeight = 16;
	wantsep = 0;
	wantTileBg = true;
	lastratio = 1.0;
	MEMSET(&smsqr, 0, sizeof(RECT));
	in_set_slider_position = 0;

	smoothScrollYInc = smoothScrollXInc = 0;
	smoothScrollYCur = smoothScrollXCur = 0;
	smoothScrollYTimerCount = smoothScrollXTimerCount = 0;
	smoothYTimer = smoothXTimer = 0;
}

ScrlBkgWnd::~ScrlBkgWnd()
{
	inDestroy = TRUE;
}

int ScrlBkgWnd::onInit()
{

	SCRLBKGWND_PARENT::onInit();

	scrollY = 0;
	scrollX = 0;

	hSep.setOrientation(SEP_HORIZONTAL);

	hScroll.setBitmaps(L"wasabi.scrollbar.horizontal.left",
	                    L"wasabi.scrollbar.horizontal.left.pressed",
	                    L"wasabi.scrollbar.horizontal.left.hover",
	                    L"wasabi.scrollbar.horizontal.right",
	                    L"wasabi.scrollbar.horizontal.right.pressed",
	                    L"wasabi.scrollbar.horizontal.right.hover",
	                    L"wasabi.scrollbar.horizontal.button",
	                    L"wasabi.scrollbar.horizontal.button.pressed",
	                    L"wasabi.scrollbar.horizontal.button.hover");

	hScroll.setBackgroundBitmaps(L"wasabi.scrollbar.horizontal.background.left",
	                              L"wasabi.scrollbar.horizontal.background.middle",
	                              L"wasabi.scrollbar.horizontal.background.right");

	vSep.setOrientation(SEP_VERTICAL);

	vScroll.setBitmaps(L"wasabi.scrollbar.vertical.left",
	                    L"wasabi.scrollbar.vertical.left.pressed",
	                    L"wasabi.scrollbar.vertical.left.hover",
	                    L"wasabi.scrollbar.vertical.right",
	                    L"wasabi.scrollbar.vertical.right.pressed",
	                    L"wasabi.scrollbar.vertical.right.hover",
	                    L"wasabi.scrollbar.vertical.button",
	                    L"wasabi.scrollbar.vertical.button.pressed",
	                    L"wasabi.scrollbar.vertical.button.hover");

	vScroll.setBackgroundBitmaps(L"wasabi.scrollbar.vertical.background.top",
	                              L"wasabi.scrollbar.vertical.background.middle",
	                              L"wasabi.scrollbar.vertical.background.bottom");

	//	hScroll->setVertical(FALSE);
	vScroll.setVertical(TRUE);

	hScroll.setStartHidden(TRUE);  // prevent showing window at creation
	vScroll.setStartHidden(TRUE);
	hSep.setStartHidden(TRUE);
	vSep.setStartHidden(TRUE);

	hScroll.setParent(this);
	vScroll.setParent(this);
	hSep.setParent(this);
	vSep.setParent(this);

	hScroll.init(getOsModuleHandle(), getOsWindowHandle());
	vScroll.init(getOsModuleHandle(), getOsWindowHandle());
	hSep.init(getOsModuleHandle(), getOsWindowHandle());
	vSep.init(getOsModuleHandle(), getOsWindowHandle());

	hScroll.setPosition(0);
	vScroll.setPosition(0);

	setSlidersPosition();           // position sliders and show them if needed

	return 1;
}

void ScrlBkgWnd::setBgBitmap(const wchar_t *b)
{
	bmp = b;
	if (b) setBgColor(DEFAULT_BGCOLOR);
}

void ScrlBkgWnd::setBgColor(ARGB32 rgb)
{
	bgColor = rgb;
}

SkinBitmap *ScrlBkgWnd::getBgBitmap(void)
{
	return bmp;
}

ARGB32 ScrlBkgWnd::getBgColor(void)
{
	return bgColor;
}

// Scroll to a specified Y-pixels
void ScrlBkgWnd::scrollToY(int y, int signal)
{

	WndCanvas *canvas = NULL;
	RECT r;
	int offset;
	int dor2 = 0;
	RECT r2 = {0, 0, 0, 0};
	int focused = gotFocus();

	if (isVirtual() || renderRatioActive())
	{
		scrollY = y;
		invalidateRect(&clientRect());
		onScrollY(y);
		return ;
	}

	if (!Wasabi::Std::Wnd::isValidWnd(getOsWindowHandle())) return ;	// no need to paint

	if (y > scrollY)
	{ // tree scrolling up, scroller going down. invalidating from the bottom. bitblting from bottom to top
		int lines = y - scrollY;
		offset = -lines;
		getClientRect(&r);
		canvas = new WndCanvas();
		canvas->attachToClient(this);

		RegionI reg;
		makeWindowOverlayMask(&reg);
		RegionI clip(&r);
		reg.offset(0, offset);
		clip.subtractRegion(&reg);
		canvas->selectClipRgn(&clip);

	  int b = hScroll.isVisible() ? hScroll.getHeight() : 0;
 	  int c = vScroll.isVisible() ? vScroll.getWidth() : 0;
 	  int a = focused && (!b);
 	  if (r.bottom-r.top-lines > 0)
 	    canvas->blit(r.left, r.top+lines, canvas, r.left, r.top, r.right-r.left- c, r.bottom-r.top-lines-a - b);

//		int a = focused && (!hScroll->isVisible());
		//if (r.bottom - r.top - lines > 0)
//			canvas->blit(r.left, r.top + lines, canvas, r.left, r.top, r.right - r.left, r.bottom - r.top - lines - a);

		canvas->selectClipRgn(NULL);
		if (!clip.isEmpty())
			invalidateRgn(&clip);


		getClientRect(&r2);
		r2.bottom = r2.top + 1;
		dor2 = 1;
		r.top = r.bottom - lines - 1;
	}
	if (y < scrollY)
	{ // tree scrolling down, scroller going up. invalidating from the top. bitblting from top to bottom
		int lines = scrollY - y;
		offset = lines;
		getClientRect(&r);
		canvas = new WndCanvas();
		canvas->attachToClient(this);

		RegionI reg;
		makeWindowOverlayMask(&reg);
		RegionI clip(&r);
		reg.offset(0, offset);
		clip.subtractRegion(&reg);
		canvas->selectClipRgn(&clip);

		    int c = vScroll.isVisible() ? vScroll.getWidth() : 0;
 	    canvas->blit(r.left, r.top+focused, canvas, r.left, r.top+lines+focused, r.right-r.left-c, r.bottom-r.top-lines-focused);
		//canvas->blit(r.left, r.top + focused, canvas, r.left, r.top + lines + focused, r.right - r.left, r.bottom - r.top - lines - focused);

		canvas->selectClipRgn(NULL);
		if (!clip.isEmpty())
			invalidateRgn(&clip);

		getClientRect(&r2);
		r2.top = r2.bottom - 1;
		dor2 = 1;
		r.bottom = r.top + lines + 1;
	}
	if (canvas)
	{
		delete canvas;
		scrollY = y;

		// in case we have a virtualCanvas, we need to tell BaseWnd to call us to paint on it next time it's needed coz we blited directly to the screen
		RECT cr;
		getClientRect(&cr);
		cr.top -= getHeaderHeight();
		RECT screenblit;
		SubtractRect(&screenblit, &cr, &r);

		// invalidate what's needed
		if (dor2 && focused)
			cascadeRepaintRect(&r2, 0);
		cascadeRepaintRect(&r);

		deferedInvalidateRect(&screenblit);

		//dbbuffer = 1;
		//repaint();
	}

	if (signal)
		updateVScroll(y);

	onScrollY(y);
}

// Scroll to a specified X-pixel
void ScrlBkgWnd::scrollToX(int x, int signal)
{
	WndCanvas *canvas = NULL;
	RECT r;
	int offset;
	int dor2 = 0;
	RECT r2 = {0, 0, 0, 0};
	int focused = gotFocus();

	if (isVirtual() || ABS(getRenderRatio() - 1.0) > 0.01)
	{
		scrollX = x;
		getClientRect(&r);
		invalidateRect(&r);
		return ;
	}

	if (x > scrollX)
	{ // tree scrolling left, scroller going right. invalidating from the right. bitblting from right to left
		int lines = x - scrollX;
		offset = -lines;
		getClientRect(&r);
		r.top -= getHeaderHeight();
		canvas = new WndCanvas();
		canvas->attachToClient(this);

		RegionI reg;
		makeWindowOverlayMask(&reg);
		RegionI clip(&r);
		reg.offset(offset, 0);
		clip.subtractRegion(&reg);
		canvas->selectClipRgn(&clip);

			    int c = vScroll.isVisible() ? vScroll.getWidth() : 0;
 	    canvas->blit(r.left+lines, r.top, canvas, r.left, r.top, r.right-r.left-lines-focused-c, r.bottom-r.top);
		//canvas->blit(r.left + lines, r.top, canvas, r.left, r.top, r.right - r.left - lines - focused, r.bottom - r.top);

		canvas->selectClipRgn(NULL);
		if (!reg.isEmpty())
			invalidateRgn(&reg);

		getClientRect(&r2);
		r2.right = r2.left + 1;
		dor2 = 1;
		r.left = r.right - lines - 1;
	}
	if (x < scrollX)
	{ // tree scrolling right, scroller going left. invalidating from the left. bitblting from left to right
		int lines = scrollX - x;
		offset = lines;
		getClientRect(&r);
		r.top -= getHeaderHeight();
		canvas = new WndCanvas();
		canvas->attachToClient(this);

		RegionI reg;
		makeWindowOverlayMask(&reg);
		RegionI clip(&r);
		reg.offset(offset, 0);
		clip.subtractRegion(&reg);
		canvas->selectClipRgn(&clip);

		    int a = focused && (!vScroll.isVisible());
 	    int c = hScroll.isVisible() ? hScroll.getHeight()-focused : 0;
 	    canvas->blit(r.left+a, r.top, canvas, r.left+lines, r.top, r.right-r.left-lines-a, r.bottom-r.top-c);

		//int a = focused && (!vScroll->isVisible());
//		canvas->blit(r.left + a, r.top, canvas, r.left + lines, r.top, r.right - r.left - lines - a, r.bottom - r.top);

		canvas->selectClipRgn(NULL);
		if (!reg.isEmpty())
			invalidateRgn(&reg);

		getClientRect(&r2);
		r2.left = r2.right - 1;
		dor2 = 1;
		r.right = r.left + lines + 1;
	}
	if (canvas)
	{
		delete canvas;
		scrollX = x;

		// in case we have a virtualCanvas, we need to tell BaseWnd to call us to paint on it next time it's needed coz we blited directly to the screen
		RECT cr;
		getClientRect(&cr);
		cr.top -= getHeaderHeight();
		RECT screenblit;
		SubtractRect(&screenblit, &cr, &r);
		deferedInvalidateRect(&screenblit);

		if (dor2 && focused)
			cascadeRepaintRect(&r2, 0);
		// invalidate what's needed
		cascadeRepaintRect(&r);

		//dbbuffer = 1;
		//repaint();
	}

	if (signal)
		updateHScroll(x);
}

void ScrlBkgWnd::setSlidersPosition()
{
	if (in_set_slider_position) return ;
	in_set_slider_position = 1;
	_setSlidersPosition();
	in_set_slider_position = 0;
}

void ScrlBkgWnd::_setSlidersPosition()
{

	if (!isInited()) return ;

	RECT d;
	getClientRect(&d);
	if ((d.left >= d.right) || (d.top >= d.bottom))
		return ;

	RECT r;
	if (inDestroy) return ;
	if (!isVisible())
	{
		needSetSliders = TRUE;
		return ;
	}

	needSetSliders = FALSE;

	if (needHScroll())
	{
		SCRLBKGWND_PARENT::getClientRect(&r);
		r.top = r.bottom - getScrollbarWidth();
		if (needVScroll())
			r.right -= getScrollbarWidth() + (wantsep ? SCROLLBAR_SEP : 0);
		RECT z; hScroll.getClientRect(&z);
		if (!Wasabi::Std::rectEqual(r, z))
		{ // assumes ScrollBars are virtual
			hScroll.resizeToRect(&r);
			RECT s = r;
			s.bottom = s.top;
			s.top -= (wantsep ? SCROLLBAR_SEP : 0);
			hSep.resizeToRect(&s);
		}
		if (!hScroll.isVisible())
		{
			hScroll.setVisible(TRUE);
			if (wantsep) hSep.setVisible(TRUE);
			onHScrollToggle(1);
		}
		hScroll.setNPages(((int)(getContentsWidth() / (r.right - r.left))) + 1);
		hScroll.setUpDownValue((int)(((float)lineHeight / (getContentsWidth() - (r.right - r.left)))*SCROLLBAR_FULL));
		hScroll.setPosition((int)((float)scrollX / getMaxScrollX() * SCROLLBAR_FULL));
	}
	else
	{
		if (hScroll.isVisible())
		{
			hScroll.setVisible(FALSE);
			if (wantsep) hSep.setVisible(FALSE);
			onHScrollToggle(0);
		}
		hScroll.setPosition(0);
		scrollToX(0);
	}

	if (needVScroll())
	{
		SCRLBKGWND_PARENT::getClientRect(&r);
		r.left = r.right - getScrollbarWidth();
		if (needHScroll())
			r.bottom -= getScrollbarWidth();
		RECT z; vScroll.getNonClientRect(&z);
		if (!Wasabi::Std::rectEqual(r, z))
		{
			vScroll.resizeToRect(&r);
			RECT s = r;
			s.right = s.left;
			s.left -= (wantsep ? SCROLLBAR_SEP : 0);
			vSep.resizeToRect(&s);
		}
		if (!vScroll.isVisible())
		{
			vScroll.setVisible(TRUE);
			if (wantsep) vSep.setVisible(TRUE);
			onVScrollToggle(1);
		}
		vScroll.setNPages(((int)(getContentsHeight() / (r.bottom - r.top))) + 1);
		vScroll.setUpDownValue((int)(((float)lineHeight / (getContentsHeight() - (r.bottom - r.top)))*SCROLLBAR_FULL));
		vScroll.setPosition((int)((float)scrollY / getMaxScrollY() * SCROLLBAR_FULL));
	}
	else
	{
		if (vScroll.isVisible())
		{
			vScroll.setVisible(FALSE);
			if (wantsep) vSep.setVisible(FALSE);
			onVScrollToggle(0);
		}
		vScroll.setPosition(0);
		scrollToY(0);
	}

	hSep.invalidate();
	vSep.invalidate();

	if (needHScroll() && needVScroll())
	{
		getNonClientRect(&smsqr);
		smsqr.left = smsqr.right - getScrollbarWidth();
		smsqr.top = smsqr.bottom - getScrollbarWidth();
		invalidateRect(&smsqr);
	}
	else
		ZERO(smsqr);
}

void ScrlBkgWnd::onHScrollToggle(int set)
{}

void ScrlBkgWnd::onVScrollToggle(int set)
{}

int ScrlBkgWnd::onPaint(Canvas *canvas)
{
	RECT d;
	getClientRect(&d);
	if (d.right > d.left + 0xFFFF || d.bottom > d.top + 0xFFFF) return 1;
	if ((d.left >= d.right) || (d.top >= d.bottom))
	{
		return SCRLBKGWND_PARENT::onPaint(canvas);
	}

	if (needSetSliders) setSlidersPosition();

	//  RECT z;
	//  GetUpdateRect(gethWnd(), &z, FALSE);

	PaintCanvas paintcanvas;
	PaintBltCanvas paintbcanvas;

	if (canvas == NULL)
	{
		if (dbbuffer)
		{
			if (!paintbcanvas.beginPaintNC(this)) return 0;
			canvas = &paintbcanvas;
		}
		else
		{
			if (!paintcanvas.beginPaint(this)) return 0;
			canvas = &paintcanvas;
		}
	}
	//dbbuffer=1;
	SCRLBKGWND_PARENT::onPaint(canvas);

	RegionI *smsq = NULL;

	if (needHScroll() && needVScroll())
	{
		renderBaseTexture(canvas, smsqr);
		smsq = new RegionI(&smsqr);
	}

	RECT r;
	LabelWnd::getNonClientRect(&r);
	RECT c = {r.left, r.top, r.right, r.top + getLabelHeight()}; // create label rect


	RegionI *clip = new RegionI();
	if (canvas->getClipRgn(clip) == 0)
	{
		delete clip;
		clip = new RegionI(&r);
		if (smsq) clip->subtractRegion(smsq);
		canvas->selectClipRgn(clip);
	}
	else
	{
		RegionI reg(&c);
		clip->subtractRegion(&reg);
		if (smsq) clip->subtractRegion(smsq);
		canvas->selectClipRgn(clip);
	}
	delete smsq;

	drawBackground(canvas);
	delete clip;

if (getRenderRatio() != lastratio) { invalidate(); lastratio = getRenderRatio(); } // todo: make that an event
	return 1;
}

int ScrlBkgWnd::needDoubleBuffer()
{
	return dbbuffer;
}

int ScrlBkgWnd::onEraseBkgnd(HDC dc)
{

	/*	DCCanvas canvas;
		canvas.cloneDC(dc);
	 
		drawBackground(&canvas);*/

	return 1;
}

// Draws tiled background
void ScrlBkgWnd::drawBackground(Canvas *canvas)
{
	RECT r(clientRect());
	RegionI reg(&r);
	RegionI old;
	canvas->getClipRgn(&old);
	reg.andRegion(&old);
	canvas->selectClipRgn(&reg);
	if (bmp.getBitmap() && bgColor == DEFAULT_BGCOLOR)
	{
		r.top -= scrollY % bmp.getBitmap()->getHeight();
		r.left -= scrollX % bmp.getBitmap()->getWidth();
		if (wantTileBg)
			bmp.getBitmap()->blitTile(canvas, &r);
		else
			bmp.getBitmap()->stretchToRect(canvas, &r);
	}
	else if (bgColor != DEFAULT_BGCOLOR)
	{
		canvas->fillRect(&r, bgColor);
	}
	canvas->selectClipRgn(&old);

}

bool ScrlBkgWnd::needHScroll()
{
	if (!wantHScroll()) return FALSE;
	RECT r;
	getNonClientRect(&r);
	if (vScroll.isVisible())
		r.right -= getScrollbarWidth();
	return (getContentsWidth() > r.right - r.left);
}

bool ScrlBkgWnd::needVScroll()
{
	if (!wantVScroll()) return FALSE;
	RECT r;
	getNonClientRect(&r);
	r.top += getHeaderHeight();
	if (hScroll.isVisible())
		r.bottom -= getScrollbarWidth();
	return (getContentsHeight() > r.bottom - r.top);
}

// Returns the current tree width in pixels
int ScrlBkgWnd::getContentsWidth()
{
	/*RECT r;
	ScrlBkgWnd::getClientRect(&r);
	return r.right-r.left;*/ 
	return 10000;
}

// Returns the current tree height in pixels
int ScrlBkgWnd::getContentsHeight()
{
	/*RECT r;
	ScrlBkgWnd::getClientRect(&r);
	return r.bottom-r.top;*/ 
	return 10000;
}

int ScrlBkgWnd::getMaxScrollY()
{
	RECT r;
	getClientRect(&r);
	return MAX<int>(0, getContentsHeight() - (r.bottom - r.top));
}

int ScrlBkgWnd::getMaxScrollX()
{
	RECT r;
	getClientRect(&r);
	return MAX<int>(0, getContentsWidth() - (r.right - r.left));
}

void ScrlBkgWnd::updateVScroll(int y)
{
	if (getMaxScrollY() == 0) { vScroll.setPosition(0); return ; }
	int z = (int)((float)y / getMaxScrollY() * SCROLLBAR_FULL);
	vScroll.setPosition(z);
}

void ScrlBkgWnd::updateHScroll(int x)
{
	if (getMaxScrollX() == 0) { hScroll.setPosition(0); return ; }
	int z = (int)((float)x / getMaxScrollX() * SCROLLBAR_FULL);
	hScroll.setPosition(z);
}

void ScrlBkgWnd::updateScrollY(bool smooth)
{
	if (getMaxScrollY() == 0) { scrollToY(0); return ; }
	int y = (int)((float)(vScroll.getPosition()) / SCROLLBAR_FULL * getMaxScrollY());
	if (!smooth)
		scrollToY(y /*& ~3*/);
	else
		smoothScrollToY(y);
}

void ScrlBkgWnd::updateScrollX(bool smooth)
{
	if (getMaxScrollX() == 0) { scrollToX(0); return ; }
	int x = (int)((float)(hScroll.getPosition()) / SCROLLBAR_FULL * getMaxScrollX());
	if (!smooth)
		scrollToX(x /*& ~3*/);
	else
		smoothScrollToX(x);
}

void ScrlBkgWnd::smoothScrollToX(int x)
{
	killSmoothXTimer();
	smoothScrollXInc = -(float)(scrollX - x) / SMOOTH_STEPS;
	smoothScrollXCur = (float)scrollX;
	smoothScrollXTimerCount = 0;
	smoothXTimer = 1;
	setTimer(TIMER_SMOOTHSCROLLX, 25);
}

void ScrlBkgWnd::killSmoothYTimer()
{
	if (smoothYTimer)
	{
		killTimer(TIMER_SMOOTHSCROLLY);
		smoothScrollYCur += smoothScrollYInc * (SMOOTH_STEPS - smoothScrollYTimerCount);
		scrollToY((int)smoothScrollYCur);
		smoothYTimer = 0;
		updateVScroll(scrollY);
	}
}

void ScrlBkgWnd::killSmoothXTimer()
{
	if (smoothXTimer)
	{
		killTimer(TIMER_SMOOTHSCROLLX);
		smoothScrollXCur += smoothScrollXInc * (SMOOTH_STEPS - smoothScrollXTimerCount);
		scrollToX((int)smoothScrollXCur);
		smoothXTimer = 0;
		updateHScroll(scrollX);
	}
}

void ScrlBkgWnd::smoothScrollToY(int y)
{
	killSmoothYTimer();
	smoothScrollYInc = -(float)(scrollY - y) / SMOOTH_STEPS;
	smoothScrollYCur = (float)scrollY;
	smoothScrollYTimerCount = 0;
	smoothYTimer = 1;
	setTimer(TIMER_SMOOTHSCROLLY, 25);
}

void ScrlBkgWnd::timerCallback(int id)
{
	switch (id)
	{
	case TIMER_SMOOTHSCROLLY:
		smoothScrollYCur += smoothScrollYInc;
		scrollToY((int)smoothScrollYCur, FALSE);
		if (++smoothScrollYTimerCount == SMOOTH_STEPS)
			killSmoothYTimer();
		return ;
	case TIMER_SMOOTHSCROLLX:
		smoothScrollXCur += smoothScrollXInc;
		scrollToX((int)smoothScrollXCur, FALSE);
		if (++smoothScrollXTimerCount == SMOOTH_STEPS)
			killSmoothXTimer();
		return ;
	}
	SCRLBKGWND_PARENT::timerCallback(id);
}

// Gets notification from sliders
int ScrlBkgWnd::childNotify(ifc_window *child, int msg, intptr_t param1, intptr_t param2)
{
	switch (msg)
	{
	case ChildNotify::SCROLLBAR_SETPOSITION:
		if (child == &vScroll)
		{
			updateScrollY(!!param1);
			return 1;
		}
		if (child == &hScroll)
		{
			updateScrollX(!!param1);
			return 1;
		}
		break;
	}

	return SCRLBKGWND_PARENT::childNotify(child, msg, param1, param2);
}

int ScrlBkgWnd::onResize()
{
	int rt = SCRLBKGWND_PARENT::onResize();
	if (!isInited()) return rt;
	invalidateRect(&smsqr);
	setSlidersPosition();
	return 1;
}

void ScrlBkgWnd::onSetVisible(int show)
{
	SCRLBKGWND_PARENT::onSetVisible(show);
	if (show)
		setSlidersPosition();
}

void ScrlBkgWnd::getClientRect(RECT *r)
{
	SCRLBKGWND_PARENT::getClientRect(r);
	if (vScroll.isVisible(1))
		r->right -= getScrollbarWidth() + (wantsep ? SCROLLBAR_SEP : 0);
	if (hScroll.isVisible(1))
		r->bottom -= getScrollbarWidth() + (wantsep ? SCROLLBAR_SEP : 0);
	r->top += getHeaderHeight();
}

/*void ScrlBkgWnd::getNonClientRect(RECT *r) {
  SCRLBKGWND_PARENT::getClientRect(r); // my non client rect is my parent's client rect
  return;
}*/

int ScrlBkgWnd::getHeaderHeight()
{
	return 0;
}

void ScrlBkgWnd::setLineHeight(int h)
{
	lineHeight = h;
}

int ScrlBkgWnd::getLinesPerPage()
{
	RECT r;
	getClientRect(&r);
	int h = r.bottom - r.top;
	return h / lineHeight;
}

int ScrlBkgWnd::getScrollX()
{
	return scrollX;
}

int ScrlBkgWnd::getScrollY()
{
	return scrollY;
}

int ScrlBkgWnd::getScrollbarWidth()
{
	// TODO: maybe do if (hScroll.isVisible())
	return hScroll.getWidth();
	return vScroll.getWidth();
	return 0;
}

/*void ScrlBkgWnd::clientToScreen(RECT *r) {
  POINT p;
  p.x = r->left;
  p.y = r->top;
  SCRLBKGWND_PARENT::clientToScreen((int *)&p.x, (int*)&p.y);
  r->left = p.x;
  r->top = p.y;
 
  p.x = r->right;
  p.y = r->bottom;
  SCRLBKGWND_PARENT::clientToScreen((int *)&p.x, (int*)&p.y);
  r->right = p.x;
  r->bottom = p.y;
}
 
void ScrlBkgWnd::clientToScreen(int *x, int *y) {
  SCRLBKGWND_PARENT::clientToScreen(x, y);
}
 
void ScrlBkgWnd::clientToScreen(POINT *p) {
  TREEWND_PARENT::clientToScreen((int *)&p->x, (int *)&p->y);
}*/

void ScrlBkgWnd::makeWindowOverlayMask(api_region *r)
{

	return ;
#ifdef WIN32
	// With this routine empty, I'm just nuking the code from x-plat builds < KP
	HDC dc = GetDC(getOsWindowHandle());

	//if (getRandomRgn)
	{
		RECT cr;
		getClientRect(&cr);
		RECT wr;
		getWindowRect(&wr);

		RegionI sr;
		Wasabi::Std::Wnd::getRandomRegion(dc, sr.getOSHandle());
		sr.offset( -wr.left, -wr.top);

		r->setRect(&cr);
		r->subtractRegion(&sr);

	}

	ReleaseDC(getOsWindowHandle(), dc);
#endif
}
