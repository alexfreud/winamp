#include "precomp.h"
#include <api/wnd/api_wnd.h>

#include "rootwndholder.h"
#include <api/wnd/notifmsg.h>
#include <tataki/canvas/canvas.h>


RootWndHolder::RootWndHolder() {
  privptr = NULL;
}

RootWndHolder::~RootWndHolder() {
}

void RootWndHolder::rootwndholder_getRect(RECT *r) {
  if (isInited())
    getClientRect(r);
  else
    MEMSET(r, 0, sizeof(RECT));
}

int RootWndHolder::onInit() {
  ROOTWNDHOLDER_PARENT::onInit();
  ifc_window *w = rootwndholder_getRootWnd();
  if (w) {
    checkInit(w);
    setName(rootwndholder_getRootWnd()->getRootWndName());
  }
  return 1;
}

void RootWndHolder::checkInit(ifc_window *w) {
  if (w && !w->isInited()) {
    if (w->getParent() == NULL)
      w->setParent(this);
//    w->setStartHidden(getStartHidden());
    w->init(this);
  }
}

int RootWndHolder::onResize() {
  int rv = ROOTWNDHOLDER_PARENT::onResize();
  if (!isInited()) return 1;
  ifc_window *held = rootwndholder_getRootWnd();
  if (!held) return rv;
  RECT r;
  rootwndholder_getRect(&r);
  if (renderRatioActive() && !held->handleRatio())
    multRatio(&r);
  held->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
  return rv;
}

/*void RootWndHolder::onSetVisible(int v) {
  ROOTWNDHOLDER_PARENT::onSetVisible(v);
  if (!rootwndholder_getRootWnd()) return;
  rootwndholder_getRootWnd()->setVisible(v);
}*/

int RootWndHolder::onActivate() {
  int r = ROOTWNDHOLDER_PARENT::onActivate();
  if (rootwndholder_getRootWnd())
    rootwndholder_getRootWnd()->onActivate();
  return r;
}

int RootWndHolder::onDeactivate() {
  int r = ROOTWNDHOLDER_PARENT::onDeactivate();
    if (rootwndholder_getRootWnd()) 
    rootwndholder_getRootWnd()->onDeactivate();
  return r;
}

int RootWndHolder::getPreferences(int what) {
  if (rootwndholder_getRootWnd())
    return rootwndholder_getRootWnd()->getPreferences(what);
  return ROOTWNDHOLDER_PARENT::getPreferences(what);
}

ifc_window *RootWndHolder::rootwndholder_getRootWnd() {
  return privptr; 
} 

void RootWndHolder::rootwndholder_setRootWnd(ifc_window *w) { 
  if (privptr == w) return;
  privptr = w;
  checkInit(w);
  if (isPostOnInit())
    onResize(); 
}

int RootWndHolder::onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source) {
  if (rootwndholder_getRootWnd())
    return rootwndholder_getRootWnd()->onAction(action, param, x, y, p1, p2, data, datalen, source);
  return ROOTWNDHOLDER_PARENT::onAction(action, param, x, y, p1, p2, data, datalen, source);
}

int RootWndHolder::childNotify(ifc_window *child, int msg, intptr_t param1, intptr_t param2) {
  if (msg == ChildNotify::NAMECHANGED && child == rootwndholder_getRootWnd())
    setName(child->getRootWndName());
  return passNotifyUp(child, msg, (int)param1, (int)param2);
}

int RootWndHolder::onPaint(Canvas *c) {
  int rt = ROOTWNDHOLDER_PARENT::onPaint(c);
  if (wantRenderBaseTexture()) {
    RECT r;
    rootwndholder_getRect(&r);
    WASABI_API_WND->skin_renderBaseTexture(getBaseTextureWindow(), c, r, this);
  }
  return rt;
}