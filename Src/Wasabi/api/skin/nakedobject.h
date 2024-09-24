#ifndef __NAKEDOBJECT_H
#define __NAKEDOBJECT_H

#include <api/wnd/wndclass/guiobjwnd.h>

// NakedObject, an invisible GuiObject

#define NAKEDOBJECT_PARENT GuiObjectWnd 

class NakedObject : public GuiObjectWnd {
  public:
    NakedObject();
    virtual ~NakedObject() {}

    virtual int getPreferences(int what);
    virtual int onResize();
    virtual void onSetVisible(int i);

  protected:
    int reentry_onresize;
    int reentry_onsetvisible;
};

#endif
