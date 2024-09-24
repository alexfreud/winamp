#include <precomp.h>
#include "alphamgr.h"
#include <api/wndmgr/layout.h>
#include <api/skin/skinparse.h>
#ifdef _WIN32
#include <tataki/blending/blending.h>
#endif
#include <bfc/util/profiler.h>
#include <bfc/wasabi_std_wnd.h>

#ifndef PI
#define PI 3.1415926536
#endif

#define ALPHAMGR_HOVERCHECK    100
#define ALPHAMGR_UPDATEALPHA   200

AlphaMgr::AlphaMgr() {
	overlayout = NULL;
	timerclient_setTimer(ALPHAMGR_HOVERCHECK, 200);
	alllinked = 0;
	autoopacify = 0;
	fast_timer_on = 0;
	big_curtransparency = 0;
	big_status = STATUS_UNKNOWN;
	big_startalpha = 0;
	big_lasttimein = 0;
	extend_px = 0;
	global_alpha = 0;
	big_enterleave_time = 0;
    fadein_ms = 1;
    fadeout_ms = 1;
    holdtime_ms = 1;
}

AlphaMgr::~AlphaMgr() 
{
	timerclient_killTimer(ALPHAMGR_HOVERCHECK);
}

void AlphaMgr::addLayout(Layout *l)
{
  checkTimer();

  if (layouts.findItem((const wchar_t *)l))
      return;

  layouts.addItem(new AlphaMgrEntry(l));
}

void AlphaMgr::removeLayout(Layout *l) {
  int p=-1;
  AlphaMgrEntry *e = layouts.findItem((const wchar_t *)l, &p);
  if (p != -1) {
    if (e->getStatus() == STATUS_IN_FADINGON || e->getStatus() == STATUS_IN_ON) {
      updateInList(e, 0);
      checkTimer();
    }
    layouts.removeByPos(p);
    in_layouts.removeItem(e);
    delete e;
    checkTimer();
  }
}

// gets the currently needed transparency, according to layout overrides and global link, and then applies this
// transparency to the layout, it does not change anything in any data structure, this is only a visual update function
void AlphaMgr::updateTransparency(Layout *l) {
  if (!l) return;
  if (l->isInited()) {
    if (l->isTransparencySafe()) {
      int a = l->getTransparencyOverride();
      if (a == -1) {
        if (l->getNoParent()!=1) {
          if (a == -1 && hasAutoOpacity(l))
            a = getTransparency(l);
          else if (a == -1 && getAllLinked())
            a = getGlobalAlpha();
        }
        if (a == -1) {
          /* why the hell would it care if it's alllinked if it's an independent window ?? (noparent=1)
          if (getAllLinked())
            a = getGlobalAlpha();
          else 
          */
            a = l->getPaintingAlpha();
        }
      }
      l->setTransparency(a);
    } else {
      l->setTransparency(255);
    }
  }
}

// returns the alpha value for this slot, that's not necessarily the transparency that should be applied to the layout
// since overrides & calculations in updateTransparency and getTransparency should apply. 
int AlphaMgr::getAlpha(AlphaMgrEntry *e) {
  if (alllinked && e->getLayout()->getNoParent() != 1) return getGlobalAlpha();
  return e->getLayout()->getAlpha();
}

int AlphaMgr::getAlpha(Layout *l) {
  int p=-1;
  AlphaMgrEntry *e = layouts.findItem((const wchar_t *)l, &p);
  if (p != -1) return getAlpha(e);
  return l->getAlpha();
}

int AlphaMgr::getGlobalAlpha() {
  return global_alpha;
}

void AlphaMgr::updateAllTransparency() {
  foreach(layouts)
    updateTransparency(layouts.getfor()->getLayout());
  endfor;
}

void AlphaMgr::setGlobalAlpha(int a) {
  global_alpha = a;
  updateAllTransparency();
}

int AlphaMgr::getCurve(AlphaMgrEntry *e) {
  int n;
  int status = e ? e->getStatus() : getBigStatus();
  if (e == NULL) {
    n = MulDiv(Wasabi::Std::getTickCount()-getBigEnterLeaveTime(),256,status == STATUS_IN_FADINGON ? fadein_ms : fadeout_ms);
    if (n > 255) n = 255; if (n < 0) n = 0;
  } else {
    if (e->getEnterLeaveTime() == -1) return -1;
    n = MulDiv(Wasabi::Std::getTickCount()-e->getEnterLeaveTime(),256,status == STATUS_IN_FADINGON ? fadein_ms : fadeout_ms);
    if (n > 255) n = 255; if (n < 0) n = 0;
  }
  return n;
}

// returns the value of the transparency if no override applies, you still need to check overrides (see updatetransparency)
int AlphaMgr::getTransparency(Layout *l) {
  if (getAutoOpacify()) 
    l = NULL;
  if (l == NULL) {
    if (getBigStatus() == STATUS_UNKNOWN) { 
      setBigStatus(STATUS_OUT_OFF); 
      Layout *main = SkinParser::getMainLayout();
      if (main)
        big_curtransparency = main->getTransparency();
      else
        big_curtransparency = 255;
    }
  }
  AlphaMgrEntry *e = NULL;
  if (l) e = layouts.findItem((const wchar_t *)l);
  int s = e ? e->getStatus() : getBigStatus();
  if (e && s == STATUS_UNKNOWN) {
    initStatus(e);
    s = e->getStatus();
  }
  switch (s) {
    case STATUS_IN_OFF: return e ? getAlpha(e) : getGlobalAlpha();
    case STATUS_OUT_OFF: return e ? getAlpha(e) : getGlobalAlpha();
    case STATUS_IN_ON: return 255;
    case STATUS_OUT_FADINGOUT: {
      int n = e ? getCurve(e) : getCurve(NULL);
  	  float sintrans = (float)(sin(((float)n/255)*PI-PI/2)/2+0.5);
      int na;
      if (e)
        na = (int)(((float)(getAlpha(e) - e->getStartAlpha()) * sintrans) + e->getStartAlpha());
      else 
        na = (int)(((float)(getGlobalAlpha() - getBigStartAlpha()) * sintrans) + getBigStartAlpha());
      return na;
    }
    case STATUS_IN_FADINGON: {
      int n = e ? getCurve(e) : getCurve(NULL);
  	  float sintrans = (float)(sin(((float)n/255)*PI-PI/2)/2+0.5);
      int na;
      if (e)
        na = (int)(((float)(255 - e->getStartAlpha()) * sintrans) + e->getStartAlpha());
      else
        na = (int)(((float)(255 - getBigStartAlpha()) * sintrans) + getBigStartAlpha());
      return na;
    }
    default: return e ? getAlpha(e) : getGlobalAlpha();
  }
}

int AlphaMgr::hasAutoOpacityOnHover(Layout *l) {
  AlphaMgrEntry *e = layouts.findItem((const wchar_t *)l);
  if (e) return hasAutoOpacityOnHover(e);
  return 0;
}

int AlphaMgr::hasAutoOpacity(Layout *l) {
  AlphaMgrEntry *e = layouts.findItem((const wchar_t *)l);
  if (e) return hasAutoOpacity(e);
  return 0;
}

int AlphaMgr::hasAutoOpacityOnFocus(Layout *l) {
  AlphaMgrEntry *e = layouts.findItem((const wchar_t *)l);
  if (e) return hasAutoOpacityOnFocus(e);
  return 0;
}

int AlphaMgr::hasAutoOpacityOnFocus(AlphaMgrEntry *e) {
  if (alllinked) return autoopacify == 2;
  return e->getLayout()->getAutoOpacify() == 2 && e->getLayout()->getNoParent() != 1;
}

int AlphaMgr::hasAutoOpacityOnHover(AlphaMgrEntry *e) {
  if (alllinked) return autoopacify == 1;
  return e->getLayout()->getAutoOpacify() == 1 && e->getLayout()->getNoParent() != 1;
}

int AlphaMgr::hasAutoOpacity(AlphaMgrEntry *e) {
  if (alllinked) return autoopacify;
  return e->getLayout()->getAutoOpacify() && e->getLayout()->getNoParent() != 1;
}

// we got a new layout to manage, and its status flags is not set, we should init it to something safe
void AlphaMgr::initStatus(AlphaMgrEntry *l, int applytransparency) {
  if (isMouseInLayout(l->getLayout())) {
	l->setEnterLeaveTime((uint32_t)-1);
    if (hasAutoOpacity(l)) {
      l->setStatus(STATUS_IN_FADINGON);
      l->onEnterLeave();
      l->setStartAlpha(l->getLayout()->getTransparency());
      checkTimer();
    } else {
      l->setStatus(STATUS_IN_OFF);
    }
    l->getLayout()->onMouseEnterLayout();
  } else {
    if (hasAutoOpacityOnHover(l)) {
      l->setStartAlpha(l->getLayout()->getTransparency());
      l->onEnterLeave();
      l->setStatus(STATUS_OUT_FADINGOUT);
      checkTimer();
    } else {
      l->setStatus(STATUS_OUT_OFF);
    }
    l->getLayout()->onMouseLeaveLayout();
  }
  if (applytransparency) updateTransparency(l->getLayout());
}

int AlphaMgr::isPointInLayout(Layout *l, int x, int y, api_region **rgn) 
{
  api_region *rg = NULL;
  if (!l) return 0;
  if (!l->isVisible()) return 0;
  RECT r,r2;
  l->getClientRect(&r);
  if (l->renderRatioActive()) l->multRatio(&r);
  l->getWindowRect(&r2);
  Wasabi::Std::offsetRect(&r, r2.left, r2.top);
//  OffsetRect(&r, r2.left, r2.top);
  if (x < r.left || x > r.right || y < r.top || y > r.bottom) return 0;
  if (rgn) {
    if (!*rgn) {
      rg = l->getComposedRegion();
      *rgn = rg;
    } else {
      rg = *rgn;
    }
  } else {
    rg = l->getComposedRegion();
  }
  if (!rgn) return 1;
  x -= r.left; y -= r.top;
  POINT pt={x,y};
  if (l->renderRatioActive()) l->divRatio((int*)&pt.x, (int*)&pt.y);
  if (l->getComposedRegion()->ptInRegion(&pt)) return 1;
  return 0;
}

int AlphaMgr::isFocusInLayout(Layout *l) {
  if (l->gotFocus()) return 1;
  OSWINDOWHANDLE fw = Wasabi::Std::Wnd::getFocus();
  while (fw) 
  {
    if (fw == l->gethWnd()) return 1;
    fw = Wasabi::Std::Wnd::getParent(fw);
  }
  return 0;
}

int AlphaMgr::isMouseInLayout(Layout *l) {
  int isin = 0;
  if (hasAutoOpacityOnFocus(l)) {
    return isFocusInLayout(l);
  } else {
    int x, y;
    api_region *r = NULL;
    Wasabi::Std::getMousePos(&x, &y);
    isin = isPointInLayout(l, x, y, &r);
    int ext = getExtendAutoOpacity();
    if (!isin && ext > 0) {
      isin = isPointInLayout(l, x-ext, y, &r);
      if (!isin) isin = isPointInLayout(l, x-ext, y-ext, &r);
      if (!isin) isin = isPointInLayout(l, x, y-ext, &r);
      if (!isin) isin = isPointInLayout(l, x+ext, y-ext, &r);
      if (!isin) isin = isPointInLayout(l, x+ext, y, &r);
      if (!isin) isin = isPointInLayout(l, x+ext, y+ext, &r);
      if (!isin) isin = isPointInLayout(l, x, y+ext, &r);
      if (!isin) isin = isPointInLayout(l, x-ext, y+ext, &r);
      if (!isin) isin = isPointInLayout(l, x-ext, y, &r);
    }
  }
  return isin;
}

int AlphaMgr::needForcedTransparencyFlag(Layout *l) {
  if (!l->isTransparencySafe()) return 0;
  if (l->isAlphaForced()) return 1;
  if (l->getTransparencyOverride() > 0) return 1; // should not be testing for < 255 here
  AlphaMgrEntry *e = layouts.findItem((const wchar_t *)l);
  if (hasAutoOpacity(e) && getAlpha(e) < 255) return 1;
  return 0;
}

void AlphaMgr::checkTimer() {
  int fading = 0;
  foreach(layouts)
    AlphaMgrEntry *e = layouts.getfor();
    if (e->getStatus() == STATUS_IN_FADINGON || e->getStatus() == STATUS_OUT_FADINGOUT) { fading++; break; }
  endfor;
  if (getAutoOpacify() && getBigStatus() == STATUS_IN_FADINGON || getBigStatus() == STATUS_OUT_FADINGOUT) 
    fading++;
  if (fading && !fast_timer_on) { 
    timerclient_setTimer(ALPHAMGR_UPDATEALPHA, 20); 
    fast_timer_on = 1; 
  } else if (!fading && fast_timer_on) { 
    timerclient_killTimer(ALPHAMGR_UPDATEALPHA); 
    fast_timer_on = 0; 
  }
}

void AlphaMgr::doEndCheck(AlphaMgrEntry *e) {
  if (getCurve(e) == 255) {
    switch (e ? e->getStatus() : getBigStatus()) {
      case STATUS_IN_FADINGON:
        if (e) e->setStatus(STATUS_IN_ON); else setBigStatus(STATUS_IN_ON);
        break;
      case STATUS_OUT_FADINGOUT:
        if (e) e->setStatus(STATUS_OUT_OFF); else setBigStatus(STATUS_OUT_OFF);
        break;
    }
    checkTimer();
  }
}

void AlphaMgr::updateInList(AlphaMgrEntry *e, int isin) {
  if (isin) {
    if (in_layouts.searchItem(e) == -1) 
    in_layouts.addItem(e);
  } else {
    in_layouts.removeItem(e);
  }
  
  int big_isin = in_layouts.getNumItems() > 0;
  
  if (getAutoOpacify()) {
    if (big_isin) {
      // mouse is in a layout, autoopacity is on
      switch (getBigStatus()) {
        case STATUS_OUT_OFF: 
        case STATUS_OUT_FADINGOUT:
        case STATUS_IN_OFF: { 
          setBigStartAlpha(e->getLayout()->getTransparency());
          onBigEnterLeave();
          setBigStatus(STATUS_IN_FADINGON);
          checkTimer();
          break;
        }
        case STATUS_IN_FADINGON: 
          doEndCheck(NULL);
          break;
      }
    } else {
      // mouse out of all layouts, autoopacity is on
      switch (getBigStatus()) {
        case STATUS_IN_FADINGON: 
        case STATUS_IN_ON: {
          setBigStartAlpha(getTransparency(NULL));
          onBigEnterLeave();
          setBigStatus(STATUS_OUT_FADINGOUT);
          checkTimer();
          break;
        }
        case STATUS_OUT_FADINGOUT:
          doEndCheck(NULL);
          break;
      }
    }
  } else {
    if (big_isin) {
      // mouse is in a layout, no autoopacity
      setBigStatus(STATUS_IN_OFF);
    } else {
      // mouse is out of all layouts, no autoopacity
      setBigStatus(STATUS_OUT_OFF);
    }
  }
}

int AlphaMgr::isFocusingExternalWindow() 
{  
  OSWINDOWHANDLE fw = Wasabi::Std::Wnd::getFocus();
  if (isOurExternalWindow(fw)) return 1;
  return 0;
}

int AlphaMgr::isOverExternalWindow() 
{
  int x, y;
  Wasabi::Std::getMousePos(&x, &y);
  int ext = getExtendAutoOpacity();

  POINT pt;
  pt.x = x; pt.y = y;
  OSWINDOWHANDLE w = Wasabi::Std::Wnd::getWindowFromPoint(pt);
  if (isOurExternalWindow(w)) return 1;
  pt.x = x-ext; pt.y = y-ext;
  w = Wasabi::Std::Wnd::getWindowFromPoint(pt);
  if (isOurExternalWindow(w)) return 1;
  pt.x = x; pt.y = y-ext;
  w = Wasabi::Std::Wnd::getWindowFromPoint(pt);
  if (isOurExternalWindow(w)) return 1;
  pt.x = x+ext; pt.y = y-ext;
  w = Wasabi::Std::Wnd::getWindowFromPoint(pt);
  if (isOurExternalWindow(w)) return 1;
  pt.x = x+ext; pt.y = y;
  w = Wasabi::Std::Wnd::getWindowFromPoint(pt);
  if (isOurExternalWindow(w)) return 1;
  pt.x = x+ext; pt.y = y+ext;
  w = Wasabi::Std::Wnd::getWindowFromPoint(pt);
  if (isOurExternalWindow(w)) return 1;
  pt.x = x; pt.y = y+ext;
  w = Wasabi::Std::Wnd::getWindowFromPoint(pt);
  if (isOurExternalWindow(w)) return 1;
  pt.x = x-ext; pt.y = y+ext;
  w = Wasabi::Std::Wnd::getWindowFromPoint(pt);
  if (isOurExternalWindow(w)) return 1;
  pt.x = x-ext; pt.y = y;
  w = Wasabi::Std::Wnd::getWindowFromPoint(pt);
  if (isOurExternalWindow(w)) return 1;
  return 0;
}

int AlphaMgr::isWasabiWindow(OSWINDOWHANDLE w) 
{
#ifdef _WIN32
  wchar_t classname[256]=L"";
  GetClassNameW(w, classname, 255);
  return (w == WASABI_API_WND->main_getRootWnd()->gethWnd() || !wcscmp(classname, BASEWNDCLASSNAME));
#else
#warning port me
  return 1;
#endif
}

int AlphaMgr::isMenuWindow(OSWINDOWHANDLE w) 
{
#ifdef _WIN32
  char classname[256]="";
  GetClassNameA(w, classname, 255);
  return STRCASEEQL(classname, "#32768");
#else
  return 0;
#warning port me
#endif
}

int AlphaMgr::isOurExternalWindow(OSWINDOWHANDLE w)
{
  OSWINDOWHANDLE wnd = w;
  if (isWasabiWindow(w)) 
    return 0;

  if (isMenuWindow(wnd)) {
    wnd = Wasabi::Std::Wnd::getFocus();
    if (isWasabiWindow(wnd) || isOurExternalWindow(wnd)) 
      return 1; 
  }
  
  while (wnd) 
  {
    if (Wasabi::Std::Wnd::isPopup(wnd))
    { 
      if (isWasabiWindow(wnd)) 
        return 0; 
      OSWINDOWHANDLE _w = Wasabi::Std::Wnd::getParent(wnd);
#ifdef _WIN32
      if (!_w) _w = GetWindow(wnd, GW_OWNER);
#else
#warning port me
#endif
      if (!wnd) _w = wnd;
      return (_w == WASABI_API_WND->main_getRootWnd()->gethWnd() || isWasabiWindow(_w) || isOurExternalWindow(_w));
    }
    wnd = Wasabi::Std::Wnd::getParent(wnd);
  }
  return 0;
}

void AlphaMgr::preHoverCheck(AlphaMgrEntry *e) {
  int isin = isMouseInLayout(e->getLayout());
  uint32_t last = e->getLastTimeIn();
  uint32_t lastbig = getBigLastTimeIn();
  if (isin) { e->onLastIn(); onBigLastIn(); }
  if (!getAutoOpacify()) {
    if (!isin && last != 0 && (last > Wasabi::Std::getTickCount() - holdtime_ms))
      isin = 1;
  } else {
    if (!isin && lastbig != 0 && (lastbig > Wasabi::Std::getTickCount() - holdtime_ms))
      isin = 1;
  }
  if (!isin) {
    if (hasAutoOpacityOnFocus(e)) {
      if (isFocusingExternalWindow()) isin = 1;
    } else {
      if (isOverExternalWindow()) isin = 1;
    }
  }
  e->setNextIn(isin);
  updateInList(e, isin);
}

void AlphaMgr::hoverCheck(Layout *l) {
  AlphaMgrEntry *e = layouts.findItem((const wchar_t *)l);
  if (e) hoverCheck(e);
}

void AlphaMgr::hoverCheck(AlphaMgrEntry *e, int applytransparency) {
  if (e->getStatus() == STATUS_UNKNOWN) 
    initStatus(e);

  int isin = e->getNextIn();
   
  if (getAutoOpacify()) {
    isin = big_status == STATUS_IN_FADINGON || big_status == STATUS_IN_ON || big_status == STATUS_IN_OFF;
  }
  if (hasAutoOpacity(e)) {
    if (isin) {
      // mouse is in, autoopacity is on
      switch (e->getStatus()) {
        case STATUS_OUT_OFF: 
        case STATUS_OUT_FADINGOUT:
        case STATUS_IN_OFF: 
          e->setStartAlpha(e->getLayout()->getTransparency());
          e->onEnterLeave();
          e->setStatus(STATUS_IN_FADINGON);
          checkTimer();
          e->getLayout()->onMouseEnterLayout();
          break;
        case STATUS_IN_FADINGON:
          doEndCheck(e);
          break;
      }
    } else {
      // mouse is out, autoopacity is on
      switch (e->getStatus()) {
        case STATUS_IN_FADINGON: 
        case STATUS_IN_ON: 
          e->setStartAlpha(e->getLayout()->getTransparency());
          e->onEnterLeave();
          e->setStatus(STATUS_OUT_FADINGOUT);
          checkTimer();
          e->getLayout()->onMouseLeaveLayout();
          break;
        case STATUS_OUT_FADINGOUT:
          doEndCheck(e);
          break;
      }
    }
  } else {
    if (isin) {
      // mouse is in, no autoopacity
      e->setStatus(STATUS_IN_OFF);
    } else {
      // mouse is out, no autoopacity
      e->setStatus(STATUS_OUT_OFF);
    }
  }
//  if (applytransparency) updateTransparency(e->getLayout());
}

void AlphaMgr::timerclient_timerCallback(int id) {
  switch(id) {
    case ALPHAMGR_HOVERCHECK: {
      foreach(layouts)
        AlphaMgrEntry *e = layouts.getfor();
        preHoverCheck(e);
      endfor;
      foreach(layouts)
        AlphaMgrEntry *e = layouts.getfor();
        hoverCheck(e);
      endfor;
    }
    break;
    case ALPHAMGR_UPDATEALPHA: {
      foreach(layouts)
        AlphaMgrEntry *e = layouts.getfor();
        if (e->getStatus() == STATUS_IN_FADINGON || e->getStatus() == STATUS_OUT_FADINGOUT) updateTransparency(e->getLayout());
      endfor;
    }
    break;
  }
}

int AlphaMgr::getBigCurTransparency() {
  switch (getBigStatus()) {
    case STATUS_IN_FADINGON:
    case STATUS_OUT_FADINGOUT:
      return getTransparency(NULL);
    case STATUS_IN_ON:
      return 255;
    case STATUS_IN_OFF:
    case STATUS_OUT_OFF:
      return getGlobalAlpha();
    default: return getTransparency(NULL);
  }
}

void AlphaMgr::setBigStartAlpha(int a) {
  big_startalpha = a;
}

void AlphaMgr::setBigStatus(int s) {
  big_status = s;
}

void AlphaMgr::onBigEnterLeave() {
  big_enterleave_time = Wasabi::Std::getTickCount();
}

uint32_t AlphaMgr::getBigEnterLeaveTime() {
  return big_enterleave_time;
}

void AlphaMgr::setAutoOpacify(int l) { 
  autoopacify = l; 
  resetTimer();
  if (l == 0) {
    foreach(layouts)
      if (layouts.getfor()->getStatus() == STATUS_IN_FADINGON) layouts.getfor()->setStatus(STATUS_IN_OFF);
      if (layouts.getfor()->getStatus() == STATUS_OUT_FADINGOUT) layouts.getfor()->setStatus(STATUS_OUT_OFF);
    endfor;
  }
  updateAllTransparency();
}

void AlphaMgr::resetTimer() {
  timerclient_killTimer(ALPHAMGR_HOVERCHECK);
  if (autoopacify == 1 && alllinked)
    timerclient_setTimer(ALPHAMGR_HOVERCHECK, 99);
  else
    timerclient_setTimer(ALPHAMGR_HOVERCHECK, 300);
}