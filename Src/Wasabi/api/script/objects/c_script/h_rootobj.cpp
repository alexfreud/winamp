#include <precomp.h>

#include <api/script/objcontroller.h>
#include "h_rootobj.h"

H_RootObject::H_RootObject(ScriptObject *o) {
  inited = 0;
  me = NULL;
  H_hook(o);
}

H_RootObject::H_RootObject() {
  inited = 0;
  me = NULL;
}

void H_RootObject::H_hook(ScriptObject *o) {
  ASSERT(!inited);
  me = o;
  addMonitorObject(o, &rootObjectGuid);
  if (count++ == 0) {
    onnotify_id = WASABI_API_MAKI->maki_addDlfRef(o->vcpu_getController(), L"onNotify", this);
  }
  inited=1;
}

H_RootObject::~H_RootObject() {
  if (!inited) return;
  WASABI_API_MAKI->maki_remDlfRef(this);
  //count--;
}

int H_RootObject::eventCallback(ScriptObject *object, int dlfid, scriptVar **params, int nparams) {
  if (object != getHookedObject()) return 0;
  if (dlfid == onnotify_id) { hook_onNotify(GET_SCRIPT_STRING(*params[0]), GET_SCRIPT_STRING(*params[1]), GET_SCRIPT_INT(*params[2]), GET_SCRIPT_INT(*params[3])); return 1; }
  return 0;
}

int H_RootObject::onnotify_id=0;
int H_RootObject::inited=0;
int H_RootObject::count=0;
