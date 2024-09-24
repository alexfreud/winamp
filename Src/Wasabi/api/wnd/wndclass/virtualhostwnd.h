#ifndef __VIRTUALHOSTWND_H
#define __VIRTUALHOSTWND_H

#include "../../common/guiobjwnd.h"

class VirtualHostWnd : public GuiObjectWnd {
  public:
    VirtualHostWnd();
    virtual ~VirtualHostWnd();

    virtual int onInit();
    virtual int onPaint(Canvas *c);
    virtual int onResize();
    virtual void onChildInvalidate(api_region *r, ifc_window *who);

    virtual int onLeftButtonDown(int x, int y);
    virtual int onLeftButtonUp(int x, int y);
    virtual int onRightButtonDown(int x, int y);
    virtual int onRightButtonUp(int x, int y);
    virtual int onLeftButtonDblClk(int x, int y);
    virtual int onRightButtonDblClk(int x, int y);
    virtual int onMouseMove(int x, int y);

    virtual void virtualhostwnd_setContent(const wchar_t *groupid);
    virtual void virtualhostwnd_setContent(SkinItem *item);
    virtual void virtualhostwnd_onNewContent();
    virtual void virtualhostwnd_onPaintBackground(Canvas *c);

    virtual void virtualhostwnd_fitToClient(int fit);
    virtual void virtualhostwnd_getContentRect(RECT *r);

    virtual ifc_window *virtualhostwnd_getContentRootWnd();
#ifdef WASABI_COMPILE_SCRIPT
    virtual ScriptObject *virtualhostwnd_findScriptObject(const wchar_t *object_id);
#endif
#ifdef WASABI_COMPILE_SKIN
    virtual GuiObject *virtualhostwnd_findObject(const wchar_t *object_id);
    virtual GuiObject *virtualhostwnd_getContent();
    virtual ScriptObject *virtualhostwnd_getContentScriptObject();
#endif

  private:

    GuiObjectWnd *group;
    int fittoclient;
    int xoffset, yoffset;
    int groupwidth, groupheight;
    int scripts_enabled;
};

#endif
