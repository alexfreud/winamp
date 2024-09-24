#include <precomp.h>

#include <api/script/scriptobj.h>
#include <api/script/objcontroller.h>
#include "scripthook.h"

#define CBCLASS ScriptHookI
START_DISPATCH;
  CB(EVENTCALLBACK,    eventCallback);
END_DISPATCH;

ScriptHookI::ScriptHookI() {
}

ScriptHookI::~ScriptHookI() {
  WASABI_API_MAKI->maki_remDlfRef(this);
  foreach(controllers)
    controllers.getfor()->removeHooks(this);
  endfor
  
}


void ScriptHookI::addMonitorObject(ScriptObject *o, const GUID *hookedclass) {
  WASABI_API_MAKI->maki_addDlfClassRef(o->vcpu_getController(), this);
  ScriptObjectController *cont = o->vcpu_getController();
  if (hookedclass == NULL) {
    cont->addObjectHook(this, o);
  } else {
    while (cont) {
      if (cont->getClassGuid() == *hookedclass) {
        cont->addObjectHook(this, o);
        break;
      }
      cont = cont->getAncestorController();
    }
  }
  if (cont == NULL) // guid not found
    return;
  controllers.addItem(cont);
}

void ScriptHookI::addMonitorClass(ScriptObject *o) {
  WASABI_API_MAKI->maki_addDlfClassRef(o->vcpu_getController(), this);
  o->vcpu_getController()->addClassHook(this);
  controllers.addItem(o->vcpu_getController());
}
