#ifndef _POPUPCB_H
#define _POPUPCB_H

#include <bfc/common.h>
#include <bfc/dispatch.h>

class ifc_dependent;

class PopupExitCallback : public Dispatchable {
  public:

  int popupexitcb_onExitPopup();
  ifc_dependent *popupexit_getDependencyPtr();

  enum {
    POPUPEXIT_ONEXITPOPUP=100,	
    POPUPEXIT_GETDEPENDENCYPTR=110,	
  };
};

inline int PopupExitCallback::popupexitcb_onExitPopup() {
  return _call(POPUPEXIT_ONEXITPOPUP, 0);
}

inline ifc_dependent *PopupExitCallback::popupexit_getDependencyPtr() {
  return _call(POPUPEXIT_GETDEPENDENCYPTR, (ifc_dependent *)NULL);
}

class PopupExitCallbackI : public PopupExitCallback {
  public:

    virtual int popupexitcb_onExitPopup()=0;
    virtual ifc_dependent *popupexit_getDependencyPtr()=0;
  
  protected:

    RECVS_DISPATCH;
};

#endif
