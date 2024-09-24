#ifndef _SVC_ACCESSIBILITY_H
#define _SVC_ACCESSIBILITY_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class Accessible;
class ifc_window;

class NOVTABLE svc_accessibility : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::ACCESSIBILITY; }

  Accessible *createAccessibleObject(ifc_window *w);

  enum {
    SVC_ACCESSIBILITY_CREATEACCESSIBLEOBJECT=10,
  };
};

inline Accessible *svc_accessibility::createAccessibleObject(ifc_window *w) {
  return _call(SVC_ACCESSIBILITY_CREATEACCESSIBLEOBJECT, (Accessible *)NULL, w);
}

class NOVTABLE svc_accessibilityI: public svc_accessibility {
  public:
    virtual Accessible *createAccessibleObject(ifc_window *w)=0;

  protected:
    RECVS_DISPATCH;
};

#include <api/service/servicei.h>
template <class T>
class AccessibilityCreatorSingle : public waServiceFactoryTSingle<svc_accessibility, T> {
public:
  svc_accessibility *getHandler() {
    return waServiceFactoryTSingle<svc_accessibility, T>::getSingleService();
  }
};


#endif
