#ifndef __SCRIPTHOOK_H
#define __SCRIPTHOOK_H

#include <api/script/vcputypes.h>
#include <bfc/dispatch.h>
#include <bfc/ptrlist.h>


class ScriptObject;
class ScriptObjectController;

// ----------------------------------------------------------------------------------------------------------

class ScriptHook : public Dispatchable {
  protected:
    ScriptHook() {};

  public:
    int eventCallback(ScriptObject *object, int dlfid, scriptVar **params, int nparams);

  enum {
    EVENTCALLBACK            = 100,
  };

};

inline int ScriptHook::eventCallback(ScriptObject *object, int dlfid, scriptVar **params, int nparams) {
  return _call(EVENTCALLBACK, 0, object, dlfid, params, nparams);
}

class ScriptHookI : public ScriptHook {

public:

  ScriptHookI();
  virtual ~ScriptHookI();

  virtual int eventCallback(ScriptObject *object, int dlfid, scriptVar **params, int nparams)=0;

  void addMonitorObject(ScriptObject *o, const GUID *hookedclass=NULL); // NULL = all classes of object o
  void addMonitorClass(ScriptObject *o);

protected:
  RECVS_DISPATCH;

  PtrList<ScriptObjectController> controllers;
};

#endif