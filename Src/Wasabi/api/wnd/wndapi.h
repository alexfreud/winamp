#ifndef _WNDAPI_H
#define _WNDAPI_H

#include <api/wnd/api_wnd.h>
#include <bfc/ptrlist.h>
#include <tataki/bitmap/autobitmap.h>
#include <api/wnd/popexitchecker.h>
// ---

class BaseTexture
{
  public:
    BaseTexture(ifc_window *_wnd, const wchar_t *_bmp) : wnd(_wnd), texture(_bmp) {}
    virtual ~BaseTexture() {}

    SkinBitmap *getTexture() { return texture.getBitmap(); }
    ifc_window *getWnd() { return wnd; }

    virtual void renderBaseTexture(ifc_window *wndbase, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha);

  private:
    ifc_window *wnd;
    AutoSkinBitmap texture;
};

// ---

class WndApi : public wnd_apiI
{
  public:

    WndApi();
    virtual ~WndApi();

    virtual ifc_window *main_getRootWnd();
    virtual void main_setRootWnd(ifc_window *w);
    virtual ifc_window *getModalWnd();
    virtual void pushModalWnd(ifc_window *w=MODALWND_NOWND);
    virtual void popModalWnd(ifc_window *w=MODALWND_NOWND);
    virtual ifc_window *rootWndFromPoint(POINT *pt);
    virtual ifc_window *rootWndFromOSHandle(OSWINDOWHANDLE wnd);
    virtual void registerRootWnd(ifc_window *wnd);
    virtual void unregisterRootWnd(ifc_window *wnd);
    virtual int rootwndIsValid(ifc_window *wnd);
    virtual void hookKeyboard(ifc_window *hooker);
    virtual void unhookKeyboard(ifc_window *hooker);
    virtual void kbdReset();
    virtual int interceptOnChar(unsigned int c);
    virtual int interceptOnKeyDown(int k);
    virtual int interceptOnKeyUp(int k);
    virtual int interceptOnSysKeyDown(int k, int kd);
    virtual int interceptOnSysKeyUp(int k, int kd);
    virtual int forwardOnChar(ifc_window *from, unsigned int c, int kd);
    virtual int forwardOnKeyDown(ifc_window *from, int k, int kd);
    virtual int forwardOnKeyUp(ifc_window *from, int k, int kd);
    virtual int forwardOnSysKeyDown(ifc_window *from, int k, int kd);
    virtual int forwardOnSysKeyUp(ifc_window *from, int k, int kd);
    virtual int forwardOnKillFocus();
    virtual void popupexit_deregister(PopupExitCallback *cb);
    virtual void popupexit_register(PopupExitCallback *cb, ifc_window *watched);
    virtual int popupexit_check(ifc_window *w);
    virtual void popupexit_signal();
    virtual void skin_renderBaseTexture(ifc_window *base, ifc_canvas *c, const RECT *r, ifc_window *destWnd, int alpha=255);
    virtual void skin_registerBaseTextureWindow(ifc_window *window, const wchar_t *bmp=NULL);
    virtual void skin_unregisterBaseTextureWindow(ifc_window *window);
    virtual void appdeactivation_push_disallow(ifc_window *w);
    virtual void appdeactivation_pop_disallow(ifc_window *w);
    virtual int appdeactivation_isallowed(ifc_window *w);
    virtual void appdeactivation_setbypass(int i);
#ifdef WASABI_COMPILE_PAINTSETS
    virtual int paintset_present(int set);
#ifdef WASABI_COMPILE_IMGLDR
    virtual void paintset_render(int set, ifc_canvas *c, const RECT *r, int alpha=255);
#ifdef WASABI_COMPILE_FONTS
    virtual void paintset_renderTitle(const wchar_t *t, ifc_canvas *c, const RECT *r, int alpha=255);
#endif // fonts
#endif // imgldr
#endif // paintsets
    virtual int forwardOnMouseWheel(int l, int a);
    virtual void setDefaultDropTarget(void *dt);
    virtual void *getDefaultDropTarget();
    static int getNumBaseTextures() { return baseTextureList.getNumItems(); }
    virtual int pushKeyboardLock();
    virtual int popKeyboardLock();
    virtual int isKeyboardLocked();

		PopupExitChecker popupExitChecker;
  private:

    static BaseTexture *getBaseTexture(ifc_window *b);
    static void renderBaseTexture(ifc_window *base, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha);
    static void renderBaseTexture(ifc_window *base, BaseTexture *s, ifc_canvas *c, const RECT &r, ifc_window *dest, int alpha);
    static PtrList<BaseTexture> baseTextureList;
    static ifc_window *genericwnd;
    static void *default_drop_target;
    static int kbdlock;
};

extern WndApi _wndApi;

#endif
