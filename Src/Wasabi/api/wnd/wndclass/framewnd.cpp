#include <precomp.h>

#include "framewnd.h"
#include <api/wnd/notifmsg.h>
#include <bfc/bfc_assert.h>
#include <tataki/canvas/canvas.h>
#include <api/wnd/PaintCanvas.h>
#include <bfc/wasabi_std_wnd.h>

#define DC_FWFOCUS 0x5122

FrameWnd::FrameWnd()
{
//  sizer = NULL;
	SNAP=1;
	snapoffsety=0;
	snapoffsetx=0;
	nchild = 0;
	for (int i = 0; i < MAXCHILD; i++) {
		children[i] = NULL;
		rwchildren[i] = NULL;
		hidey[i] = 0;
		windowshaded[i] = 0;
	}
	vert = DIVIDER_UNDEFINED;
	divideside = SDP_FROMLEFT;
	pullbarpos = PULLBAR_HALF;
	minwidth = PULLBAR_QUARTER-PULLBAR_EIGHTH;
	maxwidth = PULLBAR_HALF;
	resizeable = 0;
	slidemode = FRAMEWND_SQUISH;
	prevpullbarpos = -1;
	maxpixels=0;
	minpixels=0;
	noMaxRestriction = false;
	ZERO(sizerRect);

	h_bitmap = L"wasabi.framewnd.horizontaldivider";
	v_bitmap = L"wasabi.framewnd.verticaldivider";
	h_grabber = L"wasabi.framewnd.horizontalgrabber";
	v_grabber = L"wasabi.framewnd.verticalgrabber";
	ws_bitmap = L"wasabi.framewnd.windowshade";
	resizing = 0;
}

void FrameWnd::Set_v_bitmap(const wchar_t *new_v_bitmap)
{
	v_bitmap=new_v_bitmap;
	if (isInited()) 
	{
		invalidate();
		onResize();
	}
}

void FrameWnd::Set_v_grabber(const wchar_t *new_v_grabber)
	{
			v_grabber=new_v_grabber;
	if (isInited()) 
	{
		invalidate();
		onResize();
	}
}

FrameWnd::~FrameWnd() {
#ifdef WASABI_COMPILE_CONFIG
	if (getId() != NULL) {
		StringPrintfW buf(L"FrameWnd/ws,%s", getId());
		WASABI_API_CONFIG->setIntPrivate(buf, windowshaded[0]);
	}
#endif

	if (children[0]) // do we have any basewnd ?
		for (int i = 0; i < nchild; i++) delete children[i];
}

int FrameWnd::onInit() {
  int i;

  FRAMEWND_PARENT::onInit();

  ASSERT(vert != DIVIDER_UNDEFINED || nchild == 0);

  // have to set children for frame windows

  // fill in members
  nchild = 0;

  // make children create their windows
  for (i = 0; i < MAXCHILD; i++) {
    if (rwchildren[i] != NULL) {
      if (rwchildren[i]->init(this) != 0) {
        rwchildren[i]->setParent(this);
        nchild++;
      }
    }
  }
  prevpullbarpos = pullbarpos;

  if (nchild >= MAXCHILD) {
    int which = (divideside == SDP_FROMLEFT) ? 0 : 1;
    rwchildren[which]->bringToFront();
  }

#ifdef WASABI_COMPILE_CONFIG
  if (getId() != NULL) {
    StringPrintfW buf(L"FrameWnd/ws,%s", getId());
    int ws = WASABI_API_CONFIG->getIntPrivate(buf, /*rwchildren[0] && rwchildren[0]->childNotify(NULL, CHILD_WINDOWSHADE_CAPABLE)*/ 0);
    if (ws) {
      windowshade(0, !ws);
      windowshade(0, ws);
      pullbarpos = 0;
    }
  }
#endif

  return 1;
}

int FrameWnd::getCursorType(int x, int y) {
  RECT r;
  getClientRect(&r);
  POINT pt={x,y};
  if (y > r.top + getLabelHeight() && Wasabi::Std::pointInRect(sizerRect, pt)) {
    if (vert == DIVIDER_HORIZONTAL) return BASEWND_CURSOR_NORTHSOUTH;
    else return BASEWND_CURSOR_EASTWEST;
  }
  return BASEWND_CURSOR_POINTER;
}

int FrameWnd::setChildren(BaseWnd *newchild1, BaseWnd *newchild2) {
  return _setChildren(newchild1, newchild2, newchild1, newchild2);
}

int FrameWnd::setChildrenRootWnd(ifc_window *child1, ifc_window *child2/* =NULL */) {
  return _setChildren(child1, child2, NULL, NULL);
}

int FrameWnd::_setChildren(ifc_window *child1, ifc_window *child2, BaseWnd *child1b, BaseWnd *child2b) {
  if (child1b) { // we can delete them later
    children[0] = child1b;
    children[1] = child2b;
  } 
  rwchildren[0] = child1;
  rwchildren[1] = child2;
  nchild = 0;
  if (rwchildren[0] != NULL) nchild++;
  if (rwchildren[1] != NULL) nchild++;

  ASSERTPR(nchild >= 1, "framewnd must have one or more children");

  if (isInited()) {
    invalidate();
    onResize();
  }
  return nchild;
}

ifc_window *FrameWnd::enumChild(int which) {
  if (which < 0 || which >= MAXCHILD) return NULL;
  return rwchildren[which];
}

int FrameWnd::childNotify(ifc_window *which, int msg, intptr_t param1, intptr_t param2) {
//  ASSERT(which == rwchildren[0] || which == rwchildren[1] || which == NULL);
  switch (msg) {
    case ChildNotify::FRAMEWND_SETTITLEWIDTH:
      if (pullbarpos == param1) return 0;
      ASSERT(param1 >= 0);
        if (which == rwchildren[0]) {
          // rwchildren[1]->invalidate(); //FG> removed due to change in redraw layout
          // rwchildren[1]->repaint();
          ASSERT(divideside == SDP_FROMLEFT);
        } else {
          // rwchildren[0]->invalidate();
          // rwchildren[0]->repaint();
          ASSERT(divideside == SDP_FROMRIGHT);
        }
      pullbarpos = param1;
      // do it
      onResize();
    return 1;

    case ChildNotify::HIDEYHIDEY:
      if (which == rwchildren[0]) hidey[0] = 1;
      else if (which == rwchildren[1]) hidey[1] = 1;
      which->setVisible(FALSE);
      onResize();
    return 1;

    case ChildNotify::UNHIDEYHIDEY:
      if (which == rwchildren[0]) hidey[0] = 0;
      else if (which == rwchildren[1]) hidey[1] = 0;
      which->setVisible(TRUE);
      onResize();
    return 1;

    case ChildNotify::FRAMEWND_QUERY_SLIDE_MODE:
      return getSlideMode();

    case ChildNotify::FRAMEWND_SET_SLIDE_MODE:
      setSlideMode((FrameWndSlideMode)param1);
    break;
    case ChildNotify::GOTFOCUS:
    case ChildNotify::KILLFOCUS:
      invalidateLabel();
    break;
  }

  return FRAMEWND_PARENT::childNotify(which, msg, param1, param2);
}

/*int FrameWnd::forceFocus() {
  if (!canShowFocus()) return 0;	// we aren't showing a label
  int v = 0;
  if (nchild > 0 && rwchildren[0] != NULL) {
    if (!rwchildren[0]->canShowFocus()) v |= rwchildren[0]->gotFocus();
  }
  if (nchild > 1 && rwchildren[1] != NULL) {
    if (!rwchildren[1]->canShowFocus()) v |= rwchildren[1]->gotFocus();
  }
  return v;
}*/

void FrameWnd::setDividerType(FrameWndDividerType type) {
  vert = type;
  ASSERT(vert == DIVIDER_VERTICAL || vert == DIVIDER_HORIZONTAL);
  if (isInited())
    onResize();
}

FrameWndDividerType FrameWnd::getDividerType() {
  return vert;
}

int FrameWnd::ConvertPixToProp() {
  RECT r;
  int w;
  getClientRect(&r);
  if(vert == DIVIDER_VERTICAL) {
    w = r.right-r.left;
  } else {
    w = r.bottom-r.top;
  }
  w = (pullbarpos * PULLBAR_FULL) / w;
  return w;
}

int FrameWnd::convertPropToPix(int prop) {
  RECT r;
  int w;

  getClientRect(&r);
  if(vert == DIVIDER_VERTICAL) {
    w = r.right-r.left;
  } else {
    w = r.bottom-r.top;
  }
  return (w * prop) / PULLBAR_FULL;
}

int FrameWnd::setDividerPosNoCfg(int from, int pos) {
  divideside = from;

  ASSERT(pos >= 0);
  pullbarpos = pos;
  if (isInited())
    onResize();

	StringPrintfW buf(L"FrameWnd/%s,p", getId());
  WASABI_API_CONFIG->setIntPrivate(buf, pullbarpos);

  return 1;
}

int FrameWnd::setDividerPos(int from, int pos) {
#ifdef WASABI_COMPILE_CONFIG
  if (getId() != NULL) {
    StringPrintfW buf(L"FrameWnd/%s,p", getId());
    pos = WASABI_API_CONFIG->getIntPrivate(buf, pos);
    if (pos <= 0) pos = 0;
    else if (pos >= PULLBAR_FULL) pos = PULLBAR_FULL;
  }
#endif
  return setDividerPosNoCfg(from, pos);
}

void FrameWnd::getDividerPos(int *from, int *pos) {
  if (from != NULL) *from = divideside;
  if (pos != NULL) *pos = pullbarpos;
}

int FrameWnd::setResizeable(int is) {
  int prev = resizeable;
  resizeable = is;
  return prev;
}

void FrameWnd::setMinWidth(int min) {
  //ASSERT(min >= 0);
  minpixels = min;
}

void FrameWnd::setMaxWidth(int max)
{
  //ASSERT(max >= 0);
	maxpixels=max;
	noMaxRestriction = (max == 0);
  //maxwidth = max;
}

void FrameWnd::setSlideMode(FrameWndSlideMode mode) {
  slidemode = mode;
  if (isInited())
    onResize();
}

FrameWndSlideMode FrameWnd::getSlideMode() {
  return slidemode;
}

int FrameWnd::dragEnter(ifc_window *sourceWnd) {
  ifc_window *ch = getWindowShadedChild();
  if (ch == NULL) return FRAMEWND_PARENT::dragEnter(sourceWnd);
  return ch->getDragInterface()->dragEnter(sourceWnd);
}

int FrameWnd::dragOver(int x, int y, ifc_window *sourceWnd) {
  ifc_window *ch = getWindowShadedChild();
  if (ch == NULL) return FRAMEWND_PARENT::dragOver(x, y, sourceWnd);
  return ch->getDragInterface()->dragOver(-1, -1, sourceWnd);
}

int FrameWnd::dragLeave(ifc_window *sourceWnd) {
  ifc_window *ch = getWindowShadedChild();
  if (ch == NULL) return FRAMEWND_PARENT::dragLeave(sourceWnd);
  return ch->getDragInterface()->dragLeave(sourceWnd);
}

int FrameWnd::dragDrop(ifc_window *sourceWnd, int x, int y) {
  ifc_window *ch = getWindowShadedChild();
  if (ch == NULL) return FRAMEWND_PARENT::dragDrop(sourceWnd, x, y);
  return ch->getDragInterface()->dragDrop(sourceWnd, x, y);
}

int FrameWnd::onResize() 
{
  int rt = FRAMEWND_PARENT::onResize();
  if (!isInited()) return rt;
  RECT r;
  int sizerwidth = SIZERWIDTH;

  if (!isInited()) {
    prevpullbarpos = pullbarpos;
    return 1;	// no window to resize
  }

  getClientRect(&r);

  ASSERT(nchild >= 0);
  if (nchild == 0) {
    prevpullbarpos = pullbarpos;
    return 1;
  }

  if (hidey[0] && hidey[1]) return 0;	// both windows are hiding

  // if we have only one child, it takes up all the room
  if (hidey[0]) {
    rwchildren[1]->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
    return 1;
  } else if (hidey[1]) {
    rwchildren[0]->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
    return 1;
  }

  if (nchild == 1) {
    if (rwchildren[0] != NULL) rwchildren[0]->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
    else if (rwchildren[1] != NULL) rwchildren[1]->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
    return 1;
  }

#ifdef ASSERTS_ENABLED
  for (int i = 0; i < nchild; i++) {
    ASSERT(rwchildren[i] != NULL);
  }
#endif

  if (!resizeable) sizerwidth = 0;

  // resize the subwindows

  int w;
  if (vert == DIVIDER_VERTICAL) {
    w = r.right-r.left;
  } else {
    w = r.bottom-r.top;
  }
  int clientwidth = w;	// the logical width

  switch (pullbarpos) {
    case PULLBAR_FULL: /*w = w;*/ break;
    case PULLBAR_HALF: w = w/2; break;
    case PULLBAR_QUARTER: w = w/4; break;
    case PULLBAR_THREEQUARTER: w = w - w/4; break;
    case PULLBAR_EIGHTH: w = w/8; break;
    default: w = pullbarpos; break;
  }

	// maxpixels holds normally a negative or zero value!
	if (divideside == SDP_FROMRIGHT)
	{
		w = (clientwidth - w);
		if (maxpixels < 1 && w < -maxpixels) w = -maxpixels; // Martin> This fixes an ugly drawing overlap
		// TODO: check non-relative width as well, imoh we should rewrite this function from scrap.
	}
	else // FROMLEFT
	{
		if (maxpixels < 1 && w > clientwidth + maxpixels)
			w = clientwidth + maxpixels;
		if (w < minpixels)
			w = minpixels;
	}

  RECT r1, r2;

  if (slidemode == FRAMEWND_COVER) { // cover mode

    ASSERTPR(vert == DIVIDER_VERTICAL, "finish implementing");

    if (divideside == SDP_FROMRIGHT) {
        Wasabi::Std::setRect(&r1, r.left, r.top, r.right-r.left, r.bottom-r.top); //FG> delay resize
        Wasabi::Std::setRect(&r2, r.left+w, r.top, r.left+clientwidth - w, r.bottom-r.top);
    } else {
        Wasabi::Std::setRect(&r1, r.left, r.top, r.left+w, r.bottom-r.top); //FG> delay resize
        Wasabi::Std::setRect(&r2, r.left, r.top, r.right-r.left, r.bottom-r.top);
    }

    sizerRect.top = r.top;
    sizerRect.bottom = r.bottom;
    sizerRect.left = r.left + w;
    sizerRect.right = r.left + w + sizerwidth;

  } else { // squish mode
    // left-right
    if (vert == DIVIDER_VERTICAL) {
      sizerRect.top = r.top;
      sizerRect.bottom = r.bottom;
      if (divideside == SDP_FROMLEFT) { // from left

        //FG> Warning, this is using a rect for x,y,W,H and NOT l,r,t,b
        Wasabi::Std::setRect(&r1, r.left, r.top, w, r.bottom-r.top);
        Wasabi::Std::setRect(&r2, r.left+w+sizerwidth, r.top, (r.right-r.left)-(w+sizerwidth), r.bottom-r.top);

        sizerRect.left = r.left+w;
        sizerRect.right = sizerRect.left + sizerwidth;
      } 
      else {	// from right

        //FG> Warning, this is using a rect for x,y,W,H and NOT l,r,t,b
        Wasabi::Std::setRect(&r1, r.left, r.top, w-sizerwidth, r.bottom-r.top);
        Wasabi::Std::setRect(&r2, r.left+w, r.top, (r.right-r.left)-w, r.bottom-r.top);

        sizerRect.left = r.left+w-sizerwidth;
        sizerRect.right = r.left+w;
      }
    } else {
      // top-bottom

      //FG> Warning, this is using a rect for x,y,W,H and NOT l,r,t,b
        Wasabi::Std::setRect(&r1, r.left, r.top, r.right-r.left, w);
        Wasabi::Std::setRect(&r2, r.left, r.top+w+sizerwidth, r.right-r.left, (r.bottom-r.top)-(w+sizerwidth));

      sizerRect.top = r.top+w;
      sizerRect.bottom = r.top+w+sizerwidth;
      sizerRect.left = r.left;
      sizerRect.right = r.right;
    }
  }

  //FG> Choose resizing order. optimizes redraw by avoiding temporary overlap of rwchildren
  bool reverse = false;
  if (vert == DIVIDER_VERTICAL) {
    RECT o;
    rwchildren[0]->getNonClientRect(&o);
    reverse = (r1.right > o.right);
  } else {
    RECT o;
    rwchildren[0]->getNonClientRect(&o);
    reverse = (r1.bottom > o.bottom);    
  }

  //FG> actually resize rwchildren
  //FG> Warning, this is using a rect for x,y,W,H and NOT l,r,t,b
  if (reverse) {
    rwchildren[1]->resize(r2.left, r2.top, r2.right, r2.bottom);
    rwchildren[0]->resize(r1.left, r1.top, r1.right, r1.bottom);
  } else {
    rwchildren[0]->resize(r1.left, r1.top, r1.right, r1.bottom);
    rwchildren[1]->resize(r2.left, r2.top, r2.right, r2.bottom);
  }

  onResizeChildren(r1, r2);

//  RECT ri = sizerRect;
#if 0
  if (vert == DIVIDER_HORIZONTAL) {
    ri.left -= 2;
    ri.right += 2;
  } else {
    ri.top -= 2;
    ri.bottom += 2;
  }
#endif
//  invalidateRect(&ri);
  invalidate();
	repaint();

  prevpullbarpos = pullbarpos;

  return 1;
}

void FrameWnd::onResizeChildren(RECT leftr, RECT rightr) {
}

int FrameWnd::onPaint(Canvas *canvas) {

  RECT d;
  getClientRect(&d);
  if ((d.left >= d.right) || (d.top >= d.bottom)) {
    return FRAMEWND_PARENT::onPaint(canvas);
  }

  RECT cr;
//  PaintBltCanvas paintcanvas;
  PaintCanvas paintcanvas;

  // if only 1 child, we don't paint anything
  if (nchild <= 1) return FRAMEWND_PARENT::onPaint(canvas);

  if (canvas == NULL) {
    if (!paintcanvas.beginPaint(this)) return 0;
    canvas = &paintcanvas;
  }
  FRAMEWND_PARENT::onPaint(canvas);

  getClientRect(&cr);

  if (wantRenderBaseTexture() || !isVirtual())
    renderBaseTexture(canvas, cr);

  if (resizeable) {
    RECT r = sizerRect;
    if (vert == DIVIDER_HORIZONTAL) {
      r.left -= 2;
      r.right += 2;
    } else {
      r.top -= 2;
      r.bottom += 2;
    }

    AutoSkinBitmap &bitmap = (vert == DIVIDER_VERTICAL) ? v_bitmap : h_bitmap;
    bitmap.stretchToRectAlpha(canvas, &r);

    if (vert == DIVIDER_VERTICAL) {
      int h = sizerRect.bottom - sizerRect.top;
      int gh = v_grabber.getHeight();
      if (h > gh) {
        RECT rr = sizerRect;
        rr.top += (h - gh) / 2;
        rr.bottom -= (h - gh) / 2;
        v_grabber.stretchToRectAlpha(canvas, &rr);
      }
    } else {
      int w = sizerRect.right - sizerRect.left;
      int gw = h_grabber.getWidth();
      if (w > gw) {
        RECT rr = sizerRect;
        rr.left += (w - gw) / 2;
        rr.right -= (w - gw) / 2;
        h_grabber.stretchToRectAlpha(canvas, &rr);
      }
    }

    if (windowshaded[0]) {
      RECT wr = cr;
      if (vert == DIVIDER_VERTICAL) {
        wr.right = r.left;
      } else if (vert == DIVIDER_HORIZONTAL) {
        wr.bottom = r.top;
      }

      ws_bitmap.stretchToRect(canvas, &wr);
    }

  }

  return 1;
}

int FrameWnd::onLeftButtonDown(int x, int y) {
  FRAMEWND_PARENT::onLeftButtonDown(x, y);
  if (!resizeable) return 1;
  POINT p = { x, y };
  if (Wasabi::Std::pointInRect(sizerRect, p)) {
    beginCapture();
		RECT r;
		getClientRect(&r);
		x -= r.left;
	  y -= r.top;
		snapoffsety= y - (y % SNAP);
		snapoffsetx= x - (x % SNAP);
    resizing = 1;
    return 1;
  }
  return 0;
}

int FrameWnd::onMouseMove(int x, int y) {
  int pos, mpos;
  RECT r;

  if (!resizing) return 1;

  FRAMEWND_PARENT::onMouseMove(x,y);

  prevpullbarpos = pullbarpos;
  
  getClientRect(&r);
  x -= r.left;
  y -= r.top;

  if (vert == DIVIDER_VERTICAL) {
    pos = r.right - r.left;
		if ((x - (x % SNAP)) == snapoffsetx)
			return 1;
		mpos=x;
		snapoffsetx=(x - (x % SNAP));
  } else {
    pos = r.bottom - r.top;
		if ((y - (y % SNAP)) == snapoffsety)
			return 1;
		mpos=y;
		snapoffsety=y - (y % SNAP);
  }
  ASSERT(pos != 0);
  if (mpos < 0) mpos = 0;
  if (mpos > pos) mpos = pos;

  if(divideside == SDP_FROMLEFT) {
    pullbarpos = mpos;
  } else {
    pullbarpos = pos-mpos;
  }

	int realMinPixels;
	if (minpixels)
	{
					realMinPixels=minpixels;
			if (minpixels<0)
				realMinPixels = (r.bottom - r.top) + minpixels;
	}
	else
		realMinPixels = convertPropToPix(minwidth);

  if (divideside == SDP_FROMLEFT) 
	{
    if (pullbarpos < realMinPixels) 
		{
      if (rwchildren[0] != NULL && rwchildren[0]->childNotify(NULL, ChildNotify::FRAMEWND_WINDOWSHADE_CAPABLE, 0, 0)) {
        pullbarpos = 0;
        windowshade(0, TRUE);
      } else {
        pullbarpos = realMinPixels;
      }
    } else {
      windowshade(0, FALSE);
    }
  } else if (divideside == SDP_FROMRIGHT) {
    if (pullbarpos < realMinPixels) {
      if (rwchildren[1] != NULL /* && rwchildren[1]->childNotify(NULL, CHILD_WINDOWSHADE_CAPABLE) */) {
        pullbarpos = /*convertPropToPix(PULLBAR_FULL)-*/0;
        windowshade(1, TRUE);
      } else {
        pullbarpos = realMinPixels;
      }
    } else {
      windowshade(1, FALSE);
    }
  }

  if (!windowshaded[0] && !windowshaded[1]) {
//    if (pullbarpos > pos-convertPropToPix(minwidth))
//      pullbarpos = pos-convertPropToPix(minwidth);
		int realMaxPixels;
		if (maxpixels || noMaxRestriction)
		{
			realMaxPixels=maxpixels;
			if (maxpixels<0 || noMaxRestriction)
			{
				if (vert == DIVIDER_VERTICAL)
					realMaxPixels = (r.right - r.left) + maxpixels;
				else
					realMaxPixels = (r.bottom - r.top) + maxpixels;
			}
				
		}
		else
			realMaxPixels=convertPropToPix(maxwidth);

			 if (pullbarpos > realMaxPixels)
        pullbarpos = realMaxPixels;
  }

  ASSERT(pullbarpos >= 0);

  if (pullbarpos != prevpullbarpos && isInited())
    onResize();

  return 1;
}

int FrameWnd::onLeftButtonUp(int x, int y) {
  FRAMEWND_PARENT::onLeftButtonUp(x, y);
  if (resizing) {
    endCapture();
    resizing = 0;
#ifdef WASABI_COMPILE_CONFIG
    if (getId() != NULL) {
      StringPrintfW buf(L"FrameWnd/%s,p", getId());
      WASABI_API_CONFIG->setIntPrivate(buf, pullbarpos);
    }
#endif
    return 1;
  }
  return 0;
}

void FrameWnd::windowshade(int which, int shaded) {
  ASSERT(which == 0 || which == 1);
  if (!!windowshaded[which] == !!shaded) return;
  if (rwchildren[which] == NULL) return;
  rwchildren[which]->childNotify(NULL, ChildNotify::FRAMEWND_WINDOWSHADE_ENABLE, shaded, 0);
  windowshaded[which] = shaded;
  rwchildren[which]->setVisible(!shaded);
}

ifc_window *FrameWnd::getWindowShadedChild() {
  if (nchild != 2) return NULL;
  if (!(windowshaded[0] | windowshaded[1])) return NULL;
  return windowshaded[0] ? rwchildren[0] : rwchildren[1];
}

int FrameWnd::onGetFocus() {
  postDeferredCallback(DC_FWFOCUS, 0);
  return 1;
}

int FrameWnd::onDeferredCallback(intptr_t p1, intptr_t p2) {
  switch (p1) {
    case DC_FWFOCUS:
      if (rwchildren[0]) rwchildren[0]->setFocus();
      break;
    default:
      return FRAMEWND_PARENT::onDeferredCallback(p1, p2);
  }
  return 1;
}


void FrameWnd::setSnap(int snap)
{
	if (snap>0)
		SNAP=snap;
}