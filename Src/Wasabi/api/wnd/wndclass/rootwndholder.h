#ifndef __ROOTWNDHOLD_H
#define __ROOTWNDHOLD_H

#include <api/wnd/virtualwnd.h>

/**
 A simple wnd that holds another window. Initializes it if needed, but DOES not delete it (and for a good reason, this is a ifc_window),
 so your inheritor has to call whoever is needed to destroy the wnd
*/

#define ROOTWNDHOLDER_PARENT VirtualWnd 

class RootWndHolder : public ROOTWNDHOLDER_PARENT 
{
  public:
    RootWndHolder();
    virtual ~RootWndHolder();

    // override this
    virtual ifc_window *rootwndholder_getRootWnd(); 
    virtual void rootwndholder_getRect(RECT *r);
    virtual void rootwndholder_setRootWnd(ifc_window *w);

		// BaseWnd
    virtual int onInit();
    virtual int onResize();
//    virtual void onSetVisible(int v);
    virtual int wantRenderBaseTexture() { return 0; }
    virtual int onPaint(Canvas *c);
    virtual int onActivate();
    virtual int onDeactivate();
    virtual int getPreferences(int what);
    virtual int onAction(const wchar_t *action, const wchar_t *param=NULL, int x=-1, int y=-1, intptr_t p1=0, intptr_t p2=0, void *data=NULL, size_t datalen=0, ifc_window *source=NULL);
    virtual int childNotify(ifc_window *child, int msg, intptr_t param1=0, intptr_t param2=0);

  private:
    void checkInit(ifc_window *w);
    ifc_window *privptr;
};

#endif