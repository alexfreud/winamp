#ifndef _SVC_SCRIPTOBJECT_H
#define _SVC_SCRIPTOBJECT_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class ScriptObjectController;

class svc_scriptObject : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::SCRIPTOBJECT; }
  ScriptObjectController *getController(int n);
  void onRegisterClasses(ScriptObjectController *rootController);

  enum {
    GETCONTROLLER=10,
    ONREGISTER=20,
  };
};

inline ScriptObjectController *svc_scriptObject::getController(int n) {
  return _call(GETCONTROLLER, (ScriptObjectController *)0, n);
}

inline void svc_scriptObject::onRegisterClasses(ScriptObjectController *rootController) {
  _voidcall(ONREGISTER, rootController);
}

#endif
