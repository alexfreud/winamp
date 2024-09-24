#include <precomp.h>
#include <api.h>
#include "wndapi.h"
#include <api/wnd/api_window.h>
#include <tataki/canvas/ifc_canvas.h>

#include <api/wnd/deactivatemgr.h>
#include <api/wnd/wndtrack.h>

#include <api/syscb/callbacks/consolecb.h>
#include <api/wnd/keyboard.h>

#ifdef WASABI_COMPILE_SKIN
//  #include <api/skin/groupmgr.h>
#endif

#ifdef WASABI_COMPILE_PAINTSETS
#ifdef WASABI_COMPILE_IMGLDR
  #include <api/wnd/paintset.h>
#endif
#endif

#ifdef WASABI_COMPILE_SKIN
//#include <api/skin/skin.h>
#endif
#include <bfc/stack.h>
#include <tataki/region/region.h>
wnd_api *wndApi = NULL;

#ifndef ODS
#define ODS(msg1, msg2) __ODS(__LINE__, msg1, msg2, 0);
#endif

static Stack<ifc_window*> modal_wnd_stack;

static void __ODS(int line, wchar_t *message1, wchar_t *message2, int severity) {
  StringPrintfW s(L"wndApi(%d): %s: %s\n", line, message1, message2);
#ifdef WIN32
  DebugStringW(L"%s\n", s);
#endif
  WASABI_API_SYSCB->syscb_issueCallback(SysCallback::CONSOLE, ConsoleCallback::DEBUGMESSAGE, severity, reinterpret_cast<intptr_t>(s.getValue()));
}

WndApi::WndApi() 
{
}

WndApi::~WndApi() 
{

}

void WndApi::main_setRootWnd(ifc_window *w) {
  genericwnd = w;
}

ifc_window *WndApi::main_getRootWnd() {
  return genericwnd;
}

ifc_window *WndApi::getModalWnd() {
  if (!modal_wnd_stack.peek()) return NULL;
  return modal_wnd_stack.top();
}

void WndApi::pushModalWnd(ifc_window *w) {
  modal_wnd_stack.push(w);
}

void WndApi::popModalWnd(ifc_window *w) {
  if (getModalWnd() != w) return;
  modal_wnd_stack.pop();
}
/* TODO: Benski> move to api_wndmgr */
ifc_window *WndApi::rootWndFromPoint(POINT *pt) {
  return WindowTracker::rootWndFromPoint(pt);
}

ifc_window *WndApi::rootWndFromOSHandle(OSWINDOWHANDLE wnd) {
  return WindowTracker::rootWndFromHwnd(wnd);
}

void WndApi::registerRootWnd(ifc_window *wnd) {
  WindowTracker::addRootWnd(wnd);
}

void WndApi::unregisterRootWnd(ifc_window *wnd) {
  WindowTracker::removeRootWnd(wnd);
}
/* --- end TO MOVE ---*/

int WndApi::rootwndIsValid(ifc_window *wnd) {
  return windowTracker->checkWindow(wnd);
}
void WndApi::hookKeyboard(ifc_window *hooker) {
  Keyboard::hookKeyboard(hooker);
}

void WndApi::unhookKeyboard(ifc_window *hooker) {
  Keyboard::unhookKeyboard(hooker);
}

void WndApi::kbdReset() {
  Keyboard::reset();
}

// so when a key is pressed in a basewnd, it is first intercepted and sent to these functions to check if the system should handle it instead of the wnd...

int WndApi::interceptOnChar(unsigned int c) {
  return Keyboard::interceptOnChar(c);
}

int WndApi::interceptOnKeyDown(int k) {
  return Keyboard::interceptOnKeyDown(k);
}

int WndApi::interceptOnKeyUp(int k) {
  return Keyboard::interceptOnKeyUp(k);
}

int WndApi::interceptOnSysKeyDown(int k, int kd) {
  return Keyboard::interceptOnSysKeyDown(k, kd);
}

int WndApi::interceptOnSysKeyUp(int k, int kd) {
  return Keyboard::interceptOnSysKeyUp(k, kd);
}

// ... if not, then it is sent to the wnd, and if the wnd doesn't need it, it is then forwarded to the system again, for default handling :

int WndApi::forwardOnChar(ifc_window *from, unsigned int c, int kd) {
  return Keyboard::onForwardOnChar(from, c, kd);
}

int WndApi::forwardOnKeyDown(ifc_window *from, int k, int kd) {
  return Keyboard::onForwardOnKeyDown(from, k, kd);
}

int WndApi::forwardOnKeyUp(ifc_window *from, int k, int kd) {
  return Keyboard::onForwardOnKeyUp(from, k, kd);
}

int WndApi::forwardOnSysKeyDown(ifc_window *from, int k, int kd) {
  return Keyboard::onForwardOnSysKeyDown(from, k, kd);
}

int WndApi::forwardOnSysKeyUp(ifc_window *from, int k, int kd) {
  return Keyboard::onForwardOnSysKeyUp(from, k, kd);
}

int WndApi::forwardOnKillFocus() {
  return Keyboard::onForwardOnKillFocus();
}

void WndApi::popupexit_register(PopupExitCallback *cb, ifc_window *watched) 
{
  popupExitChecker.registerCallback(cb, watched);
}
void WndApi::popupexit_deregister(PopupExitCallback *cb) 
{
    popupExitChecker.deregisterCallback(cb);
}

int WndApi::popupexit_check(ifc_window *w) {
  
  return popupExitChecker.check(w);
}

void WndApi::popupexit_signal() 
{
  popupExitChecker.signal();
}

#define RenderBaseTexture renderBaseTexture //CUT
void WndApi::skin_renderBaseTexture(ifc_window *basetexturewnd, ifc_canvas *c, const RECT *r, ifc_window *destwnd, int alpha) {
  if (c == NULL) {
    ODS(L"illegal param", L"c == NULL");
    return;
  }
  if (basetexturewnd == NULL) {
    ODS(L"illegal param", L"base == NULL");
    BaseCloneCanvas canvas(c);
    canvas.fillRect(r, 0xFF00FF);
    return;
  }
  if (destwnd == NULL) {
    ODS(L"illegal param", L"destwnd == NULL");
    return;
  }
  renderBaseTexture(basetexturewnd, c, *r, destwnd, alpha);
}

void WndApi::skin_registerBaseTextureWindow(ifc_window *window, const wchar_t *bitmap) {
  if (window == NULL) {
    ODS(L"illegal param", L"window == NULL");
    return;
  }
  BaseTexture *s = new BaseTexture(window, bitmap);
  baseTextureList.addItem(s);
}

void WndApi::skin_unregisterBaseTextureWindow(ifc_window *window) {
  if (window == NULL) {
    ODS(L"illegal param", L"window == NULL");
    return;
  }
  for (int i=0;i<baseTextureList.getNumItems();i++) {
    if (baseTextureList.enumItem(i)->getWnd() == window) {
      BaseTexture *s = baseTextureList.enumItem(i);
      baseTextureList.delByPos(i);
      delete s;
      if (baseTextureList.getNumItems() == 0)
        baseTextureList.removeAll();
      break;
    }
  }
}

void WndApi::appdeactivation_push_disallow(ifc_window *from_whom) {
  AppDeactivationMgr::push_disallow(from_whom);
}

void WndApi::appdeactivation_pop_disallow(ifc_window *from_whom) {
  AppDeactivationMgr::pop_disallow(from_whom);
}

int WndApi::appdeactivation_isallowed(ifc_window *w) {
  return AppDeactivationMgr::is_deactivation_allowed(w);
}

void WndApi::appdeactivation_setbypass(int i) {
  AppDeactivationMgr::setbypass(i);
}

#ifdef WASABI_COMPILE_PAINTSETS
#ifdef WASABI_COMPILE_IMGLDR

void WndApi::paintset_render(int set, ifc_canvas *c, const RECT *r, int alpha) {
  if (c == NULL) {
		ODS(L"illegal param", L"c == NULL");
    return;
  }
  if (r == NULL) {
    ODS(L"illegal param", L"r == NULL");
    return;
  }
  paintset_renderPaintSet(set, c, r, alpha);
}

#ifdef WASABI_COMPILE_FONTS
void WndApi::paintset_renderTitle(const wchar_t *t, ifc_canvas *c, const RECT *r, int alpha) {
  if (c == NULL) {
    ODS(L"illegal param", L"c == NULL");
    return;
  }
  if (r == NULL) {
    ODS(L"illegal param", L"r == NULL");
    return;
  }
  ::paintset_renderTitle(t, c, r, alpha);
}
#endif // WASABI_COMPILE_FONTS
#endif // WASABI_COMPILE_IMGLDR
#endif // WASABI_COMPILE_PAINTSETS

BaseTexture *WndApi::getBaseTexture(ifc_window *b) {
  if (b == NULL) return NULL;
  for (int i=0;i<baseTextureList.getNumItems();i++)
    if (baseTextureList.enumItem(i)->getWnd() == b)
      return baseTextureList.enumItem(i);
  return NULL;
}

void WndApi::renderBaseTexture(ifc_window *base, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha) {
  renderBaseTexture(base, getBaseTexture(base), c, r, dest, alpha);
}

void WndApi::renderBaseTexture(ifc_window *base, BaseTexture *bt, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha) {
  if (!bt) {
    DebugString("Warning, no base texture found in renderBaseTexture\n");
    return;
  }

  bt->renderBaseTexture(base, c, r, dest, alpha);
}

#ifndef SAFEROUND
#define SAFEROUND(d) ((float)(int)d == d) ? (int)d : (d - (float)(int)d > 0) ? ((int)d)+1 : ((int)d)-1
#endif

void BaseTexture::renderBaseTexture(ifc_window *wndbase, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha) 
{
  // pick our basetexture
  AutoSkinBitmap *b = &texture;

  if(!b) return;

  // srcRect is the source rectangle in the basetexture
  RECT srcRect; 
  // destProjectedRect is the basetexture rectangle projected to dest coordinates
  RECT destProjectedRect; 
  
  ifc_window *p = dest;
  POINT pt;
  int sx=0, sy=0;
  while (p && p != wndbase) {
    if (!p->isVirtual()) {
      p->getPosition(&pt);
      sx += pt.x;
      sy += pt.y;
    }
    p = p->getParent();
  }
  ASSERT(p);  

  wndbase->getNonClientRect(&destProjectedRect);
  Wasabi::Std::offsetRect(&destProjectedRect, -sx, -sy);

  Wasabi::Std::setRect(&srcRect, 0, 0, b->getWidth(), b->getHeight());

#if 0
  // NONPORTABLE 
  HDC hdc=c->getHDC();
  HRGN oldRgn=CreateRectRgn(0,0,0,0);
  HRGN newRgn=CreateRectRgnIndirect(&r);

  int cs=GetClipRgn(hdc,oldRgn);

  ExtSelectClipRgn(hdc,newRgn,(cs!=1)?RGN_COPY:RGN_AND);
#endif
#ifdef _WIN32
  // TODO: review: wtf does this even accomplish?  we're still blitting to c at the end
  BaseCloneCanvas clone(c);
  RegionI oldRgn;
  RegionI newRgn(&r);

  int cs = clone.getClipRgn(&oldRgn);

  if ( cs ) newRgn.andRegion(&oldRgn);

  clone.selectClipRgn( &newRgn );
#endif
  b->stretchToRectAlpha(c, &srcRect, &destProjectedRect, alpha);

#ifdef _WIN32
  // TODO: review: wtf does this even accomplish?  we're still blitting to c at the end
  clone.selectClipRgn(cs ? &oldRgn : NULL);
#endif
}

int WndApi::forwardOnMouseWheel(int l, int a) {
  return 0;
}

#ifdef WASABI_COMPILE_PAINTSETS
int WndApi::paintset_present(int set) {
  return ::paintset_present(set);
}
#endif

void WndApi::setDefaultDropTarget(void *dt) {
  default_drop_target = dt;
}

void *WndApi::getDefaultDropTarget() {
  return default_drop_target;
}

int WndApi::pushKeyboardLock() {
  return ++kbdlock;
}

int WndApi::popKeyboardLock() {
  return --kbdlock;
}

int WndApi::isKeyboardLocked() {
  return kbdlock;
}

ifc_window *WndApi::genericwnd = NULL;
PtrList<BaseTexture> WndApi::baseTextureList;
void *WndApi::default_drop_target = NULL;
int WndApi::kbdlock = 0;
